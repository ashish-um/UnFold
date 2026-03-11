# Unfold

A spatial file explorer that visualizes your filesystem as an interactive node graph on an infinite zoomable canvas.

## Prerequisites

### Windows (MSYS2 UCRT64)
Install the required packages using `pacman`:
```bash
pacman -S mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-svg \
  mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-gcc
```

### Linux (Arch Linux / EndeavourOS)
Install the required packages using `pacman`:
```bash
sudo pacman -S cmake ninja qt6-base qt6-svg qt6-tools gcc
```

### Linux (Ubuntu / Debian)
Install the required packages using `apt`:
```bash
sudo apt update
sudo apt install cmake ninja-build qt6-base-dev qt6-svg-dev qt6-tools-dev-tools build-essential
```

## Build and Run

### Windows (PowerShell / CMD)
Make sure MSYS2 binaries are in your PATH.
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
mkdir build; cd build
cmake -G Ninja ..
ninja
.\Unfold.exe
```

### Linux
```bash
mkdir -p build && cd build
cmake -G Ninja ..
ninja
./Unfold
```

## TODO:
- [ ] Add Tests
- [ ] All necessary right menu items
- [ ] Copy paste functionality
