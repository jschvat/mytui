#include "../include/progress_bar.h"
#include "../include/window.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

// C++14 compatible clamp function
template<typename T>
T clamp_value(const T& value, const T& min_val, const T& max_val) {
    return std::max(min_val, std::min(value, max_val));
}

// ProgressBarEvent implementation
ProgressBarEvent::ProgressBarEvent(EventType type, std::shared_ptr<ProgressBar> pb, double oldVal, double newVal)
    : Event(type), progressBar(pb), oldValue(oldVal), newValue(newVal) {
    if (pb) {
        percentage = (newVal - pb->getMinValue()) / (pb->getMaxValue() - pb->getMinValue()) * 100.0;
    } else {
        percentage = 0.0;
    }
}

// ProgressBar implementation
ProgressBar::ProgressBar(std::shared_ptr<Window> parent, int x, int y, int width, int height)
    : parentWindow(parent), x(x), y(y), width(width), height(height),
      minValue(0.0), maxValue(100.0), currentValue(0.0), visible(true), active(false),
      fillChar("█"), emptyChar("░"), borderStyle("single"),
      fillColor(Color::GREEN + Color::BG_BLACK), emptyColor(Color::CYAN + Color::BG_BLACK),
      borderColor(Color::WHITE + Color::BG_BLACK), textColor(Color::WHITE + Color::BG_BLACK),
      showPercentage(true), showValue(false), customText(""),
      animated(false), animationFrame(0), draggable(false), wasLeftPressed(false) {
    calculateDimensions();
}

void ProgressBar::calculateDimensions() {
    // Ensure minimum dimensions
    if (width < 3) width = 3;
    if (height < 1) height = 1;
}

void ProgressBar::setValue(double value) {
    double oldValue = currentValue;
    currentValue = clamp_value(value, minValue, maxValue);
    
    if (oldValue != currentValue) {
        generateProgressEvent(EventType::BUTTON_CLICK, oldValue, currentValue); // Reusing existing event type
        
        // Check if progress is complete
        if (currentValue >= maxValue && oldValue < maxValue) {
            generateProgressEvent(EventType::BUTTON_TOGGLE, oldValue, currentValue); // Complete event
        }
    }
}

void ProgressBar::setRange(double min, double max) {
    if (min >= max) return;
    
    minValue = min;
    maxValue = max;
    currentValue = clamp_value(currentValue, minValue, maxValue);
}

void ProgressBar::setPercentage(double percentage) {
    double value = minValue + (percentage / 100.0) * (maxValue - minValue);
    setValue(value);
}

double ProgressBar::getPercentage() const {
    if (maxValue <= minValue) return 0.0;
    return ((currentValue - minValue) / (maxValue - minValue)) * 100.0;
}

void ProgressBar::setColors(const std::string& fill, const std::string& empty, const std::string& border, const std::string& text) {
    fillColor = fill;
    emptyColor = empty;
    if (!border.empty()) borderColor = border;
    if (!text.empty()) textColor = text;
}

void ProgressBar::updateAnimation() {
    if (!animated) return;
    
    animationFrame = (animationFrame + 1) % 4;
}

bool ProgressBar::contains(int mx, int my) const {
    if (!parentWindow) return false;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    return mx >= absX && mx < absX + width && my >= absY && my < absY + height;
}

void ProgressBar::updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight) {
    if (!visible || !parentWindow || !parentWindow->isVisible()) return;
    
    int mouseX = mouse.getMouseX();
    int mouseY = mouse.getMouseY();
    bool leftPressed = mouse.isLeftButtonPressed();
    
    bool isHovering = contains(mouseX, mouseY);
    bool wasHovering = active;
    active = isHovering;
    
    // Generate hover/leave events
    if (isHovering && !wasHovering && onHover) {
        auto event = MouseEvent(EventType::MOUSE_ENTER, mouseX, mouseY);
        onHover(event);
    } else if (!isHovering && wasHovering && onLeave) {
        auto event = MouseEvent(EventType::MOUSE_LEAVE, mouseX, mouseY);
        onLeave(event);
    }
    
    if (isHovering) {
        // Handle click events
        if (leftPressed && !wasLeftPressed && onClick) {
            auto event = MouseEvent(EventType::MOUSE_PRESS, mouseX, mouseY);
            onClick(event);
        }
        
        // Handle dragging for interactive progress bars
        if (draggable && leftPressed && onDrag) {
            int absX = parentWindow->x + x;
            double relativePos = clamp_value((double)(mouseX - absX) / width, 0.0, 1.0);
            double newValue = minValue + relativePos * (maxValue - minValue);
            setValue(newValue);
            
            auto event = MouseEvent(EventType::MOUSE_DRAG, mouseX, mouseY);
            onDrag(event);
        }
    }
    
    wasLeftPressed = leftPressed;
}

void ProgressBar::draw(UnicodeBuffer& buffer) {
    if (!visible || !parentWindow || !parentWindow->isVisible()) return;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    // Calculate progress
    double progress = getPercentage() / 100.0;
    int filledChars = (int)(progress * width);
    
    // Draw progress bar
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            std::string cellChar;
            std::string cellColor;
            
            if (col < filledChars) {
                cellChar = fillChar;
                cellColor = fillColor;
                
                // Add animation effect
                if (animated && col == filledChars - 1) {
                    switch (animationFrame) {
                        case 0: cellChar = "▉"; break;
                        case 1: cellChar = "▊"; break;
                        case 2: cellChar = "▋"; break;
                        case 3: cellChar = "▌"; break;
                    }
                }
            } else {
                cellChar = emptyChar;
                cellColor = emptyColor;
            }
            
            buffer.setCell(absX + col, absY + row, cellChar, cellColor);
        }
    }
    
    // Draw text overlay if enabled
    if (height > 0 && (showPercentage || showValue || !customText.empty())) {
        std::string displayText;
        
        if (!customText.empty()) {
            displayText = customText;
        } else if (showPercentage && showValue) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << getPercentage() << "% (" 
                << std::setprecision(2) << currentValue << ")";
            displayText = oss.str();
        } else if (showPercentage) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << getPercentage() << "%";
            displayText = oss.str();
        } else if (showValue) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << currentValue;
            displayText = oss.str();
        }
        
        // Center the text
        int textX = absX + (width - (int)displayText.length()) / 2;
        int textY = absY + height / 2;
        
        if (textX >= absX && textX + (int)displayText.length() <= absX + width) {
            buffer.drawStringClipped(textX, textY, displayText, textColor, absX + width);
        }
    }
    
    // Draw border if specified
    if (borderStyle == "single" && height > 1) {
        // Simple border for multi-line progress bars
        buffer.drawBox(absX - 1, absY - 1, width + 2, height + 2, borderColor, true, false);
    }
}

void ProgressBar::generateProgressEvent(EventType type, double oldValue, double newValue) {
    auto self = std::shared_ptr<ProgressBar>(this, [](ProgressBar*) {}); // Non-owning shared_ptr
    auto event = std::unique_ptr<ProgressBarEvent>(new ProgressBarEvent(type, self, oldValue, newValue));
    
    // Call local callbacks first
    if (type == EventType::BUTTON_CLICK && onValueChange) { // Value change event
        onValueChange(*event);
    } else if (type == EventType::BUTTON_TOGGLE && onComplete) { // Complete event
        onComplete(*event);
    }
    
    // Dispatch to global event manager
    EventManager::getInstance().dispatchEvent(std::move(event));
}