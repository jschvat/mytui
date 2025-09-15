#include "../include/dropdown_menu.h"
#include <algorithm>

DropdownMenu::DropdownMenu(int x, int y, const std::string& title)
    : x(x), y(y), title(title), visible(true), active(false), 
      selectedIndex(-1), wasLeftPressed(false), menuOpen(false) {
    calculateDimensions();
}

void DropdownMenu::calculateDimensions() {
    // Calculate trigger dimensions
    triggerWidth = title.length() + 6;  // Add more padding, no dropdown arrow
    triggerHeight = 1;
    triggerX = x;
    triggerY = y;
    
    // Calculate menu dimensions with generous padding
    width = triggerWidth + 8;  // Add extra width
    for (const auto& item : items) {
        if (!item.separator) {
            int itemWidth = item.text.length() + 8;  // Base padding
            if (!item.shortcut.empty()) {
                itemWidth += item.shortcut.length() + 4;  // Extra space for shortcut + padding
            }
            width = std::max(width, itemWidth);
        }
    }
    width = std::max(width, 20);  // Larger minimum width
    
    height = items.size() + 2;  // Items + top/bottom borders
    if (height < 3) height = 3;  // Minimum height
}

void DropdownMenu::addItem(const std::string& text, MenuCallback callback) {
    items.emplace_back(text, "", callback, true, false);
    calculateDimensions();
}

void DropdownMenu::addItem(const std::string& text, const std::string& shortcut, MenuCallback callback) {
    items.emplace_back(text, shortcut, callback, true, false);
    calculateDimensions();
}

void DropdownMenu::addSeparator() {
    items.emplace_back("", "", nullptr, false, true);
    calculateDimensions();
}

void DropdownMenu::clearItems() {
    items.clear();
    selectedIndex = -1;
    calculateDimensions();
}

void DropdownMenu::drawTrigger(UnicodeBuffer& buffer) {
    std::string triggerColor = active ? 
        Color::BLACK + Color::BG_BRIGHT_WHITE : 
        Color::BRIGHT_WHITE + Color::BG_BLACK;
    
    // Draw trigger button without dropdown arrow, with more padding
    std::string displayText = "  " + title + "  ";
    buffer.drawString(triggerX, triggerY, displayText, triggerColor);
}

void DropdownMenu::drawMenu(UnicodeBuffer& buffer) {
    if (!menuOpen) return;
    
    std::string borderColor = Color::ORANGE + Color::BG_BLACK;  // Orange borders on black background
    std::string bgColor = Color::BRIGHT_WHITE + Color::BG_BLACK;     // Match menu bar
    std::string selectedColor = Color::BLACK + Color::BG_BRIGHT_WHITE;  // Inverse for selection
    std::string disabledColor = Color::CYAN + Color::BG_BLACK;      // Dimmed on same background
    
    // Menu appears below the trigger
    int menuY = triggerY + 1;
    
    // Fill entire menu area with background color first
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            buffer.setCell(x + col, menuY + row, " ", bgColor);
        }
    }
    
    // Draw menu border using single-line box drawing over the background
    buffer.drawBox(x, menuY, width, height, borderColor, true, false);
    
    // Draw menu items
    int itemY = menuY + 1;
    for (int i = 0; i < (int)items.size(); i++) {
        const auto& item = items[i];
        
        if (item.separator) {
            // Draw separator line
            for (int col = 1; col < width - 1; col++) {
                buffer.setCell(x + col, itemY, Unicode::HORIZONTAL, borderColor);
            }
        } else {
            // Determine text color
            std::string textColor = bgColor;
            if (i == selectedIndex && item.enabled) {
                textColor = selectedColor;
            } else if (!item.enabled) {
                textColor = disabledColor;
            }
            
            // Draw highlighted background for entire row including borders
            if (i == selectedIndex && item.enabled) {
                // Highlight the entire row width including borders
                for (int col = 0; col < width; col++) {
                    std::string currentChar = " ";
                    if (col == 0) {
                        currentChar = Unicode::VERTICAL;  // Left border
                    } else if (col == width - 1) {
                        currentChar = Unicode::VERTICAL;  // Right border
                    }
                    buffer.setCell(x + col, itemY, currentChar, selectedColor);
                }
            }
            
            // Draw item text (only if not already highlighted)
            if (!(i == selectedIndex && item.enabled)) {
                std::string displayText = "  " + item.text;
                buffer.drawStringClipped(x + 1, itemY, displayText, textColor, x + width - 2);
                
                // Draw shortcut key if present (right-aligned)
                if (!item.shortcut.empty()) {
                    int shortcutX = x + width - item.shortcut.length() - 3;  // Right-align with padding
                    buffer.drawStringClipped(shortcutX, itemY, item.shortcut, textColor, x + width - 2);
                }
            } else {
                // Draw text over highlighted background
                std::string displayText = "  " + item.text;
                buffer.drawStringClipped(x + 1, itemY, displayText, selectedColor, x + width - 2);
                
                // Draw shortcut key if present (right-aligned)
                if (!item.shortcut.empty()) {
                    int shortcutX = x + width - item.shortcut.length() - 3;  // Right-align with padding
                    buffer.drawStringClipped(shortcutX, itemY, item.shortcut, selectedColor, x + width - 2);
                }
            }
        }
        itemY++;
    }
}

void DropdownMenu::draw(UnicodeBuffer& buffer) {
    if (!visible) return;
    
    drawTrigger(buffer);
    drawMenu(buffer);
}

bool DropdownMenu::triggerContains(int mx, int my) const {
    return mx >= triggerX && mx < triggerX + triggerWidth && my == triggerY;
}

bool DropdownMenu::menuContains(int mx, int my) const {
    if (!menuOpen) return false;
    int menuY = triggerY + 1;
    // Use the same adjustment logic as in updateMouse for consistency
    int adjustedX = x;
    return mx >= adjustedX && mx < adjustedX + width && my >= menuY && my < menuY + height;
}

int DropdownMenu::getItemAtPosition(int mx, int my) const {
    if (!menuOpen || !menuContains(mx, my)) return -1;
    
    int menuY = triggerY + 1;
    int itemIndex = my - (menuY + 1);  // Adjust for menu border
    
    if (itemIndex >= 0 && itemIndex < (int)items.size()) {
        const auto& item = items[itemIndex];
        if (!item.separator && item.enabled) {
            return itemIndex;
        }
    }
    return -1;
}

void DropdownMenu::updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight) {
    if (!visible) return;
    
    // Adjust menu position if it would go off screen
    int adjustedX = x;
    if (x + width > termWidth) {
        adjustedX = termWidth - width;
    }
    if (adjustedX < 0) adjustedX = 0;
    
    int mouseX = mouse.getMouseX();
    int mouseY = mouse.getMouseY();
    bool leftPressed = mouse.isLeftButtonPressed();
    
    // Reset active state
    active = false;
    
    if (leftPressed && !wasLeftPressed) {
        // Mouse clicked
        if (triggerContains(mouseX, mouseY)) {
            // Toggle menu
            menuOpen = !menuOpen;
            selectedIndex = -1;
            active = true;
        } else if (menuOpen) {
            int itemIndex = getItemAtPosition(mouseX, mouseY);
            if (itemIndex >= 0) {
                // Item clicked
                executeCallback(itemIndex);
                menuOpen = false;
                selectedIndex = -1;
            } else if (!menuContains(mouseX, mouseY)) {
                // Clicked outside menu - close it
                menuOpen = false;
                selectedIndex = -1;
            }
        }
    } else if (menuOpen) {
        // Update selection based on mouse hover
        int itemIndex = getItemAtPosition(mouseX, mouseY);
        selectedIndex = itemIndex;
        
        if (triggerContains(mouseX, mouseY) || menuContains(mouseX, mouseY)) {
            active = true;
        }
    } else if (triggerContains(mouseX, mouseY)) {
        // Hover over trigger
        active = true;
    }
    
    wasLeftPressed = leftPressed;
}

void DropdownMenu::executeCallback(int index) {
    if (index >= 0 && index < (int)items.size()) {
        const auto& item = items[index];
        if (item.enabled && item.callback) {
            item.callback();
        }
    }
}

// Static utility for drawing menu bar background
void DropdownMenu::drawMenuBar(UnicodeBuffer& buffer, int y, int termWidth) {
    std::string barColor = Color::BRIGHT_WHITE + Color::BG_BLACK;
    
    // Draw horizontal menu bar background across entire screen
    for (int x = 0; x < termWidth; x++) {
        buffer.setCell(x, y, " ", barColor);
    }
}