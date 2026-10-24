# only80sRadio 📻

<p align="center">
  <img src="radio80-0001.jpg" alt="only80sRadio — project photo 1" width="32%">
  <img src="radio80-0003.jpg" alt="only80sRadio — project photo 2" width="32%">
  <img src="radio80-0004.jpg" alt="only80sRadio — project photo 3" width="32%">
</p>

only80sRadio is a minimal internet radio designed to solve the decades-old problem of radios playing non-80s music. While modern radios come with a tuner, which means you have to manually tune a dial yourself until you finally find the 80s channel, only80sRadio is always pre-tuned to the 80s station. Just flip the ON switch and the radio knows EXACTLY what you came for. only80sRadio connects to Wi-Fi and plays the HTTP MP3 stream of your choice. In theory, it could even play non-80s music streams if you want, but that's crazy talk! Get creative with the enclosure. We made a prototype out of an empty gum container (see photos).

## Hardware

- LOLIN/WEMOS ESP32-S2 Mini (any ESP32 will work but pin mapping may vary).
- I2S-compatible DAC or amplifier (in the pictured prototype a MAX98357A is used).
- Speaker compatible with the selected amplifier (e.g., a 3W 4Ω speaker).
- Suggested power supply: rechargeable 3.7V Li-ion battery with a compatible
  charging/boost module (the prototype uses an XL-LBC3.1 module).

### I2S wiring

| ESP32-S2 Mini GPIO | I2S signal |
| --- | --- |
| GPIO 8 | BCLK |
| GPIO 7 | LRC / WS |
| GPIO 9 | DIN |

Verify your exact board pinout and DAC/amplifier labels before connecting hardware.

## Software

- PlatformIO (builds from the repository root).
- Espressif `espressif32` platform, pinned to `6.12.0` in `platformio.ini`.
- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) by Earle F.
  Philhower, pinned to exactly `1.9.9`. The MP3 decoder and I2S output come
  from the stock library; the HTTP stream class is vendored in
  `radio80s_esp32s2/` (see *Vendored code* below).

## Build and flash

```sh
git clone https://github.com/tranquility-base-0/only80sRadio.git
cd only80sRadio
cp radio80s_esp32s2/config.h.example radio80s_esp32s2/config.h
$EDITOR radio80s_esp32s2/config.h      # fill in your own values
pio run -e lolin_s2_mini               # build
pio run -e lolin_s2_mini -t upload     # flash
pio device monitor                     # serial monitor (115200)
```

## Configuration (`radio80s_esp32s2/config.h`, git-ignored)

| Setting | Meaning |
| --- | --- |
| `WIFI_SSID` / `WIFI_PASSWORD` | Your Wi-Fi network credentials. |
| `STATION_URL` | The HTTP MP3 stream URL you want to listen to. Configure a stream you have the right to use; no default endpoint is provided. |
| `STATION_NAME` | Shown on the serial console at boot. |
| `PLAYER_USER_AGENT` | The HTTP User-Agent sent with each stream GET request, including reconnects. |

`config.h.example` is tracked in the repository with placeholder values. Copy it to `config.h` as shown above, then edit `config.h` and replace all five example values with your own settings before building. The real `config.h` is git-ignored: keep it local and never commit it, especially if it contains Wi-Fi credentials. The firmware refuses to start if any required configuration value is unset, empty, or still begins with `YOUR_`.

### User-Agent

The firmware sends a unique, provider-neutral User-Agent with each stream GET request, including reconnects:

```
only80sRadio/1.0 (+https://github.com/tranquility-base-0/only80sRadio)
```

Do not name a stream provider in the User-Agent; it identifies this
player, not any station. The stock `AudioFileSourceHTTPStream` in
ESP8266Audio 1.9.9 does not expose a User-Agent setter (its HTTP client
is a private member), which is why a vendored copy exists — see below.

## Vendored code

`radio80s_esp32s2/AudioFileSourceHTTPStreamUA.{h,cpp}` is derived from
`AudioFileSourceHTTPStream` in ESP8266Audio **tag 1.9.9** (modified
2026-10-24) with the following modifications:

1. the class is renamed (`AudioFileSourceHTTPStreamUA`) so the copy and
   the unmodified library do not produce duplicate symbol definitions;
2. a `SetUserAgent()` setter and a 96-byte `userAgent[]` buffer were
   added (bounded copy, NUL-terminated);
3. a `(url, userAgent)` constructor overload was added so the configured
   User-Agent is stored before the first request is made;
4. `open()` applies the configured User-Agent before every GET —
   including reconnect requests, which re-enter `open()`.

Otherwise this implementation is based on upstream 1.9.9; upstream
remains the authoritative implementation. The upstream copyright and
license notices are preserved in both files.

## Known limitations (confirmed against the actual 1.9.9 implementation)

- **Plain HTTP only.** The stream class uses a plain `WiFiClient`; HTTPS
  stream URLs are not supported by this class.
- **No redirect following on ESP32.** `open()` enables forced redirect
  following on ESP8266 only (`#ifndef ESP32`); on ESP32 an HTTP 3xx
  response is treated as a failure.
- **No automatic reconnect by default.** The stream class supports
  `SetReconnect(tries, delayMs)`, but the sketch leaves the library
  default (0 tries), matching the original prototype's behavior. When a
  reconnect does happen (after configuring retries), the configured
  User-Agent is sent again.
- **No ICY metadata handling.** The sketch uses the plain HTTP stream
  class, which does not request ICY metadata.
- **No Wi-Fi connection timeout.** `setup()` waits indefinitely for
  Wi-Fi, matching the original prototype.
- The result of `mp3->begin()` is not checked, matching the original
  prototype.

## Licensing

The project's original code — the sketch, `platformio.ini`,
`config.h.example`, and documentation — is licensed under the GNU
General Public License, either version 3 of the License, or (at your
option) any later version (GPL-3.0-or-later). See
[`LICENSE`](LICENSE) for the full license text.

The [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)
dependency is licensed under GPL-3.0-or-later as well. Firmware built
from this repository statically incorporates that library, and the
vendored stream copy retains its upstream copyright and license notices.
Distributing firmware that incorporates GPL-licensed libraries requires
careful GPL compliance review; see
[`THIRD-PARTY-NOTICES.md`](THIRD-PARTY-NOTICES.md) for the attribution
and modification record.

Third-party libraries remain subject to their own licenses.
