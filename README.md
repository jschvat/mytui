# Modern TUI Framework

A clean, class-based terminal user interface framework with Unicode box-drawing and advanced mouse support.

## Features

- **Modular Design**: Separate files for mouse handling, buffering, windows, and application logic
- **Unicode Box Drawing**: Professional appearance with double-line borders (╔╗╚╝═║)
- **Advanced Mouse Support**: Smooth dragging, resizing, and clicking with optimized input handling
- **Window Management**: Multiple resizable, draggable windows with proper z-ordering
- **Bracket-Style Close Buttons**: Easy-to-click `[█]` close buttons with red highlights
- **Visual States**: Different border styles for active, dragging, and resizing states

## Architecture

```
├── mouse_handler.{h,cpp}  # Fast mouse input processing and parsing
├── colors.h               # ANSI color codes and Unicode symbols
├── buffer.{h,cpp}         # Screen buffer with Unicode support
├── window.{h,cpp}         # Window class with drag/resize/close
├── tui_app.{h,cpp}        # Main application framework
└── demo.cpp              # Example usage
```

## Building

```bash
make
./demo
```

## Usage

```cpp
#include "tui_app.h"

int main() {
    TUIApplication app;
    
    auto window = std::make_shared<Window>(10, 5, 40, 12, "My Window");
    app.addWindow(window);
    
    app.run();
    return 0;
}
```

## Controls

- **Drag**: Click and drag title bar to move windows
- **Resize**: Click and drag `#` in bottom-right corner
- **Close**: Click the red `[█]` button in title bar
- **Quit**: Press `Q` key

## Design Principles

- **No Dependencies**: Pure C++ with ANSI escape sequences, no ncurses
- **Single-Width Characters**: All UI elements use single-width characters for perfect alignment
- **Efficient Rendering**: Optimized buffer system with minimal screen updates
- **Clean Separation**: Mouse handling, rendering, and window logic are cleanly separated