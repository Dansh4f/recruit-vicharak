ESP32 High-Speed HTTPS Download to SPIFFS
========================================

Overview
--------
This project (Arduino framework for ESP32) downloads a file over HTTPS and writes it to SPIFFS. The code in this repository demonstrates an HTTPS client using `WiFiClientSecure` + `HTTPClient`, writes the received bytes into SPIFFS, and measures download and write speeds.

This README documents how the current code works, how to configure and build it, how to measure throughput, and suggested fixes/optimizations to reach or exceed 400 KB/s sustained throughput.

Quick checklist (requirements coverage)
-------------------------------------
- Hardware: ESP32 Dev Kit and internet connection — Documentation included. (Done)
- Software: Arduino framework, SPIFFS, HTTPS client — Code uses `WiFiClientSecure`, `HTTPClient`, and `SPIFFS`. (Done)
- Configurable URL: `lib/core/HttpsClient.hpp` contains a `const char* url` you can change. (Done)
- File written to SPIFFS and speeds measured/printed to Serial. (Done)
- Error handling and suggestions included; a minor bug is noted and fixed recommendation provided. (Documented)

Project layout (relevant files)
------------------------------
- `src/main.cpp` — boots, mounts SPIFFS, connects to WiFi, calls the HTTPS download routine.
- `lib/core/WiFiManager.hpp` — WiFi connection helper using `WiFiMulti` and `wifimanager()`.
- `lib/core/HttpsClient.hpp` — HTTPS download and SPIFFS write logic; measures download and write speeds.
- `lib/core/SPIFFS.hpp` — simple reader for the stored file (`readFile()`).

How it works (implementation summary)
------------------------------------
- WiFi: `WiFiMulti` is used and `wifimanager()` adds an AP (hardcoded SSID/password in `WiFiManager.hpp`) and blocks until connected.
- SPIFFS: Mounted in `setup()` using `SPIFFS.begin(true)` (format if not mounted).
- HTTPS Download: `HttpsClient.hpp`:
  - Uses `WiFiClientSecure` + `HTTPClient`.
  - `client.setInsecure()` is used (disables certificate verification) for simpler TLS setup.
  - The target URL is in `const char* url` (edit this to change what to download).
  - A large static buffer `buff[8192]` (8 KB) is used for read/write I/O.
  - The code streams the response into SPIFFS, measuring micros() before/after download and per-write time to compute speeds.
  - It prints lines like `average download speed: 115.08 KB/s` and `avg write speed: 794.88 KB/s` to Serial (values vary by run).

Key parameters and reasoning / performance knobs
-----------------------------------------------
- Buffer size: 8192 bytes (8 KB). This is a good compromise between memory usage and throughput. Increasing the buffer size (for example to 16 KB) can reduce call overhead and improve throughput, at the cost of RAM.
- TLS: `setInsecure()` reduces TLS verification overhead and avoids certificate management; this can slightly improve throughput but reduces security. For production, use certificate pinning or root CA verification.
- SPIFFS writes: The code writes in blocks matching the network buffer read size. The write speed depends on the flash chip and partition scheme; use the default SPIFFS/LittleFS settings in PlatformIO or tune the flash frequency and mode in `platformio.ini` if needed.
- WiFi: Signal strength and AP/router performance are often the real bottleneck. Keep the ESP32 near the access point and avoid crowded 2.4 GHz channels.

Configuration
-------------
- Change download URL:
  - The project currently uses the URL hardcoded in `lib/core/HttpsClient.hpp`:
    `https://100daysofcode.s3-eu-west-1.amazonaws.com/schedule.txt`.
    This is a small text file useful for functionality checks but too small for a reliable throughput measurement.
  For performance testing (400 KB/s target) use a larger test file, for example:
  `https://100daysofcode.s3-eu-west-1.amazonaws.com/rfc2616.txt`.
- Change output filename:
  - Edit `const char* filename` in `lib/core/HttpsClient.hpp` (currently `"/file.txt"`).
- Change WiFi credentials:
  - Edit `lib/core/WiFiManager.hpp` — `ssid` and `password` are currently hardcoded.

Build and flash (PlatformIO, Windows PowerShell)
------------------------------------------------
Open a PowerShell terminal at the project root and run:

```powershell
# Build
pio run

# Upload/Flash to the connected ESP32 (PlatformIO environment default)
pio run -t upload

# Monitor serial output at 115200 baud
pio device monitor -b 115200
```

If your `platformio.ini` uses a different environment name (e.g. `esp32dev`), pass `-e <env>` to `pio run` and `-t upload`.

How to run the test
-------------------
1. Edit `lib/core/HttpsClient.hpp` and set the desired `const char* url` value. Examples used in these logs:
  - Small functional check: `https://100daysofcode.s3-eu-west-1.amazonaws.com/schedule.txt`
  - Larger test file used here: `https://100daysofcode.s3-eu-west-1.amazonaws.com/rfc2616.txt`
  - For sustained throughput benchmarking, use a large binary (50–100+ MB) such as `https://speed.hetzner.de/100MB.bin`.
2. Ensure WiFi credentials in `lib/core/WiFiManager.hpp` are correct.
3. Build + flash, then open the serial monitor at 115200 baud.
4. Reset the ESP32 so the download runs and watch the monitor for the download/write lines.

Example outputs (from your runs):

- schedule.txt (small file) — functional check

```
fetching url: https://100daysofcode.s3-eu-west-1.amazonaws.com/schedule.txt
file size: 70 byte
writing to SPIFFS...
file written to spiffs
average download speed: 115.08 KB/s
avg write speed: 794.88 KB/s
```

- rfc2616.txt (larger text file used in your run)

```
fetching url: https://100daysofcode.s3-eu-west-1.amazonaws.com/rfc2616.txt
file size: 422279 byte
writing to SPIFFS...
file written to spiffs
average download speed: 14.23 KB/s
avg write speed: 35.62 KB/s
```

Notes:
- Small files (schedule.txt) validate correctness but are too short to measure sustained network throughput accurately.
- The `rfc2616.txt` run shows low download speed in this environment; causes may include server throttling, WiFi conditions, or TLS overhead on the server side.
- For a reliable measurement toward the 400 KB/s target, use a large binary hosted on a high-bandwidth server (see `speed.hetzner.de`) and run the test several times, averaging results.

Note: The repository prints the computed speeds (download and write) to Serial using `Serial.printf()`; use those values for validation.

Measuring and validating 400 KB/s
---------------------------------
- The code computes download speed as: totalReceived (bytes) / duration (seconds) -> KB/s. This is printed as `average download speed`.
- For a reliable measurement:
  - Use a large file (tens of MB) to amortize connection setup and startup overhead.
  - Re-run the test several times and average the results.
  - Verify both download and write speeds — if write is lower than 400 KB/s it can throttle the effective throughput.

Actual serial output (concise):

```
Phase 1 completed!you are connected to : realme 6
IP address172.31.82.82WIFI RSSI-18dbm

 fetching url: https://100daysofcode.s3-eu-west-1.amazonaws.com/schedule.txt
HTTP begin..
HTTP GET code: 200
file size: 70 byte
SPIFFS total  space: 1318001 bytes
 used space: 753 bytes
 free space: 1318001 bytes
writing to SPIFFS...
file written to spiffs
file size wrrtten: 70 bytes
average download speed: 115.08 KB/s
avg write speed: 794.88 KB/s
```

Measured speeds:
- Download: 115.08 KB/s (does NOT meet 400 KB/s target)
- Write to SPIFFS: 794.88 KB/s

Test with a larger file (example)

If you change the URL in `lib/core/HttpsClient.hpp` to:

`https://100daysofcode.s3-eu-west-1.amazonaws.com/rfc2616.txt`

the serial output looks like:

```
Phase 1 completed!you are connected to : realme 6
IP address172.31.82.82WIFI RSSI-18dbm

 fetching url: https://100daysofcode.s3-eu-west-1.amazonaws.com/rfc2616.txt
HTTP begin..
HTTP GET code: 200
file size: 422279 byte
SPIFFS total  space: 1318001 bytes
 used space: 388799 bytes
 free space: 1318001 bytes
writing to SPIFFS...
file written to spiffs
file size wrrtten: 422279 bytes
average download speed: 14.23 KB/s
avg write speed: 35.62 KB/s
```

Measured speeds for this run:
- Download: 14.23 KB/s
- Write to SPIFFS: 35.62 KB/s

Conclusion: Using this host and network, both download and write speeds are well below the 400 KB/s target — investigate network bandwidth, server throttling, WiFi signal, or try a different high-bandwidth host for accurate throughput testing.

Error handling implemented in the code
-------------------------------------
- SPIFFS mount failure check in `setup()` returns early and prints an error.
- WiFi connection loop blocks until connected; it prints progress messages.
- HTTPS client: checks `http.begin()` success, `http.GET()` return code and `HTTP_CODE_OK` (200). On failure, prints error messages and calls `http.end()`.
- Storage space check: `space(len)` attempts to compare file size vs free SPIFFS space and aborts if not enough space.
- File open for writing is checked; if `SPIFFS.open` fails, code reports and returns.

Known issues and suggested fixes
-------------------------------
1) freeBytes calculation bug in `lib/core/HttpsClient.hpp`:
   - Current code:
     size_t totalBytes = SPIFFS.totalBytes();
     size_t usedBytes = SPIFFS.usedBytes();
     size_t freeBytes = totalBytes - freeBytes; // BUG
   - Fix:
     size_t freeBytes = totalBytes - usedBytes;

2) TLS security:
   - `client.setInsecure()` disables certificate validation. For production, prefer validating the server certificate or using certificate pinning.

3) Configurable URL via Serial or Web UI (enhancement):
   - Currently the URL is a compile-time constant. You could add a simple serial menu or a small web server to allow runtime changes.

4) SPIFFS vs LittleFS:
   - Consider using LittleFS (if supported) for better wear leveling and performance on newer Arduino cores.

5) Partition/Flash tuning:
   - If SPIFFS writes are the bottleneck, check partition size, flash frequency and mode in `platformio.ini` and the board configuration.

Small code cleanup suggestions
----------------------------
- Make `ssid`, `password`, `url`, and `filename` non-global or place in a `config.h` to centralize configuration.
- Add timeouts on the WiFi connect loop to avoid indefinite blocking.
- Add retries for HTTP failures with exponential backoff.

Files changed / created by this README
-------------------------------------
- `README.md` (this file) — documentation, test instructions, and suggested fixes.

Next steps / troubleshooting
---------------------------
- If measured speeds are below target:
  - Move the ESP32 closer to the AP or try a less congested WiFi channel.
  - Increase the buffer size in `lib/core/HttpsClient.hpp` (watch RAM usage).
  - Ensure the file server can deliver the requested throughput (use a well-known speed test host).
  - Check SPIFFS write speeds by running a local write microbenchmark (write large blocks to SPIFFS and measure write throughput).

Contact
-------
For follow-ups, say what measurement logs you get from the serial monitor and I can suggest targeted fixes or prepare a small code patch to improve throughput.


Acknowledgements
----------------
This README was generated from the source files in the repository: `src/main.cpp`, and `lib/core/*.hpp`.

