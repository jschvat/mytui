#include "../include/radio_buttons.h"
#include "../include/window.h"
#include "../include/buffer.h"
#include <algorithm>

// RadioButtonEvent implementation
RadioButtonEvent::RadioButtonEvent(EventType type, std::shared_ptr<RadioButtons> rb, int oldIndex, int newIndex,
                                   const std::string& oldVal, const std::string& newVal, const std::string& text)
    : Event(type), radioButtons(rb), oldSelectedIndex(oldIndex), newSelectedIndex(newIndex),
      oldValue(oldVal), newValue(newVal), selectedText(text) {
}

// RadioButtons implementation
RadioButtons::RadioButtons(std::shared_ptr<Window> parent, int x, int y, bool horizontal)
    : parentWindow(parent), x(x), y(y), width(0), height(0), selectedIndex(-1),
      visible(true), active(false), enabled(true),
      selectedChar("●"), unselectedChar("○"),
      buttonColor(Color::WHITE + Color::BG_BLACK), labelColor(Color::BRIGHT_WHITE + Color::BG_BLACK),
      selectedColor(Color::BRIGHT_GREEN + Color::BG_BLACK), activeColor(Color::BLACK + Color::BG_BRIGHT_WHITE),
      disabledColor(Color::CYAN + Color::BG_BLACK),
      horizontal(horizontal), spacing(2), wasLeftPressed(false), hoveredIndex(-1) {
    calculateDimensions();
}

void RadioButtons::addItem(const std::string& text, const std::string& value, bool enabled) {
    items.emplace_back(text, value, enabled);
    calculateDimensions();
}

void RadioButtons::addItem(const RadioButtonItem& item) {
    items.push_back(item);
    calculateDimensions();
}

void RadioButtons::removeItem(int index) {
    if (index >= 0 && index < (int)items.size()) {
        items.erase(items.begin() + index);
        
        // Adjust selection
        if (selectedIndex == index) {
            selectedIndex = -1;
        } else if (selectedIndex > index) {
            selectedIndex--;
        }
        
        calculateDimensions();
    }
}

void RadioButtons::clearItems() {
    items.clear();
    selectedIndex = -1;
    calculateDimensions();
}

void RadioButtons::setItemEnabled(int index, bool enabled) {
    if (index >= 0 && index < (int)items.size()) {
        items[index].enabled = enabled;
        
        // If disabled item was selected, clear selection
        if (!enabled && selectedIndex == index) {
            selectedIndex = -1;
        }
    }
}

void RadioButtons::setItemText(int index, const std::string& text) {
    if (index >= 0 && index < (int)items.size()) {
        items[index].text = text;
        calculateDimensions();
    }
}

void RadioButtons::calculateDimensions() {
    if (items.empty()) {
        width = height = 0;
        return;
    }
    
    if (horizontal) {
        // Horizontal layout: sum widths, height = 1
        width = 0;
        for (size_t i = 0; i < items.size(); i++) {
            width += 4 + UnicodeUtils::getDisplayWidth(items[i].text); // (●) text
            if (i < items.size() - 1) {
                width += spacing;
            }
        }
        height = 1;
    } else {
        // Vertical layout: max width, sum heights
        width = 0;
        for (const auto& item : items) {
            int itemWidth = 4 + UnicodeUtils::getDisplayWidth(item.text); // (●) text
            width = std::max(width, itemWidth);
        }
        height = (int)items.size();
    }
}

void RadioButtons::setSelectedIndex(int index) {
    if (index < -1 || index >= (int)items.size()) return;
    if (index >= 0 && !items[index].enabled) return;
    
    int oldIndex = selectedIndex;
    std::string oldValue = getSelectedValue();
    
    selectedIndex = index;
    
    std::string newValue = getSelectedValue();
    std::string selectedText = (index >= 0) ? items[index].text : "";
    
    if (oldIndex != selectedIndex) {
        generateRadioEvent(EventType::BUTTON_TOGGLE, oldIndex, selectedIndex);
        generateRadioEvent(EventType::BUTTON_CLICK, oldIndex, selectedIndex);
    }
}

void RadioButtons::setSelectedValue(const std::string& value) {
    for (int i = 0; i < (int)items.size(); i++) {
        if (items[i].value == value && items[i].enabled) {
            setSelectedIndex(i);
            return;
        }
    }
    setSelectedIndex(-1); // Clear selection if value not found
}

std::string RadioButtons::getSelectedValue() const {
    if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        return items[selectedIndex].value;
    }
    return "";
}

std::string RadioButtons::getSelectedText() const {
    if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        return items[selectedIndex].text;
    }
    return "";
}

const RadioButtonItem& RadioButtons::getItem(int index) const {
    static RadioButtonItem empty("", "");
    if (index >= 0 && index < (int)items.size()) {
        return items[index];
    }
    return empty;
}

std::vector<std::string> RadioButtons::getItemTexts() const {
    std::vector<std::string> texts;
    for (const auto& item : items) {
        texts.push_back(item.text);
    }
    return texts;
}

std::vector<std::string> RadioButtons::getItemValues() const {
    std::vector<std::string> values;
    for (const auto& item : items) {
        values.push_back(item.value);
    }
    return values;
}

void RadioButtons::setHorizontal(bool isHorizontal) {
    horizontal = isHorizontal;
    calculateDimensions();
}

void RadioButtons::setEnabled(bool newEnabled) {
    enabled = newEnabled;
    if (!enabled) {
        active = false;
        hoveredIndex = -1;
    }
}

void RadioButtons::setColors(const std::string& button, const std::string& label, const std::string& selected,
                            const std::string& active, const std::string& disabled) {
    buttonColor = button;
    labelColor = label;
    if (!selected.empty()) selectedColor = selected;
    if (!active.empty()) activeColor = active;
    if (!disabled.empty()) disabledColor = disabled;
}

int RadioButtons::getItemAtPosition(int mx, int my) const {
    if (!parentWindow) return -1;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    if (horizontal) {
        if (my != absY) return -1;
        
        int currentX = absX;
        for (int i = 0; i < (int)items.size(); i++) {
            int itemWidth = 4 + UnicodeUtils::getDisplayWidth(items[i].text);
            
            if (mx >= currentX && mx < currentX + itemWidth) {
                return i;
            }
            
            currentX += itemWidth + spacing;
        }
    } else {
        if (mx < absX || mx >= absX + width) return -1;
        
        int itemY = my - absY;
        if (itemY >= 0 && itemY < (int)items.size()) {
            return itemY;
        }
    }
    
    return -1;
}

bool RadioButtons::contains(int mx, int my) const {
    if (!parentWindow) return false;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    return mx >= absX && mx < absX + width && my >= absY && my < absY + height;
}

void RadioButtons::updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight) {
    if (!visible || !enabled || !parentWindow || !parentWindow->isVisible()) return;
    
    int mouseX = mouse.getMouseX();
    int mouseY = mouse.getMouseY();
    bool leftPressed = mouse.isLeftButtonPressed();
    
    bool isHovering = contains(mouseX, mouseY);
    bool wasHovering = active;
    active = isHovering;
    
    int currentHoverIndex = isHovering ? getItemAtPosition(mouseX, mouseY) : -1;
    int prevHoverIndex = hoveredIndex;
    hoveredIndex = currentHoverIndex;
    
    // Generate hover/leave events for individual items
    if (currentHoverIndex != prevHoverIndex) {
        if (prevHoverIndex >= 0 && onItemLeave) {
            auto event = MouseEvent(EventType::MOUSE_LEAVE, mouseX, mouseY);
            onItemLeave(event);
        }
        if (currentHoverIndex >= 0 && onItemHover) {
            auto event = MouseEvent(EventType::MOUSE_ENTER, mouseX, mouseY);
            onItemHover(event);
        }
    }
    
    if (isHovering) {
        // Handle click events
        if (leftPressed && !wasLeftPressed) {
            int clickedIndex = getItemAtPosition(mouseX, mouseY);
            
            if (clickedIndex >= 0 && items[clickedIndex].enabled) {
                setSelectedIndex(clickedIndex);
                
                if (onClick) {
                    auto event = MouseEvent(EventType::MOUSE_PRESS, mouseX, mouseY);
                    onClick(event);
                }
            }
        }
    }
    
    wasLeftPressed = leftPressed;
}

void RadioButtons::draw(UnicodeBuffer& buffer) {
    if (!visible || !parentWindow || !parentWindow->isVisible()) return;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    if (horizontal) {
        // Horizontal layout
        int currentX = absX;
        
        for (int i = 0; i < (int)items.size(); i++) {
            const auto& item = items[i];
            
            // Determine colors
            std::string currentButtonColor = buttonColor;
            std::string currentLabelColor = labelColor;
            
            if (!enabled || !item.enabled) {
                currentButtonColor = disabledColor;
                currentLabelColor = disabledColor;
            } else if (i == hoveredIndex) {
                currentButtonColor = activeColor;
                currentLabelColor = activeColor;
            } else if (i == selectedIndex) {
                currentButtonColor = selectedColor;
                currentLabelColor = selectedColor;
            }
            
            // Draw radio button
            buffer.setCell(currentX, absY, "(", currentButtonColor);
            buffer.setCell(currentX + 1, absY, (i == selectedIndex) ? selectedChar : unselectedChar, currentButtonColor);
            buffer.setCell(currentX + 2, absY, ")", currentButtonColor);
            buffer.setCell(currentX + 3, absY, " ", currentLabelColor);
            
            // Draw label
            buffer.drawStringClipped(currentX + 4, absY, item.text, currentLabelColor, currentX + 4 + UnicodeUtils::getDisplayWidth(item.text));
            
            currentX += 4 + UnicodeUtils::getDisplayWidth(item.text) + spacing;
        }
    } else {
        // Vertical layout
        for (int i = 0; i < (int)items.size(); i++) {
            const auto& item = items[i];
            
            // Determine colors
            std::string currentButtonColor = buttonColor;
            std::string currentLabelColor = labelColor;
            
            if (!enabled || !item.enabled) {
                currentButtonColor = disabledColor;
                currentLabelColor = disabledColor;
            } else if (i == hoveredIndex) {
                currentButtonColor = activeColor;
                currentLabelColor = activeColor;
            } else if (i == selectedIndex) {
                currentButtonColor = selectedColor;
                currentLabelColor = selectedColor;
            }
            
            // Draw radio button
            buffer.setCell(absX, absY + i, "(", currentButtonColor);
            buffer.setCell(absX + 1, absY + i, (i == selectedIndex) ? selectedChar : unselectedChar, currentButtonColor);
            buffer.setCell(absX + 2, absY + i, ")", currentButtonColor);
            buffer.setCell(absX + 3, absY + i, " ", currentLabelColor);
            
            // Draw label
            buffer.drawStringClipped(absX + 4, absY + i, item.text, currentLabelColor, absX + width);
        }
    }
}

void RadioButtons::generateRadioEvent(EventType type, int oldIndex, int newIndex) {
    auto self = std::shared_ptr<RadioButtons>(this, [](RadioButtons*) {}); // Non-owning shared_ptr
    
    std::string oldValue = (oldIndex >= 0 && oldIndex < (int)items.size()) ? items[oldIndex].value : "";
    std::string newValue = (newIndex >= 0 && newIndex < (int)items.size()) ? items[newIndex].value : "";
    std::string selectedText = (newIndex >= 0 && newIndex < (int)items.size()) ? items[newIndex].text : "";
    
    auto event = std::unique_ptr<RadioButtonEvent>(new RadioButtonEvent(type, self, oldIndex, newIndex, oldValue, newValue, selectedText));
    
    // Call local callbacks first
    switch (type) {
        case EventType::BUTTON_TOGGLE:
            if (onSelectionChange) onSelectionChange(*event);
            break;
        case EventType::BUTTON_CLICK:
            if (onItemSelect) onItemSelect(*event);
            break;
        default:
            break;
    }
    
    // Dispatch to global event manager
    EventManager::getInstance().dispatchEvent(std::move(event));
}