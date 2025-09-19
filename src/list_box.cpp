#include "../include/list_box.h"
#include "../include/window.h"
#include <algorithm>

// ListBoxEvent implementation
ListBoxEvent::ListBoxEvent(EventType type, std::shared_ptr<ListBox> lb, int index, const std::string& text, const std::string& value)
    : Event(type), listBox(lb), itemIndex(index), itemText(text), itemValue(value) {
    if (lb) {
        oldSelectedIndex = newSelectedIndex = lb->getSelectedIndex();
    } else {
        oldSelectedIndex = newSelectedIndex = -1;
    }
}

// ListBox implementation
ListBox::ListBox(std::shared_ptr<Window> parent, int x, int y, int width, int height)
    : parentWindow(parent), x(x), y(y), width(width), height(height),
      selectedIndex(-1), scrollOffset(0), visible(true), active(false), enabled(true),
      borderColor(Color::WHITE + Color::BG_BLACK), backgroundColor(Color::BRIGHT_WHITE + Color::BG_BLACK),
      textColor(Color::BLACK + Color::BG_WHITE), selectedColor(Color::WHITE + Color::BG_BLUE),
      activeColor(Color::BLACK + Color::BG_BRIGHT_WHITE), disabledColor(Color::CYAN + Color::BG_BLACK),
      separatorColor(Color::CYAN + Color::BG_BLACK),
      showScrollbar(true), scrollbarColor(Color::WHITE + Color::BG_CYAN), scrollThumbColor(Color::BLACK + Color::BG_BRIGHT_WHITE),
      multiSelect(false), wasLeftPressed(false), hoveredIndex(-1), dragging(false) {
    calculateDimensions();
}

void ListBox::addItem(const std::string& text, const std::string& value, const std::string& color, bool enabled) {
    items.emplace_back(text, value, color, enabled, false);
    if (multiSelect) {
        selectedItems.push_back(false);
    }
}

void ListBox::addSeparator() {
    items.emplace_back("", "", separatorColor, false, true);
    if (multiSelect) {
        selectedItems.push_back(false);
    }
}

void ListBox::clearItems() {
    items.clear();
    selectedItems.clear();
    selectedIndex = -1;
    scrollOffset = 0;
}

void ListBox::setSelectedIndex(int index) {
    if (index < -1 || index >= (int)items.size()) return;
    if (index >= 0 && (!items[index].enabled || items[index].separator)) return;
    
    int oldIndex = selectedIndex;
    selectedIndex = index;
    
    if (selectedIndex >= 0) {
        ensureItemVisible(selectedIndex);
    }
    
    if (oldIndex != selectedIndex) {
        generateListEvent(EventType::BUTTON_TOGGLE, selectedIndex);
    }
}

std::string ListBox::getSelectedValue() const {
    if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        return items[selectedIndex].value;
    }
    return "";
}

void ListBox::setMultiSelect(bool enabled) {
    multiSelect = enabled;
    if (enabled) {
        selectedItems.resize(items.size(), false);
    } else {
        selectedItems.clear();
    }
}

void ListBox::setItemSelected(int index, bool selected) {
    if (!multiSelect || index < 0 || index >= (int)items.size()) return;
    
    if (index < (int)selectedItems.size()) {
        selectedItems[index] = selected && items[index].enabled && !items[index].separator;
    }
}

std::vector<int> ListBox::getSelectedIndices() const {
    std::vector<int> indices;
    if (multiSelect) {
        for (int i = 0; i < (int)selectedItems.size(); i++) {
            if (selectedItems[i]) {
                indices.push_back(i);
            }
        }
    } else if (selectedIndex >= 0) {
        indices.push_back(selectedIndex);
    }
    return indices;
}

void ListBox::scrollUp() {
    if (scrollOffset > 0) {
        scrollOffset--;
    }
}

void ListBox::scrollDown() {
    int maxScroll = std::max(0, (int)items.size() - getVisibleItemCount());
    if (scrollOffset < maxScroll) {
        scrollOffset++;
    }
}

void ListBox::ensureItemVisible(int index) {
    if (index < 0 || index >= (int)items.size()) return;
    
    int visibleCount = getVisibleItemCount();
    
    if (index < scrollOffset) {
        scrollOffset = index;
    } else if (index >= scrollOffset + visibleCount) {
        scrollOffset = index - visibleCount + 1;
    }
    
    scrollOffset = std::max(0, std::min(scrollOffset, (int)items.size() - visibleCount));
}

int ListBox::getVisibleItemCount() const {
    return height - 2; // Account for borders
}

int ListBox::getItemAtPosition(int mx, int my) const {
    if (!parentWindow) return -1;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    if (mx < absX + 1 || mx >= absX + width - 1 || my < absY + 1 || my >= absY + height - 1) {
        return -1;
    }
    
    int itemY = my - absY - 1;
    int itemIndex = scrollOffset + itemY;
    
    if (itemIndex >= 0 && itemIndex < (int)items.size()) {
        return itemIndex;
    }
    
    return -1;
}

bool ListBox::contains(int mx, int my) const {
    if (!parentWindow) return false;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    return mx >= absX && mx < absX + width && my >= absY && my < absY + height;
}

void ListBox::updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight) {
    if (!visible || !enabled || !parentWindow || !parentWindow->isVisible()) return;
    
    int mouseX = mouse.getMouseX();
    int mouseY = mouse.getMouseY();
    bool leftPressed = mouse.isLeftButtonPressed();
    
    bool isHovering = contains(mouseX, mouseY);
    bool wasHovering = active;
    active = isHovering;
    
    if (isHovering && !wasHovering && onHover) {
        auto event = MouseEvent(EventType::MOUSE_ENTER, mouseX, mouseY);
        onHover(event);
    } else if (!isHovering && wasHovering && onLeave) {
        auto event = MouseEvent(EventType::MOUSE_LEAVE, mouseX, mouseY);
        onLeave(event);
    }
    
    int currentHoverIndex = isHovering ? getItemAtPosition(mouseX, mouseY) : -1;
    if (currentHoverIndex != hoveredIndex) {
        if (hoveredIndex >= 0 && onItemLeave) {
            generateListEvent(EventType::MOUSE_LEAVE, hoveredIndex);
        }
        hoveredIndex = currentHoverIndex;
        if (hoveredIndex >= 0 && onItemHover) {
            generateListEvent(EventType::MOUSE_ENTER, hoveredIndex);
        }
    }
    
    if (isHovering && leftPressed && !wasLeftPressed) {
        int clickedIndex = getItemAtPosition(mouseX, mouseY);
        if (clickedIndex >= 0 && items[clickedIndex].enabled && !items[clickedIndex].separator) {
            if (multiSelect) {
                setItemSelected(clickedIndex, !isItemSelected(clickedIndex));
            } else {
                setSelectedIndex(clickedIndex);
            }
            generateListEvent(EventType::BUTTON_CLICK, clickedIndex);
        }
    }
    
    wasLeftPressed = leftPressed;
}

void ListBox::draw(UnicodeBuffer& buffer) {
    if (!visible || !parentWindow || !parentWindow->isVisible()) return;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    // Draw border
    buffer.drawBox(absX, absY, width, height, borderColor, true, false);
    
    // Fill background
    for (int row = 1; row < height - 1; row++) {
        for (int col = 1; col < width - 1; col++) {
            buffer.setCell(absX + col, absY + row, " ", backgroundColor);
        }
    }
    
    // Draw items
    int visibleCount = getVisibleItemCount();
    for (int i = 0; i < visibleCount && scrollOffset + i < (int)items.size(); i++) {
        int itemIndex = scrollOffset + i;
        const auto& item = items[itemIndex];
        
        int itemY = absY + 1 + i;
        
        if (item.separator) {
            // Draw separator
            for (int col = 1; col < width - 1; col++) {
                buffer.setCell(absX + col, itemY, "─", separatorColor);
            }
        } else {
            // Determine item color
            std::string itemColor = item.color.empty() ? textColor : item.color;
            
            if (!item.enabled) {
                itemColor = disabledColor;
            } else if (itemIndex == hoveredIndex) {
                itemColor = activeColor;
            } else if (itemIndex == selectedIndex || (multiSelect && isItemSelected(itemIndex))) {
                itemColor = selectedColor;
            }
            
            // Draw selection indicator for multi-select
            if (multiSelect) {
                std::string indicator = isItemSelected(itemIndex) ? "✓" : " ";
                buffer.setCell(absX + 1, itemY, indicator, itemColor);
                buffer.drawStringClipped(absX + 3, itemY, item.text, itemColor, absX + width - 1);
            } else {
                buffer.drawStringClipped(absX + 2, itemY, item.text, itemColor, absX + width - 1);
            }
        }
    }
    
    // Draw scrollbar if needed
    if (showScrollbar && (int)items.size() > visibleCount) {
        int scrollbarX = absX + width - 2;
        int scrollbarHeight = height - 2;
        
        // Draw scrollbar track
        for (int i = 0; i < scrollbarHeight; i++) {
            buffer.setCell(scrollbarX, absY + 1 + i, "│", scrollbarColor);
        }
        
        // Draw scroll thumb
        if (scrollbarHeight > 0 && !items.empty()) {
            int thumbSize = std::max(1, (visibleCount * scrollbarHeight) / (int)items.size());
            int thumbPos = (scrollOffset * (scrollbarHeight - thumbSize)) / std::max(1, (int)items.size() - visibleCount);
            
            for (int i = 0; i < thumbSize; i++) {
                buffer.setCell(scrollbarX, absY + 1 + thumbPos + i, "█", scrollThumbColor);
            }
        }
    }
}

const ListBoxItem& ListBox::getItem(int index) const {
    static ListBoxItem empty("", "");
    if (index >= 0 && index < (int)items.size()) {
        return items[index];
    }
    return empty;
}

bool ListBox::isItemSelected(int index) const {
    if (multiSelect && index >= 0 && index < (int)selectedItems.size()) {
        return selectedItems[index];
    }
    return index == selectedIndex;
}

void ListBox::calculateDimensions() {
    if (width < 5) width = 5;
    if (height < 3) height = 3;
}

void ListBox::generateListEvent(EventType type, int itemIndex) {
    auto self = std::shared_ptr<ListBox>(this, [](ListBox*) {});
    
    std::string itemText = (itemIndex >= 0 && itemIndex < (int)items.size()) ? items[itemIndex].text : "";
    std::string itemValue = (itemIndex >= 0 && itemIndex < (int)items.size()) ? items[itemIndex].value : "";
    
    auto event = std::unique_ptr<ListBoxEvent>(new ListBoxEvent(type, self, itemIndex, itemText, itemValue));
    
    // Call local callbacks
    switch (type) {
        case EventType::BUTTON_TOGGLE:
            if (onSelectionChange) onSelectionChange(*event);
            break;
        case EventType::BUTTON_CLICK:
            if (onItemSelect) onItemSelect(*event);
            break;
        case EventType::MOUSE_ENTER:
            if (onItemHover) onItemHover(*event);
            break;
        case EventType::MOUSE_LEAVE:
            if (onItemLeave) onItemLeave(*event);
            break;
        default:
            break;
    }
    
    EventManager::getInstance().dispatchEvent(std::move(event));
}