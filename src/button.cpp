#include "../include/button.h"
#include "../include/window.h"
#include "../include/colors.h"
#include <algorithm>

Button::Button(int x, int y, int w, int h, const std::string& text, 
               std::shared_ptr<Window> parent, ButtonType type)
    : x(x), y(y), w(w), h(h), text(text), type(type), state(ButtonState::NORMAL),
      enabled(type != ButtonType::DISABLED), toggled(false), parentWindow(parent),
      showShadow(true), wasLeftPressed(false), isHovered(false), wasHovered(false) {
    
    // Set default colors for shadow-based design
    switch (type) {
        case ButtonType::REGULAR:
            textColor = Color::BLACK;
            backgroundColor = Color::BG_WHITE;
            hoverColor = Color::BG_BRIGHT_CYAN;
            pressedColor = Color::BG_BLUE;
            shadowColor = Color::BLACK;
            break;
        case ButtonType::TOGGLE:
            textColor = Color::BLACK;
            backgroundColor = Color::BG_WHITE;
            hoverColor = Color::BG_BRIGHT_CYAN;
            pressedColor = Color::BG_BLUE;
            toggledColor = Color::BG_GREEN;
            shadowColor = Color::BLACK;
            break;
        case ButtonType::DISABLED:
            textColor = Color::BLACK;
            backgroundColor = Color::BG_BLACK;
            hoverColor = Color::BG_BLACK;
            pressedColor = Color::BG_BLACK;
            disabledColor = Color::BG_BLACK;
            shadowColor = Color::BLACK;
            enabled = false;
            break;
    }
}

void Button::draw(UnicodeBuffer& buffer) {
    // Only draw if we have a valid parent window
    auto parent = parentWindow.lock();
    if (!parent || !parent->isVisible()) return;
    
    // Get absolute position relative to screen
    int absX = getAbsoluteX();
    int absY = getAbsoluteY();
    
    // Check if button is within parent window bounds
    if (absX + w > parent->x + parent->w - 1 || absY + h > parent->y + parent->h - 1) {
        return; // Button extends outside parent window
    }
    
    // Draw shadow first (bottom-right offset by 1)
    if (showShadow && !isPressed()) {
        drawShadow(buffer);
    }
    
    // Draw button background
    drawBackground(buffer);
    
    // Draw text content
    drawText(buffer);
}

void Button::updateMouse(FastMouseHandler& mouse) {
    if (!enabled) return;
    
    // Only process mouse if we have a valid parent window
    auto parent = parentWindow.lock();
    if (!parent || !parent->isVisible()) return;
    
    int mouseX = mouse.getMouseX();
    int mouseY = mouse.getMouseY();
    bool leftPressed = mouse.isLeftButtonPressed();
    bool mouseOver = contains(mouseX, mouseY);
    
    // Generate mouse enter/leave events
    generateMouseEvents(mouse);
    
    // Update hover state
    isHovered = mouseOver;
    
    // Handle click events
    if (leftPressed && !wasLeftPressed && mouseOver) {
        // Button press started
        updateState(mouseOver, true);
        
        // Generate press event
        generateButtonEvent(EventType::BUTTON_PRESS);
        
        if (type == ButtonType::TOGGLE) {
            toggled = !toggled;
            // Generate toggle event
            generateButtonEvent(EventType::BUTTON_TOGGLE);
            if (onToggle) {
                onToggle(toggled);
            }
        }
        
        // Generate click event
        generateButtonEvent(EventType::BUTTON_CLICK);
        
        if (onClick) {
            onClick();
        }
    } else if (!leftPressed && wasLeftPressed) {
        // Button release
        updateState(mouseOver, false);
        
        // Generate release event if released over the button
        if (mouseOver) {
            generateButtonEvent(EventType::BUTTON_RELEASE);
        }
    } else {
        // Update state based on mouse position
        updateState(mouseOver, leftPressed && mouseOver);
    }
    
    wasLeftPressed = leftPressed;
}

void Button::updateState(bool mouseOver, bool mousePressed) {
    if (!enabled) {
        state = ButtonState::NORMAL;
        return;
    }
    
    if (type == ButtonType::TOGGLE && toggled) {
        state = ButtonState::TOGGLED_ON;
    } else if (mousePressed && mouseOver) {
        state = ButtonState::PRESSED;
    } else if (mouseOver) {
        state = ButtonState::HOVERED;
    } else {
        state = ButtonState::NORMAL;
    }
}

void Button::drawShadow(UnicodeBuffer& buffer) {
    if (!showShadow) return;
    
    int absX = getAbsoluteX();
    int absY = getAbsoluteY();
    
    // Draw shadow with offset (bottom and right edges)
    std::string shadowColorStr = shadowColor + Color::BG_BLACK;
    
    // Right edge shadow
    for (int row = 1; row <= h; row++) {
        if (absY + row < absY + h + 1) {
            buffer.setCell(absX + w, absY + row, Unicode::MEDIUM_SHADE, shadowColorStr);
        }
    }
    
    // Bottom edge shadow  
    for (int col = 1; col <= w; col++) {
        if (absX + col < absX + w + 1) {
            buffer.setCell(absX + col, absY + h, Unicode::MEDIUM_SHADE, shadowColorStr);
        }
    }
}

void Button::drawBackground(UnicodeBuffer& buffer) {
    int absX = getAbsoluteX();
    int absY = getAbsoluteY();
    
    // Get current background color based on state
    std::string bgColor = getCurrentBackgroundColor();
    
    // Adjust position for pressed state (simulate button press)
    int offsetX = isPressed() ? 1 : 0;
    int offsetY = isPressed() ? 1 : 0;
    
    // Draw button background
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            buffer.setCell(absX + col + offsetX, absY + row + offsetY, " ", bgColor);
        }
    }
}

void Button::drawText(UnicodeBuffer& buffer) {
    int absX = getAbsoluteX();
    int absY = getAbsoluteY();
    
    std::string displayText = getDisplayText();
    
    // Adjust position for pressed state
    int offsetX = isPressed() ? 1 : 0;
    int offsetY = isPressed() ? 1 : 0;
    
    // Calculate text position (centered)
    int textX = absX + offsetX + (w - (int)displayText.length()) / 2;
    int textY = absY + offsetY + h / 2;
    
    // Ensure text fits within button bounds
    textX = std::max(absX + offsetX, textX);
    textX = std::min(absX + offsetX + w - (int)displayText.length(), textX);
    textY = std::max(absY + offsetY, textY);
    textY = std::min(absY + offsetY + h, textY);
    
    // Get text and background colors
    std::string txtColor = getCurrentTextColor();
    std::string bgColor = getCurrentBackgroundColor();
    
    // Draw the text
    buffer.drawStringClipped(textX, textY, displayText, txtColor + bgColor, absX + offsetX + w);
}

std::string Button::getDisplayText() const {
    if (type == ButtonType::TOGGLE) {
        if (toggled) {
            return Unicode::CHECK + " " + text;
        } else {
            return Unicode::CIRCLE + " " + text;
        }
    }
    
    return text;
}

std::string Button::getCurrentBackgroundColor() const {
    if (!enabled) {
        return disabledColor.empty() ? backgroundColor : disabledColor;
    }
    
    switch (state) {
        case ButtonState::PRESSED:
            return pressedColor;
        case ButtonState::HOVERED:
            return hoverColor;
        case ButtonState::TOGGLED_ON:
            return toggledColor.empty() ? pressedColor : toggledColor;
        case ButtonState::NORMAL:
        default:
            return backgroundColor;
    }
}

std::string Button::getCurrentTextColor() const {
    if (!enabled) {
        return Color::BLACK; // Dark text for disabled buttons
    }
    
    // For pressed or toggled buttons, use white text for contrast
    if (state == ButtonState::PRESSED || state == ButtonState::TOGGLED_ON) {
        return Color::WHITE;
    }
    
    return textColor;
}

std::string Button::getStateDescription() const {
    switch (state) {
        case ButtonState::NORMAL: return "Normal";
        case ButtonState::HOVERED: return "Hovered";
        case ButtonState::PRESSED: return "Pressed";
        case ButtonState::TOGGLED_ON: return "Toggled ON";
        case ButtonState::TOGGLED_OFF: return "Toggled OFF";
        default: return "Unknown";
    }
}

// Position helper methods
int Button::getAbsoluteX() const {
    auto parent = parentWindow.lock();
    if (!parent) return x;
    return parent->getContentX() + x;
}

int Button::getAbsoluteY() const {
    auto parent = parentWindow.lock();
    if (!parent) return y;
    return parent->getContentY() + y;
}

// Utility methods
bool Button::contains(int mx, int my) const {
    int absX = getAbsoluteX();
    int absY = getAbsoluteY();
    return mx >= absX && mx < absX + w && my >= absY && my < absY + h;
}

bool Button::containsRelative(int mx, int my) const {
    return mx >= x && mx < x + w && my >= y && my < y + h;
}

bool Button::isPressed() const {
    return state == ButtonState::PRESSED;
}

bool Button::isToggleButton() const {
    return type == ButtonType::TOGGLE;
}

// State management methods
void Button::setEnabled(bool newEnabled) {
    enabled = newEnabled;
    if (!enabled) {
        state = ButtonState::NORMAL;
    }
}

void Button::setToggled(bool newToggled) {
    if (type == ButtonType::TOGGLE) {
        toggled = newToggled;
        state = toggled ? ButtonState::TOGGLED_ON : ButtonState::NORMAL;
    }
}

void Button::setText(const std::string& newText) {
    text = newText;
}

void Button::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

void Button::setSize(int newW, int newH) {
    w = std::max(3, newW); // Minimum width for usability
    h = std::max(1, newH); // Minimum height for usability
}

// Event handler setters
void Button::setOnClick(std::function<void()> callback) {
    onClick = callback;
}

void Button::setOnToggle(std::function<void(bool)> callback) {
    onToggle = callback;
}

// Parent window management
void Button::setParentWindow(std::shared_ptr<Window> parent) {
    parentWindow = parent;
}

std::shared_ptr<Window> Button::getParentWindow() const {
    return parentWindow.lock();
}

// Style customization
void Button::setColors(const std::string& text, const std::string& background,
                      const std::string& hover, const std::string& pressed,
                      const std::string& disabled, const std::string& toggledOn) {
    textColor = text;
    backgroundColor = background;
    hoverColor = hover;
    pressedColor = pressed;
    if (!disabled.empty()) disabledColor = disabled;
    if (!toggledOn.empty()) toggledColor = toggledOn;
}

void Button::setShadow(const std::string& shadowColorStr, bool enable) {
    shadowColor = shadowColorStr;
    showShadow = enable;
}

// Event generation methods
void Button::generateButtonEvent(EventType type) {
    auto parent = parentWindow.lock();
    if (!parent) return;
    
    auto self = std::shared_ptr<Button>(this, [](Button*) {}); // Non-owning shared_ptr
    auto event = std::unique_ptr<ButtonEvent>(new ButtonEvent(type, self));
    
    // Set additional event data
    event->buttonText = text;
    event->buttonX = x;
    event->buttonY = y;
    
    if (type == EventType::BUTTON_TOGGLE) {
        event->toggleState = toggled;
    }
    
    // Call local callbacks first
    switch (type) {
        case EventType::BUTTON_CLICK:
            if (onButtonClick) onButtonClick(*event);
            break;
        case EventType::BUTTON_PRESS:
            if (onButtonPress) onButtonPress(*event);
            break;
        case EventType::BUTTON_RELEASE:
            if (onButtonRelease) onButtonRelease(*event);
            break;
        case EventType::BUTTON_TOGGLE:
            if (onButtonToggle) onButtonToggle(*event);
            break;
        default:
            break;
    }
    
    // Dispatch to global event manager
    EventManager::getInstance().dispatchEvent(std::move(event));
}

void Button::generateMouseEvents(const FastMouseHandler& mouse) {
    int mouseX = mouse.getMouseX();
    int mouseY = mouse.getMouseY();
    bool mouseOver = contains(mouseX, mouseY);
    
    // Generate mouse enter/leave events for buttons
    if (mouseOver && !wasHovered) {
        auto hoverEvent = std::unique_ptr<MouseEvent>(new MouseEvent(EventType::MOUSE_ENTER, mouseX, mouseY));
        if (onButtonHover) onButtonHover(*hoverEvent);
        EventManager::getInstance().dispatchEvent(std::move(hoverEvent));
        wasHovered = true;
    } else if (!mouseOver && wasHovered) {
        auto leaveEvent = std::unique_ptr<MouseEvent>(new MouseEvent(EventType::MOUSE_LEAVE, mouseX, mouseY));
        if (onButtonLeave) onButtonLeave(*leaveEvent);
        EventManager::getInstance().dispatchEvent(std::move(leaveEvent));
        wasHovered = false;
    }
}