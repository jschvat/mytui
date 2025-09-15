#include "../include/dropdown_menu.h"
#include <algorithm>

DropdownMenu::DropdownMenu(int x, int y, const std::string& title)
    : x(x), y(y), title(title), visible(true), active(false), 
      selectedIndex(-1), wasLeftPressed(false), menuOpen(false) {
    calculateDimensions();
}

void DropdownMenu::calculateDimensions() {
    // Calculate trigger dimensions
    triggerWidth = title.length() + 4;  // Add padding and dropdown arrow
    triggerHeight = 1;
    triggerX = x;
    triggerY = y;
    
    // Calculate menu dimensions
    width = triggerWidth;
    for (const auto& item : items) {
        if (!item.separator) {
            width = std::max(width, (int)item.text.length() + 4);
        }
    }
    width = std::max(width, 12);  // Minimum width
    
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
        Color::BLACK + Color::BG_BRIGHT_CYAN : 
        Color::WHITE + Color::BG_BLUE;
    
    // Draw trigger button with dropdown arrow
    std::string displayText = " " + title + " " + Unicode::TRIANGLE_DOWN;
    buffer.drawString(triggerX, triggerY, displayText, triggerColor);
}

void DropdownMenu::drawMenu(UnicodeBuffer& buffer) {
    if (!menuOpen) return;
    
    std::string borderColor = Color::BRIGHT_CYAN;
    std::string bgColor = Color::BLACK + Color::BG_WHITE;
    std::string selectedColor = Color::WHITE + Color::BG_BLUE;
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
    return mx >= x && mx < x + width && my >= menuY && my < menuY + height;
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