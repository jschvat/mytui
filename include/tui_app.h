#pragma once

#include "buffer.h"
#include "mouse_handler.h"
#include "window.h"
#include <vector>
#include <memory>
#include <sys/ioctl.h>

enum class CursorType {
    DEFAULT,
    POINTER,
    HAND,
    RESIZE,
    MOVE,
    TEXT,
    CROSSHAIR
};

class TUIApplication {
protected:
    FastMouseHandler mouse;
    UnicodeBuffer* buffer;
    std::vector<std::shared_ptr<Window>> windows;
    int term_width, term_height;
    int frame;
    
    // Cursor state
    CursorType current_cursor_type;
    int last_mouse_x, last_mouse_y;
    bool mouse_moved;
    
    void setupTerminal();
    void restoreTerminal();
    void updateTerminalSize();
    void drawBackground();
    void drawStatusBar();
    void drawMouseCursor();
    CursorType determineCursorType(int mouse_x, int mouse_y);
    
public:
    TUIApplication();
    ~TUIApplication();
    
    void addWindow(std::shared_ptr<Window> window);
    void removeWindow(std::shared_ptr<Window> window);
    virtual void run();
    void quit();
    
    int getTermWidth() const { return term_width; }
    int getTermHeight() const { return term_height; }
};