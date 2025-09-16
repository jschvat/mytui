#pragma once

#include "buffer.h"
#include "mouse_handler.h"
#include <string>
#include <vector>

class Window {
public:
    int x, y, w, h;
    std::string title;
    bool active, dragging, resizing, visible;
    int dragOffsetX, dragOffsetY;
    int moveCount, resizeCount;
    
    // Scrollable content support
    std::vector<std::string> content;
    int scrollX, scrollY;
    int contentWidth, contentHeight;
    bool enableScrollbars;
    
    enum { MIN_WIDTH = 15, MIN_HEIGHT = 6 };
    bool wasLeftPressed = false;
    
    // Scrollbar drag state
    bool draggingVerticalThumb = false;
    bool draggingHorizontalThumb = false;
    int dragThumbOffset = 0;
    
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
    
    // Content management
    void setContent(const std::vector<std::string>& newContent);
    void addContentLine(const std::string& line);
    void clearContent();
    void calculateContentDimensions();
    
    // Scrolling
    void scrollUp(int lines = 1);
    void scrollDown(int lines = 1);
    void scrollLeft(int chars = 1);
    void scrollRight(int chars = 1);
    
    // Scrollbar helpers
    bool needsVerticalScrollbar() const;
    bool needsHorizontalScrollbar() const;
    void drawScrollbars(UnicodeBuffer& buffer);
    
    // Scrollbar mouse interaction
    bool verticalScrollbarContains(int mx, int my) const;
    bool horizontalScrollbarContains(int mx, int my) const;
    bool verticalThumbContains(int mx, int my) const;
    bool horizontalThumbContains(int mx, int my) const;
    void handleScrollbarClick(int mx, int my);
    void handleScrollbarDrag(int mx, int my);
};