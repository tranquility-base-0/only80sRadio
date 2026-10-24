# Third-party notices

This project incorporates and modifies third-party software. The
original license terms below are preserved; nothing here modifies them.

## ESP8266Audio 1.9.9

- Project: [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)
- Version: tag `1.9.9` (exact pin in `platformio.ini`)
- Copyright: (C) 2017  Earle F. Philhower, III
- License: GNU General Public License, either version 3 of the License,
  or (at your option) any later version (GPL-3.0-or-later). See the
  license text in [`LICENSE`](LICENSE).

### Modified files

The following files in `radio80s_esp32s2/` are derived from
`src/AudioFileSourceHTTPStream.{h,cpp}` at tag 1.9.9 and carry the
upstream copyright and license notices plus a GPL section 5(a)
modification note:

- `radio80s_esp32s2/AudioFileSourceHTTPStreamUA.h`
- `radio80s_esp32s2/AudioFileSourceHTTPStreamUA.cpp`

### Summary of modifications (as documented in the files themselves)

- The class is renamed `AudioFileSourceHTTPStream` →
  `AudioFileSourceHTTPStreamUA` so this copy and the unmodified library
  copy do not produce duplicate symbol definitions.
- A `SetUserAgent()` setter and a 96-byte `userAgent[]` buffer were
  added (bounded copy, NUL-terminated).
- A `(url, userAgent)` constructor overload was added so the configured
  User-Agent is stored before the first request is made.
- `open()` applies the configured User-Agent before every HTTP GET,
  including reconnect requests, which re-enter `open()`.

Otherwise the implementation is based on upstream 1.9.9; upstream
remains the authoritative implementation.

### Unmodified library components

All other ESP8266Audio components used by this project — notably
`AudioGeneratorMP3` and `AudioOutputI2S` — are used unmodified from the
pinned 1.9.9 release and remain subject to their upstream license.
