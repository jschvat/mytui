#include "../include/window.h"
#include <algorithm>

Window::Window(int x, int y, int w, int h, const std::string& title)
    : x(x), y(y), w(w), h(h), title(title), active(false), dragging(false), 
      resizing(false), visible(true), dragOffsetX(0), dragOffsetY(0), 
      moveCount(0), resizeCount(0), scrollX(0), scrollY(0), 
      contentWidth(0), contentHeight(0), enableScrollbars(true),
      draggingVerticalThumb(false), draggingHorizontalThumb(false), dragThumbOffset(0) {}

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
    
    // Calculate content area dimensions (account for scrollbars)
    int contentAreaWidth = w - 2;  // Account for left/right borders
    int contentAreaHeight = h - 2; // Account for top/bottom borders
    
    if (needsVerticalScrollbar()) contentAreaWidth--;
    if (needsHorizontalScrollbar()) contentAreaHeight--;
    
    // Content area background (stop before borders and scrollbars)
    for (int row = 1; row < h - 1; row++) {
        for (int col = 1; col < w - 1; col++) {
            if (needsVerticalScrollbar() && col == w - 2) continue;   // Skip vertical scrollbar column
            if (needsHorizontalScrollbar() && row == h - 2) continue; // Skip horizontal scrollbar row
            buffer.setCell(x + col, y + row, " ", contentColor);
        }
    }
    
    // Draw scrollable content
    if (!content.empty()) {
        for (int row = 0; row < contentAreaHeight; row++) {
            int contentRow = row + scrollY;
            if (contentRow >= 0 && contentRow < (int)content.size()) {
                std::string line = content[contentRow];
                
                // Apply horizontal scroll
                if (scrollX < (int)line.length()) {
                    std::string visiblePart = line.substr(scrollX);
                    buffer.drawStringClipped(x + 1, y + 1 + row, visiblePart, contentColor, x + 1 + contentAreaWidth);
                }
            }
        }
    } else {
        // Default content when no scrollable content is set
        buffer.drawStringClipped(x + 2, y + 2, Unicode::BULLET + " " + title, Color::BRIGHT_BLUE + Color::BG_WHITE, x + w - 2);
        buffer.drawStringClipped(x + 2, y + 3, Unicode::ARROW_RIGHT + " Size: " + std::to_string(w) + "x" + std::to_string(h), contentColor, x + w - 2);
        
        if (h > 5) {
            buffer.drawStringClipped(x + 2, y + 4, Unicode::TRIANGLE_UP + " Moves: " + std::to_string(moveCount), contentColor, x + w - 2);
        }
        if (h > 6) {
            buffer.drawStringClipped(x + 2, y + 5, Unicode::TRIANGLE_DOWN + " Resizes: " + std::to_string(resizeCount), contentColor, x + w - 2);
        }
        
        // Status with Unicode indicators
        if (h > 7) {
            std::string status = active ? Unicode::CHECK + " ACTIVE" : Unicode::CIRCLE + " Inactive";
            if (dragging) status = Unicode::TRIANGLE_RIGHT + " DRAGGING";
            if (resizing) status = Unicode::TRIANGLE_UP + " RESIZING";
            buffer.drawStringClipped(x + 2, y + h - 3, status, Color::BRIGHT_WHITE + Color::BG_BLUE, x + w - 2);
        }
        
        // Debug scrollbar info
        if (h > 8) {
            std::string debugInfo = "Content: " + std::to_string(contentWidth) + "x" + std::to_string(contentHeight) + 
                                   " V:" + (needsVerticalScrollbar() ? "Y" : "N") + 
                                   " H:" + (needsHorizontalScrollbar() ? "Y" : "N");
            buffer.drawStringClipped(x + 2, y + h - 4, debugInfo, Color::BRIGHT_YELLOW + Color::BG_BLUE, x + w - 2);
        }
    }
    
    // Draw scrollbars if needed (show for all visible windows, not just active)
    if (enableScrollbars) {
        drawScrollbars(buffer);
    }
    
    // Resize handle in border corner (bottom-right)
    if (w > 6 && h > 3) {
        buffer.setCell(x + w - 1, y + h - 1, Unicode::RESIZE_HANDLE, borderColor);
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
        } else if (verticalThumbContains(mouseX, mouseY)) {
            // Start dragging vertical thumb
            active = true;
            draggingVerticalThumb = true;
            int scrollbarHeight = h - 2 - (needsHorizontalScrollbar() ? 1 : 0);
            int trackHeight = scrollbarHeight - 1;
            int thumbPos = 1;
            if (contentHeight > trackHeight) {
                thumbPos = 1 + (scrollY * (trackHeight - 1)) / std::max(1, contentHeight - trackHeight);
            }
            dragThumbOffset = mouseY - (y + 1 + thumbPos);
        } else if (horizontalThumbContains(mouseX, mouseY)) {
            // Start dragging horizontal thumb
            active = true;
            draggingHorizontalThumb = true;
            int scrollbarWidth = w - 2 - (needsVerticalScrollbar() ? 1 : 0);
            int trackWidth = scrollbarWidth - 1;
            int thumbPos = 1;
            if (contentWidth > trackWidth) {
                thumbPos = 1 + (scrollX * (trackWidth - 1)) / std::max(1, contentWidth - trackWidth);
            }
            dragThumbOffset = mouseX - (x + 1 + thumbPos);
        } else if (verticalScrollbarContains(mouseX, mouseY) || horizontalScrollbarContains(mouseX, mouseY)) {
            // Handle scrollbar button/track clicks
            active = true;
            handleScrollbarClick(mouseX, mouseY);
        }
    } else if (!leftPressed && wasLeftPressed) {
        dragging = false;
        resizing = false;
        draggingVerticalThumb = false;
        draggingHorizontalThumb = false;
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
    } else if (leftPressed && (draggingVerticalThumb || draggingHorizontalThumb)) {
        // Handle scrollbar thumb dragging
        handleScrollbarDrag(mouseX, mouseY);
    }
    
    wasLeftPressed = leftPressed;
}

// Content management methods
void Window::setContent(const std::vector<std::string>& newContent) {
    content = newContent;
    calculateContentDimensions();
    scrollX = 0;
    scrollY = 0;
}

void Window::addContentLine(const std::string& line) {
    content.push_back(line);
    calculateContentDimensions();
}

void Window::clearContent() {
    content.clear();
    contentWidth = 0;
    contentHeight = 0;
    scrollX = 0;
    scrollY = 0;
}

void Window::calculateContentDimensions() {
    contentHeight = content.size();
    contentWidth = 0;
    
    for (const auto& line : content) {
        contentWidth = std::max(contentWidth, (int)line.length());
    }
}

// Scrolling methods
void Window::scrollUp(int lines) {
    scrollY = std::max(0, scrollY - lines);
}

void Window::scrollDown(int lines) {
    int availableHeight = h - 2; // Account for borders
    if (needsHorizontalScrollbar()) availableHeight--; // Account for horizontal scrollbar
    int maxScrollY = std::max(0, contentHeight - availableHeight);
    scrollY = std::min(maxScrollY, scrollY + lines);
}

void Window::scrollLeft(int chars) {
    scrollX = std::max(0, scrollX - chars);
}

void Window::scrollRight(int chars) {
    int availableWidth = w - 2; // Account for borders
    if (needsVerticalScrollbar()) availableWidth--; // Account for vertical scrollbar
    int maxScrollX = std::max(0, contentWidth - availableWidth);
    scrollX = std::min(maxScrollX, scrollX + chars);
}

// Scrollbar helper methods
bool Window::needsVerticalScrollbar() const {
    if (!enableScrollbars || content.empty()) return false;
    // Check if content height exceeds available space, assuming horizontal scrollbar might exist
    int availableHeight = h - 2; // Account for top/bottom borders
    if (contentWidth > (w - 3)) availableHeight--; // Reserve space for horizontal scrollbar (need 3 chars: borders + scrollbar)
    return contentHeight > availableHeight;
}

bool Window::needsHorizontalScrollbar() const {
    if (!enableScrollbars || content.empty()) return false;
    // Check if content width exceeds available space, assuming vertical scrollbar might exist
    int availableWidth = w - 2; // Account for left/right borders
    if (contentHeight > (h - 3)) availableWidth--; // Reserve space for vertical scrollbar (need 3 chars: borders + scrollbar)
    return contentWidth > availableWidth;
}

void Window::drawScrollbars(UnicodeBuffer& buffer) {
    std::string trackColor = Color::BLACK + Color::BG_BLACK;
    std::string thumbColor = Color::WHITE + Color::BG_CYAN;
    std::string buttonColor = Color::BRIGHT_WHITE + Color::BG_BLUE;
    
    // Calculate scrollbar needs without circular dependency
    bool needsVert = needsVerticalScrollbar();
    bool needsHoriz = needsHorizontalScrollbar();
    
    // Vertical scrollbar (1 character before right border)
    if (needsVert) {
        int scrollbarX = x + w - 2;
        int scrollbarHeight = h - 2 - (needsHoriz ? 1 : 0);
        
        // Bounds checking
        if (scrollbarHeight > 2 && scrollbarX >= 0) {
            // Draw up arrow button
            buffer.setCell(scrollbarX, y + 1, Unicode::SCROLLBAR_BUTTON_UP, buttonColor);
            
            // Draw down arrow button
            buffer.setCell(scrollbarX, y + scrollbarHeight, Unicode::SCROLLBAR_BUTTON_DOWN, buttonColor);
            
            // Draw scrollbar track (between arrow buttons)
            int trackHeight = scrollbarHeight - 1;
            for (int i = 1; i < trackHeight; i++) {
                if (y + 1 + i >= 0) {
                    buffer.setCell(scrollbarX, y + 1 + i, Unicode::SCROLLBAR_TRACK, trackColor);
                }
            }
            
            // Calculate thumb position and size with bounds checking
            if (contentHeight > 0 && trackHeight > 1) {
                int thumbSize = std::max(1, std::min(trackHeight - 1, (trackHeight * trackHeight) / contentHeight));
                int maxThumbPos = trackHeight - 1 - thumbSize;
                int thumbPos = 1; // Start after up button
                
                if (contentHeight > trackHeight && maxThumbPos > 0) {
                    thumbPos = 1 + (scrollY * maxThumbPos) / std::max(1, contentHeight - trackHeight);
                    thumbPos = std::max(1, std::min(trackHeight - thumbSize, thumbPos));
                }
                
                // Draw thumb with bounds checking
                for (int i = 0; i < thumbSize; i++) {
                    if (y + 1 + thumbPos + i >= 0 && thumbPos + i < trackHeight) {
                        buffer.setCell(scrollbarX, y + 1 + thumbPos + i, Unicode::SCROLLBAR_THUMB, thumbColor);
                    }
                }
            }
        }
    }
    
    // Horizontal scrollbar (1 character before bottom border)
    if (needsHoriz) {
        int scrollbarY = y + h - 2;
        int scrollbarWidth = w - 2 - (needsVert ? 1 : 0);
        
        // Bounds checking
        if (scrollbarWidth > 2 && scrollbarY >= 0) {
            // Draw left arrow button
            buffer.setCell(x + 1, scrollbarY, Unicode::SCROLLBAR_BUTTON_LEFT, buttonColor);
            
            // Draw right arrow button
            buffer.setCell(x + scrollbarWidth, scrollbarY, Unicode::SCROLLBAR_BUTTON_RIGHT, buttonColor);
            
            // Draw scrollbar track (between arrow buttons)
            int trackWidth = scrollbarWidth - 1;
            for (int i = 1; i < trackWidth; i++) {
                if (x + 1 + i >= 0) {
                    buffer.setCell(x + 1 + i, scrollbarY, Unicode::SCROLLBAR_TRACK, trackColor);
                }
            }
            
            // Calculate thumb position and size with bounds checking
            if (contentWidth > 0 && trackWidth > 1) {
                int thumbSize = std::max(1, std::min(trackWidth - 1, (trackWidth * trackWidth) / contentWidth));
                int maxThumbPos = trackWidth - 1 - thumbSize;
                int thumbPos = 1; // Start after left button
                
                if (contentWidth > trackWidth && maxThumbPos > 0) {
                    thumbPos = 1 + (scrollX * maxThumbPos) / std::max(1, contentWidth - trackWidth);
                    thumbPos = std::max(1, std::min(trackWidth - thumbSize, thumbPos));
                }
                
                // Draw thumb with bounds checking
                for (int i = 0; i < thumbSize; i++) {
                    if (x + 1 + thumbPos + i >= 0 && thumbPos + i < trackWidth) {
                        buffer.setCell(x + 1 + thumbPos + i, scrollbarY, Unicode::SCROLLBAR_THUMB, thumbColor);
                    }
                }
            }
        }
    }
}

// Scrollbar mouse interaction methods
bool Window::verticalScrollbarContains(int mx, int my) const {
    if (!needsVerticalScrollbar()) return false;
    int scrollbarX = x + w - 2;
    int scrollbarHeight = h - 2 - (needsHorizontalScrollbar() ? 1 : 0);
    return mx == scrollbarX && my >= y + 1 && my <= y + scrollbarHeight;
}

bool Window::horizontalScrollbarContains(int mx, int my) const {
    if (!needsHorizontalScrollbar()) return false;
    int scrollbarY = y + h - 2;
    int scrollbarWidth = w - 2 - (needsVerticalScrollbar() ? 1 : 0);
    return my == scrollbarY && mx >= x + 1 && mx <= x + scrollbarWidth;
}

bool Window::verticalThumbContains(int mx, int my) const {
    if (!verticalScrollbarContains(mx, my)) return false;
    
    int scrollbarX = x + w - 2;
    int scrollbarHeight = h - 2 - (needsHorizontalScrollbar() ? 1 : 0);
    int trackHeight = scrollbarHeight - 1;
    
    if (contentHeight > 0 && trackHeight > 1) {
        int thumbSize = std::max(1, std::min(trackHeight - 1, (trackHeight * trackHeight) / contentHeight));
        int maxThumbPos = trackHeight - 1 - thumbSize;
        int thumbPos = 1;
        
        if (contentHeight > trackHeight && maxThumbPos > 0) {
            thumbPos = 1 + (scrollY * maxThumbPos) / std::max(1, contentHeight - trackHeight);
            thumbPos = std::max(1, std::min(trackHeight - thumbSize, thumbPos));
        }
        
        return my >= y + 1 + thumbPos && my < y + 1 + thumbPos + thumbSize;
    }
    return false;
}

bool Window::horizontalThumbContains(int mx, int my) const {
    if (!horizontalScrollbarContains(mx, my)) return false;
    
    int scrollbarY = y + h - 2;
    int scrollbarWidth = w - 2 - (needsVerticalScrollbar() ? 1 : 0);
    int trackWidth = scrollbarWidth - 1;
    
    if (contentWidth > 0 && trackWidth > 1) {
        int thumbSize = std::max(1, std::min(trackWidth - 1, (trackWidth * trackWidth) / contentWidth));
        int maxThumbPos = trackWidth - 1 - thumbSize;
        int thumbPos = 1;
        
        if (contentWidth > trackWidth && maxThumbPos > 0) {
            thumbPos = 1 + (scrollX * maxThumbPos) / std::max(1, contentWidth - trackWidth);
            thumbPos = std::max(1, std::min(trackWidth - thumbSize, thumbPos));
        }
        
        return mx >= x + 1 + thumbPos && mx < x + 1 + thumbPos + thumbSize;
    }
    return false;
}

void Window::handleScrollbarClick(int mx, int my) {
    // Vertical scrollbar buttons
    if (needsVerticalScrollbar()) {
        int scrollbarX = x + w - 2;
        if (mx == scrollbarX) {
            if (my == y + 1) {
                // Up button clicked
                scrollUp();
                return;
            }
            int scrollbarHeight = h - 2 - (needsHorizontalScrollbar() ? 1 : 0);
            if (my == y + scrollbarHeight) {
                // Down button clicked
                scrollDown();
                return;
            }
            
            // Track clicked - page up/down
            if (verticalScrollbarContains(mx, my) && !verticalThumbContains(mx, my)) {
                int trackHeight = scrollbarHeight - 1;
                int thumbPos = 1;
                if (contentHeight > trackHeight) {
                    thumbPos = 1 + (scrollY * (trackHeight - 1)) / std::max(1, contentHeight - trackHeight);
                }
                
                if (my < y + 1 + thumbPos) {
                    scrollUp(5); // Page up
                } else {
                    scrollDown(5); // Page down
                }
                return;
            }
        }
    }
    
    // Horizontal scrollbar buttons
    if (needsHorizontalScrollbar()) {
        int scrollbarY = y + h - 2;
        if (my == scrollbarY) {
            if (mx == x + 1) {
                // Left button clicked
                scrollLeft();
                return;
            }
            int scrollbarWidth = w - 2 - (needsVerticalScrollbar() ? 1 : 0);
            if (mx == x + scrollbarWidth) {
                // Right button clicked
                scrollRight();
                return;
            }
            
            // Track clicked - page left/right
            if (horizontalScrollbarContains(mx, my) && !horizontalThumbContains(mx, my)) {
                int trackWidth = scrollbarWidth - 1;
                int thumbPos = 1;
                if (contentWidth > trackWidth) {
                    thumbPos = 1 + (scrollX * (trackWidth - 1)) / std::max(1, contentWidth - trackWidth);
                }
                
                if (mx < x + 1 + thumbPos) {
                    scrollLeft(5); // Page left
                } else {
                    scrollRight(5); // Page right
                }
                return;
            }
        }
    }
}

void Window::handleScrollbarDrag(int mx, int my) {
    // Handle vertical thumb dragging
    if (draggingVerticalThumb && needsVerticalScrollbar()) {
        int scrollbarHeight = h - 2 - (needsHorizontalScrollbar() ? 1 : 0);
        int trackHeight = scrollbarHeight - 1;
        
        if (trackHeight > 1) {
            int thumbSize = std::max(1, std::min(trackHeight - 1, (trackHeight * trackHeight) / contentHeight));
            int maxThumbPos = trackHeight - 1 - thumbSize;
            int newThumbPos = (my - y - 1) - dragThumbOffset;
            newThumbPos = std::max(1, std::min(maxThumbPos + 1, newThumbPos));
            
            if (maxThumbPos > 0) {
                int newScrollY = ((newThumbPos - 1) * (contentHeight - trackHeight)) / maxThumbPos;
                scrollY = std::max(0, std::min(contentHeight - trackHeight, newScrollY));
            }
        }
    }
    
    // Handle horizontal thumb dragging
    if (draggingHorizontalThumb && needsHorizontalScrollbar()) {
        int scrollbarWidth = w - 2 - (needsVerticalScrollbar() ? 1 : 0);
        int trackWidth = scrollbarWidth - 1;
        
        if (trackWidth > 1) {
            int thumbSize = std::max(1, std::min(trackWidth - 1, (trackWidth * trackWidth) / contentWidth));
            int maxThumbPos = trackWidth - 1 - thumbSize;
            int newThumbPos = (mx - x - 1) - dragThumbOffset;
            newThumbPos = std::max(1, std::min(maxThumbPos + 1, newThumbPos));
            
            if (maxThumbPos > 0) {
                int newScrollX = ((newThumbPos - 1) * (contentWidth - trackWidth)) / maxThumbPos;
                scrollX = std::max(0, std::min(contentWidth - trackWidth, newScrollX));
            }
        }
    }
}