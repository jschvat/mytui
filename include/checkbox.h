#pragma once

#include "buffer.h"
#include "mouse_handler.h"
#include "colors.h"
#include "event_system.h"
#include <string>
#include <functional>
#include <memory>

// Forward declaration
class Window;

// Checkbox events
class CheckboxEvent : public Event {
public:
    std::weak_ptr<class Checkbox> checkbox;
    bool oldState;
    bool newState;
    std::string label;
    
    CheckboxEvent(EventType type, std::shared_ptr<class Checkbox> cb, bool oldState, bool newState, const std::string& label);
};

// Checkbox component
class Checkbox {
private:
    std::shared_ptr<Window> parentWindow;
    int x, y;                    // Position relative to parent
    int width, height;           // Dimensions (calculated from label)
    std::string label;           // Checkbox label text
    bool checked;                // Current state
    bool visible;
    bool active;
    bool enabled;
    
    // Visual properties
    std::string checkedChar;
    std::string uncheckedChar;
    std::string boxColor;
    std::string labelColor;
    std::string activeColor;
    std::string disabledColor;
    
    // Mouse interaction
    bool wasLeftPressed;
    
    void generateCheckboxEvent(EventType type, bool oldState, bool newState);
    void calculateDimensions();
    
public:
    Checkbox(std::shared_ptr<Window> parent, int x, int y, const std::string& label, bool initialState = false);
    ~Checkbox() = default;
    
    // State management
    void setChecked(bool state);
    void toggle();
    bool isChecked() const { return checked; }
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }
    
    // Label management
    void setLabel(const std::string& newLabel);
    std::string getLabel() const { return label; }
    
    // Visual configuration
    void setCheckedChar(const std::string& ch) { checkedChar = ch; }
    void setUncheckedChar(const std::string& ch) { uncheckedChar = ch; }
    void setColors(const std::string& box, const std::string& labelCol, const std::string& active = "", const std::string& disabled = "");
    
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
    std::function<void(const CheckboxEvent&)> onStateChange;
    std::function<void(const CheckboxEvent&)> onCheck;
    std::function<void(const CheckboxEvent&)> onUncheck;
    std::function<void(const MouseEvent&)> onHover;
    std::function<void(const MouseEvent&)> onLeave;
    std::function<void(const MouseEvent&)> onClick;
    
    // Event callback setters
    void setOnStateChange(std::function<void(const CheckboxEvent&)> callback) { onStateChange = callback; }
    void setOnCheck(std::function<void(const CheckboxEvent&)> callback) { onCheck = callback; }
    void setOnUncheck(std::function<void(const CheckboxEvent&)> callback) { onUncheck = callback; }
    void setOnHover(std::function<void(const MouseEvent&)> callback) { onHover = callback; }
    void setOnLeave(std::function<void(const MouseEvent&)> callback) { onLeave = callback; }
    void setOnClick(std::function<void(const MouseEvent&)> callback) { onClick = callback; }
};