#include "../include/window.h"
#include <algorithm>

Window::Window(int x, int y, int w, int h, const std::string& title)
    : x(x), y(y), w(w), h(h), title(title), active(false), dragging(false), 
      resizing(false), visible(true), dragOffsetX(0), dragOffsetY(0), 
      moveCount(0), resizeCount(0) {}

void Window::draw(UnicodeBuffer& buffer) {
    if (!visible) return;
    
    std::string borderColor, titleBgColor, titleFgColor, contentColor, shadowColor;
    bool heavy = false, rounded = false;
    
    if (resizing) {
        borderColor = Color::BRIGHT_MAGENTA;
        titleBgColor = Color::BG_MAGENTA;
        titleFgColor = Color::BRIGHT_WHITE;
        heavy = true;
    } else if (dragging) {
        borderColor = Color::BRIGHT_YELLOW;
        titleBgColor = Color::BG_YELLOW;
        titleFgColor = Color::BLACK;
        heavy = true;
    } else if (active) {
        borderColor = Color::BRIGHT_CYAN;
        titleBgColor = Color::BG_BRIGHT_CYAN;
        titleFgColor = Color::BLACK;
        rounded = true;
    } else {
        borderColor = Color::CYAN;
        titleBgColor = Color::BG_CYAN;
        titleFgColor = Color::BLACK;
    }
    
    contentColor = Color::BLACK + Color::BG_WHITE;
    shadowColor = Color::BLACK + Color::BG_BLACK;
    
    // Draw solid black shadow directly adjacent to window (no gap)
    // Right edge shadow
    for (int i = 1; i < h; i++) {
        if (x + w < 200 && y + i < 100) {
            buffer.setCell(x + w, y + i, Unicode::FULL_BLOCK, shadowColor);
        }
    }
    // Bottom edge shadow
    for (int i = 1; i < w; i++) {
        if (x + i < 200 && y + h < 100) {
            buffer.setCell(x + i, y + h, Unicode::FULL_BLOCK, shadowColor);
        }
    }
    // Bottom-right corner shadow
    if (x + w < 200 && y + h < 100) {
        buffer.setCell(x + w, y + h, Unicode::FULL_BLOCK, shadowColor);
    }
    
    // Draw main window box with style variations
    buffer.drawBox(x, y, w, h, borderColor, rounded, heavy);
    
    // Title bar background
    for (int i = 1; i < w - 1; i++) {
        buffer.setCell(x + i, y, " ", titleBgColor);
    }
    
    // Window title with Unicode elements
    std::string displayTitle = resizing ? " " + Unicode::RESIZE_HANDLE + " " + title + " " : 
                              dragging ? " " + Unicode::TRIANGLE_RIGHT + " " + title + " " : 
                              " " + Unicode::DIAMOND + " " + title + " ";
    
    int titleLen = std::min((int)displayTitle.length(), w - 8);
    buffer.drawStringClipped(x + 2, y, displayTitle.substr(0, titleLen), titleFgColor + titleBgColor, x + w - 4);
    
    // Close button with bracket style [█] with red background
    buffer.setCell(x + w - 4, y, "[", titleFgColor + titleBgColor);
    buffer.setCell(x + w - 3, y, Unicode::FULL_BLOCK, Color::BRIGHT_RED + Color::BG_RED);
    buffer.setCell(x + w - 2, y, "]", titleFgColor + titleBgColor);
    
    // Content area
    for (int row = 1; row < h - 1; row++) {
        for (int col = 1; col < w - 1; col++) {
            if (row >= h - 2 && col >= w - 2) continue; // Skip resize area
            buffer.setCell(x + col, y + row, " ", contentColor);
        }
    }
    
    // Resize handle indicators (don't overwrite border corners)
    if (w > 15 && h > 6) {
        buffer.setCell(x + w - 2, y + h - 2, Unicode::RESIZE_HANDLE, Color::BRIGHT_GREEN + Color::BG_WHITE);
    }
    
    // Content with Unicode bullets and symbols (clipped to not overwrite borders)
    buffer.drawStringClipped(x + 2, y + 2, Unicode::BULLET + " " + title, Color::BRIGHT_BLUE + Color::BG_WHITE, x + w - 1);
    buffer.drawStringClipped(x + 2, y + 3, Unicode::ARROW_RIGHT + " Size: " + std::to_string(w) + "x" + std::to_string(h), contentColor, x + w - 1);
    
    if (h > 5) {
        buffer.drawStringClipped(x + 2, y + 4, Unicode::TRIANGLE_UP + " Moves: " + std::to_string(moveCount), contentColor, x + w - 1);
    }
    if (h > 6) {
        buffer.drawStringClipped(x + 2, y + 5, Unicode::TRIANGLE_DOWN + " Resizes: " + std::to_string(resizeCount), contentColor, x + w - 1);
    }
    
    // Status with Unicode indicators
    if (h > 7) {
        std::string status = active ? Unicode::CHECK + " ACTIVE" : Unicode::CIRCLE + " Inactive";
        if (dragging) status = Unicode::TRIANGLE_RIGHT + " DRAGGING";
        if (resizing) status = Unicode::RESIZE_HANDLE + " RESIZING";
        buffer.drawStringClipped(x + 2, y + h - 3, status, Color::BRIGHT_WHITE + Color::BG_BLUE, x + w - 1);
    }
}

bool Window::titleContains(int mx, int my) const {
    return mx >= x + 1 && mx < x + w - 6 && my == y;
}

bool Window::closeButtonContains(int mx, int my) const {
    // Make close button easier to click - entire right portion of title bar
    return mx >= x + w - 6 && mx <= x + w - 1 && my == y;
}

bool Window::resizeHandleContains(int mx, int my) const {
    // Check the bottom-right corner area for resizing
    return ((mx >= x + w - 3 && mx <= x + w - 1) &&
            (my >= y + h - 3 && my <= y + h - 1));
}

bool Window::contains(int mx, int my) const {
    return mx >= x && mx < x + w && my >= y && my < y + h;
}

void Window::updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight) {
    if (!visible) return;
    
    int mouseX = mouse.getMouseX();
    int mouseY = mouse.getMouseY();
    bool leftPressed = mouse.isLeftButtonPressed();
    
    if (leftPressed && !wasLeftPressed) {
        if (closeButtonContains(mouseX, mouseY)) {
            visible = false;
        } else if (resizeHandleContains(mouseX, mouseY)) {
            active = true;
            resizing = true;
        } else if (titleContains(mouseX, mouseY)) {
            active = true;
            dragging = true;
            dragOffsetX = mouseX - x;
            dragOffsetY = mouseY - y;
        }
    } else if (!leftPressed && wasLeftPressed) {
        dragging = false;
        resizing = false;
    } else if (leftPressed && dragging) {
        int newX = std::max(0, std::min(mouseX - dragOffsetX, termWidth - w - 2));
        int newY = std::max(0, std::min(mouseY - dragOffsetY, termHeight - h - 2));
        
        if (newX != x || newY != y) {
            x = newX;
            y = newY;
            moveCount++;
        }
    } else if (leftPressed && resizing) {
        int newW = std::max((int)MIN_WIDTH, mouseX - x + 1);
        int newH = std::max((int)MIN_HEIGHT, mouseY - y + 1);
        
        newW = std::min(newW, termWidth - x);
        newH = std::min(newH, termHeight - y);
        
        if (newW != w || newH != h) {
            w = newW;
            h = newH;
            resizeCount++;
        }
    }
    
    wasLeftPressed = leftPressed;
}