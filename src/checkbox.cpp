#include "../include/checkbox.h"
#include "../include/window.h"
#include "../include/component_clipping.h"
#include <algorithm>

// CheckboxEvent implementation
CheckboxEvent::CheckboxEvent(EventType type, std::shared_ptr<Checkbox> cb, bool oldState, bool newState, const std::string& label)
    : Event(type), checkbox(cb), oldState(oldState), newState(newState), label(label) {
}

// Checkbox implementation
Checkbox::Checkbox(std::shared_ptr<Window> parent, int x, int y, const std::string& label, bool initialState)
    : parentWindow(parent), x(x), y(y), label(label), checked(initialState), 
      visible(true), active(false), enabled(true),
      checkedChar("✓"), uncheckedChar(" "),
      boxColor(Color::WHITE + Color::BG_BLACK), labelColor(Color::BRIGHT_WHITE + Color::BG_BLACK),
      activeColor(Color::BLACK + Color::BG_BRIGHT_WHITE), disabledColor(Color::CYAN + Color::BG_BLACK),
      wasLeftPressed(false) {
    calculateDimensions();
}

void Checkbox::calculateDimensions() {
    // Width: [X] + space + label
    width = 4 + (int)label.length(); // [X] label
    height = 1;
}

void Checkbox::setChecked(bool state) {
    bool oldState = checked;
    checked = state;
    
    if (oldState != checked) {
        generateCheckboxEvent(EventType::BUTTON_TOGGLE, oldState, checked);
        
        if (checked) {
            generateCheckboxEvent(EventType::BUTTON_PRESS, oldState, checked); // Check event
        } else {
            generateCheckboxEvent(EventType::BUTTON_RELEASE, oldState, checked); // Uncheck event
        }
    }
}

void Checkbox::toggle() {
    if (enabled) {
        setChecked(!checked);
    }
}

void Checkbox::setEnabled(bool state) {
    enabled = state;
    if (!enabled) {
        active = false;
    }
}

void Checkbox::setLabel(const std::string& newLabel) {
    label = newLabel;
    calculateDimensions();
}

void Checkbox::setColors(const std::string& box, const std::string& labelCol, const std::string& active, const std::string& disabled) {
    boxColor = box;
    labelColor = labelCol;
    if (!active.empty()) activeColor = active;
    if (!disabled.empty()) disabledColor = disabled;
}

bool Checkbox::contains(int mx, int my) const {
    if (!parentWindow) return false;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    return mx >= absX && mx < absX + width && my >= absY && my < absY + height;
}

void Checkbox::updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight) {
    if (!visible || !enabled || !parentWindow || !parentWindow->isVisible()) return;
    
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
        if (leftPressed && !wasLeftPressed) {
            toggle();
            
            if (onClick) {
                auto event = MouseEvent(EventType::MOUSE_PRESS, mouseX, mouseY);
                onClick(event);
            }
        }
    }
    
    wasLeftPressed = leftPressed;
}

void Checkbox::draw(UnicodeBuffer& buffer) {
    if (!visible || !parentWindow || !parentWindow->isVisible()) return;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    // Determine colors based on state
    std::string currentBoxColor = boxColor;
    std::string currentLabelColor = labelColor;
    
    if (!enabled) {
        currentBoxColor = disabledColor;
        currentLabelColor = disabledColor;
    } else if (active) {
        currentBoxColor = activeColor;
        currentLabelColor = activeColor;
    }
    
    // Calculate clipping bounds
    auto clipBounds = ComponentClipping::calculateClipBounds(parentWindow, x, y, width, height);
    if (clipBounds.isEmpty) return;
    
    // Draw checkbox box with clipping
    if (ComponentClipping::shouldDraw(clipBounds, absX, absY)) {
        buffer.setCell(absX, absY, "[", currentBoxColor);
    }
    if (ComponentClipping::shouldDraw(clipBounds, absX + 1, absY)) {
        buffer.setCell(absX + 1, absY, checked ? checkedChar : uncheckedChar, currentBoxColor);
    }
    if (ComponentClipping::shouldDraw(clipBounds, absX + 2, absY)) {
        buffer.setCell(absX + 2, absY, "]", currentBoxColor);
    }
    if (ComponentClipping::shouldDraw(clipBounds, absX + 3, absY)) {
        buffer.setCell(absX + 3, absY, " ", currentLabelColor);
    }
    
    // Draw label with clipping
    if (!label.empty()) {
        int labelStartX = std::max(absX + 4, clipBounds.startX);
        int labelEndX = std::min(absX + width, clipBounds.endX);
        
        if (labelStartX < labelEndX && absY >= clipBounds.startY && absY < clipBounds.endY) {
            // Calculate which part of the label to show
            int labelOffset = labelStartX - (absX + 4);
            int labelLength = labelEndX - labelStartX;
            
            if (labelOffset >= 0 && labelOffset < (int)label.length()) {
                std::string clippedLabel = label.substr(labelOffset, labelLength);
                buffer.drawStringClipped(labelStartX, absY, clippedLabel, currentLabelColor, labelEndX);
            }
        }
    }
}

void Checkbox::generateCheckboxEvent(EventType type, bool oldState, bool newState) {
    auto self = std::shared_ptr<Checkbox>(this, [](Checkbox*) {}); // Non-owning shared_ptr
    auto event = std::unique_ptr<CheckboxEvent>(new CheckboxEvent(type, self, oldState, newState, label));
    
    // Call local callbacks first
    switch (type) {
        case EventType::BUTTON_TOGGLE:
            if (onStateChange) onStateChange(*event);
            break;
        case EventType::BUTTON_PRESS:
            if (newState && onCheck) onCheck(*event);
            break;
        case EventType::BUTTON_RELEASE:
            if (!newState && onUncheck) onUncheck(*event);
            break;
        default:
            break;
    }
    
    // Dispatch to global event manager
    EventManager::getInstance().dispatchEvent(std::move(event));
}