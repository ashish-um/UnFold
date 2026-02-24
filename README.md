# Unfold

A spatial file explorer that visualizes your filesystem as an interactive node graph on an infinite zoomable canvas.

## Build

Requires MSYS2 UCRT64 with Qt6:

```bash
pacman -S mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-svg \
  mingw-w64-ucrt-x86_64-qt6-tools
```

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
mkdir build; cd build
cmake -G Ninja ..
ninja
.\Unfold.exe
```

## TODO:
- [ ] Add Tests
- [ ] All necessary right menu items
- [ ] Copy paste functionality
