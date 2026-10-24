/*
  AudioFileSourceHTTPStreamUA
  Streaming HTTP source

  Derived from AudioFileSourceHTTPStream in ESP8266Audio, tag 1.9.9
  (https://github.com/earlephilhower/ESP8266Audio). Modifications,
  per GPL section 5(a):
    - class renamed AudioFileSourceHTTPStream -> AudioFileSourceHTTPStreamUA
      so this copy and the unmodified library copy do not produce duplicate
      symbol definitions;
    - added SetUserAgent()/userAgent[] and a (url, userAgent) constructor
      overload; open() applies the configured User-Agent before every GET
      (covers the initial open and any reconnect requests, which re-enter
      open()).
  Modified 2026-10-24. This file is otherwise based on ESP8266Audio 1.9.9
  as described above; upstream remains the authoritative implementation.

  Copyright (C) 2017  Earle F. Philhower, III

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(ESP32) || defined(ESP8266)

#include "AudioFileSourceHTTPStreamUA.h"

AudioFileSourceHTTPStreamUA::AudioFileSourceHTTPStreamUA()
{
  pos = 0;
  reconnectTries = 0;
  saveURL[0] = 0;
  userAgent[0] = 0;
}

AudioFileSourceHTTPStreamUA::AudioFileSourceHTTPStreamUA(const char *url)
{
  saveURL[0] = 0;
  reconnectTries = 0;
  userAgent[0] = 0;
  open(url);
}

AudioFileSourceHTTPStreamUA::AudioFileSourceHTTPStreamUA(const char *url, const char *ua)
{
  saveURL[0] = 0;
  reconnectTries = 0;
  userAgent[0] = 0;
  SetUserAgent(ua);   // bounded copy, NUL-terminated, NULL-safe (leaves empty string on NULL)
  open(url);          // open() applies userAgent before GET(), so the first request carries it
}

bool AudioFileSourceHTTPStreamUA::open(const char *url)
{
  pos = 0;
  http.begin(client, url);
  http.setReuse(true);
#ifndef ESP32
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
#endif
  if (userAgent[0]) http.setUserAgent(userAgent);
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    cb.st(STATUS_HTTPFAIL, PSTR("Can't open HTTP request"));
    return false;
  }
  size = http.getSize();
  strncpy(saveURL, url, sizeof(saveURL));
  saveURL[sizeof(saveURL)-1] = 0;
  return true;
}

AudioFileSourceHTTPStreamUA::~AudioFileSourceHTTPStreamUA()
{
  http.end();
}

uint32_t AudioFileSourceHTTPStreamUA::read(void *data, uint32_t len)
{
  if (data==NULL) {
    audioLogger->printf_P(PSTR("ERROR! AudioFileSourceHTTPStreamUA::read passed NULL data\n"));
    return 0;
  }
  return readInternal(data, len, false);
}

uint32_t AudioFileSourceHTTPStreamUA::readNonBlock(void *data, uint32_t len)
{
  if (data==NULL) {
    audioLogger->printf_P(PSTR("ERROR! AudioFileSourceHTTPStreamUA::readNonBlock passed NULL data\n"));
    return 0;
  }
  return readInternal(data, len, true);
}

uint32_t AudioFileSourceHTTPStreamUA::readInternal(void *data, uint32_t len, bool nonBlock)
{
retry:
  if (!http.connected()) {
    cb.st(STATUS_DISCONNECTED, PSTR("Stream disconnected"));
    http.end();
    for (int i = 0; i < reconnectTries; i++) {
      char buff[64];
      sprintf_P(buff, PSTR("Attempting to reconnect, try %d"), i);
      cb.st(STATUS_RECONNECTING, buff);
      delay(reconnectDelayMs);
      if (open(saveURL)) {
        cb.st(STATUS_RECONNECTED, PSTR("Stream reconnected"));
        break;
      }
    }
    if (!http.connected()) {
      cb.st(STATUS_DISCONNECTED, PSTR("Unable to reconnect"));
      return 0;
    }
  }
  if ((size > 0) && (pos >= size)) return 0;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  NetworkClient *stream = http.getStreamPtr();
#else
  WiFiClient *stream = http.getStreamPtr();
#endif

  // Can't read past EOF...
  if ( (size > 0) && (len > (uint32_t)(pos - size)) ) len = pos - size;

  if (!nonBlock) {
    int start = millis();
    while ((stream->available() < (int)len) && (millis() - start < 500)) yield();
  }

  size_t avail = stream->available();
  if (!nonBlock && !avail) {
    cb.st(STATUS_NODATA, PSTR("No stream data available"));
    http.end();
    goto retry;
  }
  if (avail == 0) return 0;
  if (avail < len) len = avail;

  int read = stream->read(reinterpret_cast<uint8_t*>(data), len);
  pos += read;
  return read;
}

bool AudioFileSourceHTTPStreamUA::seek(int32_t pos, int dir)
{
  audioLogger->printf_P(PSTR("ERROR! AudioFileSourceHTTPStreamUA::seek not implemented!"));
  (void) pos;
  (void) dir;
  return false;
}

bool AudioFileSourceHTTPStreamUA::close()
{
  http.end();
  return true;
}

bool AudioFileSourceHTTPStreamUA::isOpen()
{
  return http.connected();
}

uint32_t AudioFileSourceHTTPStreamUA::getSize()
{
  return size;
}

uint32_t AudioFileSourceHTTPStreamUA::getPos()
{
  return pos;
}

#endif
