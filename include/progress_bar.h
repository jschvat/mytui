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

// Progress bar events
class ProgressBarEvent : public Event {
public:
    std::weak_ptr<class ProgressBar> progressBar;
    double oldValue;
    double newValue;
    double percentage;
    
    ProgressBarEvent(EventType type, std::shared_ptr<class ProgressBar> pb, double oldVal, double newVal);
};

// Progress bar component
class ProgressBar {
private:
    std::shared_ptr<Window> parentWindow;
    int x, y;                    // Position relative to parent
    int width, height;           // Dimensions
    double minValue, maxValue;   // Value range
    double currentValue;         // Current progress value
    bool visible;
    bool active;
    
    // Visual properties
    std::string fillChar;
    std::string emptyChar;
    std::string borderStyle;
    std::string fillColor;
    std::string emptyColor;
    std::string borderColor;
    std::string textColor;
    
    // Text display
    bool showPercentage;
    bool showValue;
    std::string customText;
    
    // Animation
    bool animated;
    int animationFrame;
    
    // Mouse interaction
    bool draggable;
    bool wasLeftPressed;
    
    void generateProgressEvent(EventType type, double oldValue, double newValue);
    void calculateDimensions();
    
public:
    ProgressBar(std::shared_ptr<Window> parent, int x, int y, int width, int height = 1);
    ~ProgressBar() = default;
    
    // Value management
    void setValue(double value);
    void setRange(double min, double max);
    void setPercentage(double percentage);
    double getValue() const { return currentValue; }
    double getPercentage() const;
    double getMinValue() const { return minValue; }
    double getMaxValue() const { return maxValue; }
    
    // Visual configuration
    void setFillChar(const std::string& ch) { fillChar = ch; }
    void setEmptyChar(const std::string& ch) { emptyChar = ch; }
    void setColors(const std::string& fill, const std::string& empty, const std::string& border = "", const std::string& text = "");
    void setBorderStyle(const std::string& style) { borderStyle = style; }
    
    // Text display options
    void setShowPercentage(bool show) { showPercentage = show; }
    void setShowValue(bool show) { showValue = show; }
    void setCustomText(const std::string& text) { customText = text; }
    
    // Animation
    void setAnimated(bool enabled) { animated = enabled; }
    void updateAnimation();
    
    // Interaction
    void setDraggable(bool enabled) { draggable = enabled; }
    void updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight);
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
    std::function<void(const ProgressBarEvent&)> onValueChange;
    std::function<void(const ProgressBarEvent&)> onComplete;
    std::function<void(const MouseEvent&)> onHover;
    std::function<void(const MouseEvent&)> onLeave;
    std::function<void(const MouseEvent&)> onClick;
    std::function<void(const MouseEvent&)> onDrag;
    
    // Event callback setters
    void setOnValueChange(std::function<void(const ProgressBarEvent&)> callback) { onValueChange = callback; }
    void setOnComplete(std::function<void(const ProgressBarEvent&)> callback) { onComplete = callback; }
    void setOnHover(std::function<void(const MouseEvent&)> callback) { onHover = callback; }
    void setOnLeave(std::function<void(const MouseEvent&)> callback) { onLeave = callback; }
    void setOnClick(std::function<void(const MouseEvent&)> callback) { onClick = callback; }
    void setOnDrag(std::function<void(const MouseEvent&)> callback) { onDrag = callback; }
};