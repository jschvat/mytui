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

// Radio button item
struct RadioButtonItem {
    std::string text;
    std::string value;
    bool enabled;
    
    RadioButtonItem(const std::string& text, const std::string& value = "", bool enabled = true)
        : text(text), value(value.empty() ? text : value), enabled(enabled) {}
};

// Radio button events
class RadioButtonEvent : public Event {
public:
    std::weak_ptr<class RadioButtons> radioButtons;
    int oldSelectedIndex;
    int newSelectedIndex;
    std::string oldValue;
    std::string newValue;
    std::string selectedText;
    
    RadioButtonEvent(EventType type, std::shared_ptr<class RadioButtons> rb, int oldIndex, int newIndex, 
                     const std::string& oldVal, const std::string& newVal, const std::string& text);
};

// Radio buttons component (group of mutually exclusive options)
class RadioButtons {
private:
    std::shared_ptr<Window> parentWindow;
    int x, y;                    // Position relative to parent
    int width, height;           // Dimensions (calculated)
    std::vector<RadioButtonItem> items;
    int selectedIndex;           // Currently selected item (-1 = none)
    bool visible;
    bool active;
    bool enabled;
    
    // Visual properties
    std::string selectedChar;
    std::string unselectedChar;
    std::string buttonColor;
    std::string labelColor;
    std::string selectedColor;
    std::string activeColor;
    std::string disabledColor;
    
    // Layout
    bool horizontal;             // Layout direction
    int spacing;                 // Space between items
    
    // Mouse interaction
    bool wasLeftPressed;
    int hoveredIndex;
    
    void generateRadioEvent(EventType type, int oldIndex, int newIndex);
    void calculateDimensions();
    int getItemAtPosition(int mx, int my) const;
    
public:
    RadioButtons(std::shared_ptr<Window> parent, int x, int y, bool horizontal = false);
    ~RadioButtons() = default;
    
    // Item management
    void addItem(const std::string& text, const std::string& value = "", bool enabled = true);
    void addItem(const RadioButtonItem& item);
    void removeItem(int index);
    void clearItems();
    void setItemEnabled(int index, bool enabled);
    void setItemText(int index, const std::string& text);
    
    // Selection management
    void setSelectedIndex(int index);
    void setSelectedValue(const std::string& value);
    int getSelectedIndex() const { return selectedIndex; }
    std::string getSelectedValue() const;
    std::string getSelectedText() const;
    bool hasSelection() const { return selectedIndex >= 0; }
    
    // Item access
    int getItemCount() const { return (int)items.size(); }
    const RadioButtonItem& getItem(int index) const;
    std::vector<std::string> getItemTexts() const;
    std::vector<std::string> getItemValues() const;
    
    // Layout configuration
    void setHorizontal(bool horizontal);
    bool isHorizontal() const { return horizontal; }
    void setSpacing(int spacing) { this->spacing = spacing; calculateDimensions(); }
    int getSpacing() const { return spacing; }
    
    // State management
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }
    
    // Visual configuration
    void setSelectedChar(const std::string& ch) { selectedChar = ch; }
    void setUnselectedChar(const std::string& ch) { unselectedChar = ch; }
    void setColors(const std::string& button, const std::string& label, const std::string& selected = "", 
                   const std::string& active = "", const std::string& disabled = "");
    
    // Interaction
    void updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight);
    bool contains(int mx, int my) const;
    
    // Rendering
    void draw(UnicodeBuffer& buffer);
    void show() { visible = true; }
    void hide() { visible = false; }
    bool isVisible() const { return visible; }
    
    // Position management
    void setPosition(int newX, int newY) { x = newX; y = newY; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Event callbacks
    std::function<void(const RadioButtonEvent&)> onSelectionChange;
    std::function<void(const RadioButtonEvent&)> onItemSelect;
    std::function<void(const MouseEvent&)> onItemHover;
    std::function<void(const MouseEvent&)> onItemLeave;
    std::function<void(const MouseEvent&)> onClick;
    
    // Event callback setters
    void setOnSelectionChange(std::function<void(const RadioButtonEvent&)> callback) { onSelectionChange = callback; }
    void setOnItemSelect(std::function<void(const RadioButtonEvent&)> callback) { onItemSelect = callback; }
    void setOnItemHover(std::function<void(const MouseEvent&)> callback) { onItemHover = callback; }
    void setOnItemLeave(std::function<void(const MouseEvent&)> callback) { onItemLeave = callback; }
    void setOnClick(std::function<void(const MouseEvent&)> callback) { onClick = callback; }
};