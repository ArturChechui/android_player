# Android Native Audio Player (`libplayer`)

A high-performance Android C++ Native Audio Player demonstrating lock-free concurrency, low-latency audio streaming with **AAudio**, and software MP3 decoding using **dr_mp3**.

---

## Architecture Overview

The system is decoupled using **SOLID principles** and the **Facade pattern** to separate network I/O, audio decoding, and real-time playback into distinct pipeline stages.

```text
[ Kotlin UI ] ──JNI Bridge──► [ AudioEngine Facade ]
                                     │
           ┌─────────────────────────┴─────────────────────────┐
           ▼                                                   ▼
  [ HttpDataSource ]                                  [ AAudio Engine ]
   (POSIX Sockets)                                    (Low-Latency HAL)
           │                                                   ▲
           ▼                                                   │
   [ AudioDecoder ] ──(Decoded PCM)──► [ LockFreeRingBuffer ] ─┘
     (dr_mp3 API)                         (SPSC / Atomics)

```

* **`HttpDataSource`**: Manages POSIX network sockets and streams raw HTTP bytes over TCP.
* **`AudioDecoder`**: Uses `dr_mp3` to decode compressed MP3 radio streams into 16-bit PCM frames in real-time.
* **`LockFreeRingBuffer`**: Single-Producer Single-Consumer (SPSC) circular buffer utilizing 64-bit atomic counters for thread-safe lock-free synchronization between decoding and audio rendering threads.
* **`AudioEngine`**: High-level facade interfacing with AAudio, managing volume scaling, mute states, and output stream life cycles.

---

## Tech Stack

* **Language**: C++17, Kotlin
* **NDK Libraries**: AAudio, POSIX Sockets
* **Decoding**: `dr_mp3` (header-only C decoder)
* **Build System**: CMake, NDK Clang

---

## Key Features

* **Lock-Free Concurrency**: Zero mutexes on the audio thread to eliminate priority inversions and audio glitches (XRUNs).
* **Decoupled SOLID Design**: Network, decoding, and rendering components function independently.
* **Real-time Volume & Mute**: Real-time gain scaling with optimized bypass logic when volume is at 100%.

---

## Setup & Build

1. **Clone the repository**:
```bash
git clone https://github.com/ArturChechui/android_player.git

```
2. **Open in Android Studio**:
Open the project folder and sync Gradle with NDK installed (v25+ recommended).
3. **Build & Run**:
Select an active emulator or physical device running Android 8.0+ (API 26+) and press **Run**.
