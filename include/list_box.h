#pragma once

#include "buffer.h"
#include "mouse_handler.h"
#include "colors.h"
#include "event_system.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Forward declaration
class Window;

// List box item
struct ListBoxItem {
    std::string text;
    std::string value;
    std::string color;
    bool enabled;
    bool separator;
    
    ListBoxItem(const std::string& text, const std::string& value = "", const std::string& color = "", bool enabled = true, bool separator = false)
        : text(text), value(value.empty() ? text : value), color(color), enabled(enabled), separator(separator) {}
};

// List box events
class ListBoxEvent : public Event {
public:
    std::weak_ptr<class ListBox> listBox;
    int itemIndex;
    std::string itemText;
    std::string itemValue;
    int oldSelectedIndex;
    int newSelectedIndex;
    
    ListBoxEvent(EventType type, std::shared_ptr<class ListBox> lb, int index, const std::string& text, const std::string& value);
};

// List box component (scrollable list of selectable items)
class ListBox {
private:
    std::shared_ptr<Window> parentWindow;
    int x, y;                    // Position relative to parent
    int width, height;           // Dimensions
    std::vector<ListBoxItem> items;
    int selectedIndex;           // Currently selected item (-1 = none)
    int scrollOffset;            // Top visible item index
    bool visible;
    bool active;
    bool enabled;
    
    // Visual properties
    std::string borderColor;
    std::string backgroundColor;
    std::string textColor;
    std::string selectedColor;
    std::string activeColor;
    std::string disabledColor;
    std::string separatorColor;
    
    // Scroll properties
    bool showScrollbar;
    std::string scrollbarColor;
    std::string scrollThumbColor;
    
    // Selection mode
    bool multiSelect;
    std::vector<bool> selectedItems;
    
    // Mouse interaction
    bool wasLeftPressed;
    int hoveredIndex;
    bool dragging;
    
    void generateListEvent(EventType type, int itemIndex);
    void calculateDimensions();
    int getItemAtPosition(int mx, int my) const;
    void ensureItemVisible(int index);
    void updateScrollbar();
    
public:
    ListBox(std::shared_ptr<Window> parent, int x, int y, int width, int height);
    ~ListBox() = default;
    
    // Item management
    void addItem(const std::string& text, const std::string& value = "", const std::string& color = "", bool enabled = true);
    void addItem(const ListBoxItem& item);
    void addSeparator();
    void insertItem(int index, const std::string& text, const std::string& value = "", const std::string& color = "", bool enabled = true);
    void removeItem(int index);
    void clearItems();
    void setItemText(int index, const std::string& text);
    void setItemValue(int index, const std::string& value);
    void setItemColor(int index, const std::string& color);
    void setItemEnabled(int index, bool enabled);
    
    // Selection management
    void setSelectedIndex(int index);
    void setSelectedValue(const std::string& value);
    int getSelectedIndex() const { return selectedIndex; }
    std::string getSelectedValue() const;
    std::string getSelectedText() const;
    bool hasSelection() const { return selectedIndex >= 0; }
    
    // Multi-selection (if enabled)
    void setMultiSelect(bool enabled);
    bool isMultiSelect() const { return multiSelect; }
    void setItemSelected(int index, bool selected);
    bool isItemSelected(int index) const;
    std::vector<int> getSelectedIndices() const;
    std::vector<std::string> getSelectedValues() const;
    std::vector<std::string> getSelectedTexts() const;
    void clearSelection();
    void selectAll();
    
    // Scrolling
    void scrollUp();
    void scrollDown();
    void scrollPageUp();
    void scrollPageDown();
    void scrollToTop();
    void scrollToBottom();
    void scrollToItem(int index);
    int getScrollOffset() const { return scrollOffset; }
    int getVisibleItemCount() const;
    
    // Item access
    int getItemCount() const { return (int)items.size(); }
    const ListBoxItem& getItem(int index) const;
    std::vector<std::string> getItemTexts() const;
    std::vector<std::string> getItemValues() const;
    int findItemByValue(const std::string& value) const;
    int findItemByText(const std::string& text) const;
    
    // State management
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }
    
    // Visual configuration
    void setColors(const std::string& border, const std::string& background, const std::string& text,
                   const std::string& selected = "", const std::string& active = "", const std::string& disabled = "");
    void setScrollbarColors(const std::string& scrollbar, const std::string& thumb);
    void setShowScrollbar(bool show) { showScrollbar = show; }
    
    // Interaction
    void updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight);
    void handleKeyboard(char ch, int keyCode);
    bool contains(int mx, int my) const;
    
    // Rendering
    void draw(UnicodeBuffer& buffer);
    void show() { visible = true; }
    void hide() { visible = false; }
    bool isVisible() const { return visible; }
    
    // Position management
    void setPosition(int newX, int newY) { x = newX; y = newY; }
    void setSize(int newWidth, int newHeight) { width = newWidth; height = newHeight; calculateDimensions(); }
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Event callbacks
    std::function<void(const ListBoxEvent&)> onSelectionChange;
    std::function<void(const ListBoxEvent&)> onItemSelect;
    std::function<void(const ListBoxEvent&)> onItemDoubleClick;
    std::function<void(const ListBoxEvent&)> onItemHover;
    std::function<void(const ListBoxEvent&)> onItemLeave;
    std::function<void(const MouseEvent&)> onScroll;
    std::function<void(const MouseEvent&)> onHover;
    std::function<void(const MouseEvent&)> onLeave;
    
    // Event callback setters
    void setOnSelectionChange(std::function<void(const ListBoxEvent&)> callback) { onSelectionChange = callback; }
    void setOnItemSelect(std::function<void(const ListBoxEvent&)> callback) { onItemSelect = callback; }
    void setOnItemDoubleClick(std::function<void(const ListBoxEvent&)> callback) { onItemDoubleClick = callback; }
    void setOnItemHover(std::function<void(const ListBoxEvent&)> callback) { onItemHover = callback; }
    void setOnItemLeave(std::function<void(const ListBoxEvent&)> callback) { onItemLeave = callback; }
    void setOnScroll(std::function<void(const MouseEvent&)> callback) { onScroll = callback; }
    void setOnHover(std::function<void(const MouseEvent&)> callback) { onHover = callback; }
    void setOnLeave(std::function<void(const MouseEvent&)> callback) { onLeave = callback; }
};