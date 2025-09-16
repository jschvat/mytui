#include "../include/dropdown_menu.h"
#include <algorithm>
#include <memory>

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
            
            // Draw highlighted background for content area (excluding borders)
            if (i == selectedIndex && item.enabled) {
                // Highlight the content area between borders (x+1 to x+width-2)
                for (int col = 1; col < width - 1; col++) {
                    buffer.setCell(x + col, itemY, " ", selectedColor);
                }
            }
            
            // Draw item text
            std::string displayText = "  " + item.text;
            std::string finalTextColor = textColor;
            
            // Use selected color for highlighted items
            if (i == selectedIndex && item.enabled) {
                finalTextColor = selectedColor;
            }
            
            buffer.drawStringClipped(x + 1, itemY, displayText, finalTextColor, x + width - 2);
            
            // Draw shortcut key if present (right-aligned)
            if (!item.shortcut.empty()) {
                int shortcutX = x + width - item.shortcut.length() - 3;  // Right-align with padding
                buffer.drawStringClipped(shortcutX, itemY, item.shortcut, finalTextColor, x + width - 2);
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

// Static utility for drawing menu bar background
void DropdownMenu::drawMenuBar(UnicodeBuffer& buffer, int y, int termWidth) {
    std::string barColor = Color::BRIGHT_WHITE + Color::BG_BLACK;
    
    // Draw horizontal menu bar background across entire screen
    for (int x = 0; x < termWidth; x++) {
        buffer.setCell(x, y, " ", barColor);
    }
}

// Collision detection and position adjustment for menus
void DropdownMenu::adjustMenuPositions(std::vector<std::shared_ptr<DropdownMenu>>& menus, int termWidth) {
    if (menus.empty()) return;
    
    // Sort menus by trigger position for left-to-right processing
    std::sort(menus.begin(), menus.end(), [](const std::shared_ptr<DropdownMenu>& a, const std::shared_ptr<DropdownMenu>& b) {
        return a->getX() < b->getX();
    });
    
    // Adjust each menu to avoid overlaps
    for (size_t i = 0; i < menus.size(); i++) {
        auto& currentMenu = menus[i];
        if (!currentMenu->isOpen()) continue;
        
        // Check collision with previous menu
        if (i > 0) {
            auto& prevMenu = menus[i - 1];
            if (prevMenu->isOpen()) {
                int prevMenuRight = prevMenu->getMenuX() + prevMenu->getMenuWidth();
                int currentMenuLeft = currentMenu->getMenuX();
                
                // If there's overlap, move current menu to the right
                if (currentMenuLeft < prevMenuRight) {
                    int newX = prevMenuRight + 1; // 1 character gap
                    
                    // Make sure it doesn't go off screen
                    if (newX + currentMenu->getMenuWidth() > termWidth) {
                        // If it would go off screen, try moving it to the left of the trigger
                        newX = currentMenu->getX() - currentMenu->getMenuWidth();
                        if (newX < 0) {
                            // If still doesn't fit, compress the previous menu
                            newX = std::max(0, termWidth - currentMenu->getMenuWidth());
                        }
                    }
                    
                    currentMenu->setMenuX(newX);
                }
            }
        }
        
        // Check collision with next menu (for right-aligned menus)
        if (i < menus.size() - 1) {
            auto& nextMenu = menus[i + 1];
            if (nextMenu->isOpen()) {
                int currentMenuRight = currentMenu->getMenuX() + currentMenu->getMenuWidth();
                int nextMenuLeft = nextMenu->getMenuX();
                
                // If there's overlap, try to adjust
                if (currentMenuRight > nextMenuLeft) {
                    int availableSpace = nextMenuLeft - currentMenu->getMenuX();
                    if (availableSpace > 10) { // Minimum usable width
                        // Could implement menu width compression here if needed
                    }
                }
            }
        }
        
        // Final screen bounds check
        int menuRight = currentMenu->getMenuX() + currentMenu->getMenuWidth();
        if (menuRight > termWidth) {
            int adjustedX = termWidth - currentMenu->getMenuWidth();
            currentMenu->setMenuX(std::max(0, adjustedX));
        }
    }
}