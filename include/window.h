#pragma once

#include "buffer.h"
#include "mouse_handler.h"
#include <string>

class Window {
public:
    int x, y, w, h;
    std::string title;
    bool active, dragging, resizing, visible;
    int dragOffsetX, dragOffsetY;
    int moveCount, resizeCount;
    
    enum { MIN_WIDTH = 15, MIN_HEIGHT = 6 };
    bool wasLeftPressed = false;
    
    Window(int x, int y, int w, int h, const std::string& title);
    
    void draw(UnicodeBuffer& buffer);
    void updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight);
    
    bool titleContains(int mx, int my) const;
    bool closeButtonContains(int mx, int my) const;
    bool resizeHandleContains(int mx, int my) const;
    bool contains(int mx, int my) const;
    
    void close() { visible = false; }
    void show() { visible = true; }
    bool isVisible() const { return visible; }
};