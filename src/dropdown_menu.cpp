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
            width = std::max(width, (int)item.text.length() + 8);  // More padding
        }
    }
    width = std::max(width, 20);  // Larger minimum width
    
    height = items.size() + 2;  // Items + top/bottom borders
    if (height < 3) height = 3;  // Minimum height
}

void DropdownMenu::addItem(const std::string& text, MenuCallback callback) {
    items.emplace_back(text, callback, true, false);
    calculateDimensions();
}

void DropdownMenu::addSeparator() {
    items.emplace_back("", nullptr, false, true);
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
    
    std::string borderColor = Color::BRIGHT_YELLOW;
    std::string bgColor = Color::BLACK + Color::BG_BRIGHT_WHITE;
    std::string selectedColor = Color::BRIGHT_WHITE + Color::BG_MAGENTA;
    std::string disabledColor = Color::BLACK + Color::BG_WHITE;
    
    // Menu appears below the trigger
    int menuY = triggerY + 1;
    
    // Draw menu background and border
    buffer.drawBox(x, menuY, width, height, borderColor, false, false);
    
    // Fill menu background
    for (int row = 1; row < height - 1; row++) {
        for (int col = 1; col < width - 1; col++) {
            buffer.setCell(x + col, menuY + row, " ", bgColor);
        }
    }
    
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
            // Determine item color
            std::string itemColor = bgColor;
            if (i == selectedIndex && item.enabled) {
                itemColor = selectedColor;
            } else if (!item.enabled) {
                itemColor = disabledColor;
            }
            
            // Draw item background
            for (int col = 1; col < width - 1; col++) {
                buffer.setCell(x + col, itemY, " ", itemColor);
            }
            
            // Draw item text
            std::string displayText = " " + item.text;
            buffer.drawStringClipped(x + 1, itemY, displayText, itemColor, x + width - 1);
            
            // Add checkmark or indicator if needed
            if (item.enabled && i == selectedIndex) {
                buffer.setCell(x + width - 2, itemY, Unicode::TRIANGLE_RIGHT, itemColor);
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