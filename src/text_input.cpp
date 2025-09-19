#include "../include/text_input.h"
#include "../include/window.h"
#include <algorithm>
#include <cctype>

// C++14 compatible clamp function
template<typename T>
T clamp_value(const T& value, const T& min_val, const T& max_val) {
    return std::max(min_val, std::min(value, max_val));
}

// TextInputEvent implementation
TextInputEvent::TextInputEvent(EventType type, std::shared_ptr<TextInput> ti, const std::string& oldText, const std::string& newText, char ch)
    : Event(type), textInput(ti), oldText(oldText), newText(newText), character(ch) {
    if (ti) {
        cursorPosition = ti->getCursorPosition();
    } else {
        cursorPosition = 0;
    }
}

// TextInput implementation
TextInput::TextInput(std::shared_ptr<Window> parent, int x, int y, int width, int height)
    : parentWindow(parent), x(x), y(y), width(width), height(height),
      text(""), placeholder(""), cursorPos(0), scrollOffset(0),
      visible(true), active(false), enabled(true), focused(false),
      borderColor(Color::WHITE + Color::BG_BLACK), textColor(Color::BRIGHT_WHITE + Color::BG_BLACK),
      backgroundFill(" "), focusedBorderColor(Color::BRIGHT_CYAN + Color::BG_BLACK),
      placeholderColor(Color::CYAN + Color::BG_BLACK), cursorColor(Color::BLACK + Color::BG_BRIGHT_WHITE),
      selectionColor(Color::BLACK + Color::BG_BRIGHT_BLUE),
      maxLength(-1), passwordMode(false), passwordChar('*'), allowedChars(""), forbiddenChars(""),
      selectionStart(-1), selectionEnd(-1), hasSelection(false),
      wasLeftPressed(false), dragging(false) {
    calculateDimensions();
}

void TextInput::calculateDimensions() {
    // Ensure minimum dimensions
    if (width < 3) width = 3;
    if (height < 1) height = 1;
}

void TextInput::setText(const std::string& newText) {
    std::string oldText = text;
    text = newText;
    
    // Enforce max length
    if (maxLength > 0 && (int)text.length() > maxLength) {
        text = text.substr(0, maxLength);
    }
    
    // Adjust cursor position
    cursorPos = std::min(cursorPos, (int)text.length());
    clearSelection();
    
    if (oldText != text) {
        generateTextEvent(EventType::KEY_PRESS, oldText, text);
    }
}

void TextInput::appendText(const std::string& appendText) {
    setText(text + appendText);
}

void TextInput::insertText(const std::string& insertText) {
    if (!enabled) return;
    
    std::string oldText = text;
    
    // Delete selection first if exists
    if (hasSelection) {
        deleteSelection();
    }
    
    // Insert text at cursor position
    text.insert(cursorPos, insertText);
    
    // Enforce max length
    if (maxLength > 0 && (int)text.length() > maxLength) {
        text = text.substr(0, maxLength);
    }
    
    cursorPos += (int)insertText.length();
    cursorPos = std::min(cursorPos, (int)text.length());
    
    if (oldText != text) {
        generateTextEvent(EventType::KEY_PRESS, oldText, text);
    }
}

void TextInput::clear() {
    setText("");
}

std::string TextInput::getDisplayText() const {
    if (passwordMode) {
        return std::string(text.length(), passwordChar);
    }
    return text;
}

void TextInput::setPlaceholder(const std::string& newPlaceholder) {
    placeholder = newPlaceholder;
}

void TextInput::setCursorPosition(int pos) {
    cursorPos = clamp_value(pos, 0, (int)text.length());
    clearSelection();
}

void TextInput::moveCursorLeft() {
    if (cursorPos > 0) {
        cursorPos--;
        clearSelection();
    }
}

void TextInput::moveCursorRight() {
    if (cursorPos < (int)text.length()) {
        cursorPos++;
        clearSelection();
    }
}

void TextInput::moveCursorHome() {
    cursorPos = 0;
    clearSelection();
}

void TextInput::moveCursorEnd() {
    cursorPos = (int)text.length();
    clearSelection();
}

void TextInput::selectAll() {
    if (!text.empty()) {
        selectionStart = 0;
        selectionEnd = (int)text.length();
        hasSelection = true;
        cursorPos = selectionEnd;
    }
}

void TextInput::selectRange(int start, int end) {
    start = clamp_value(start, 0, (int)text.length());
    end = clamp_value(end, 0, (int)text.length());
    
    if (start != end) {
        selectionStart = std::min(start, end);
        selectionEnd = std::max(start, end);
        hasSelection = true;
        cursorPos = end;
    } else {
        clearSelection();
    }
}

std::string TextInput::getSelectedText() const {
    if (!hasSelection) return "";
    return text.substr(selectionStart, selectionEnd - selectionStart);
}

void TextInput::clearSelection() {
    hasSelection = false;
    selectionStart = -1;
    selectionEnd = -1;
}

void TextInput::deleteSelection() {
    if (!hasSelection) return;
    
    std::string oldText = text;
    text.erase(selectionStart, selectionEnd - selectionStart);
    cursorPos = selectionStart;
    clearSelection();
    
    if (oldText != text) {
        generateTextEvent(EventType::KEY_PRESS, oldText, text);
    }
}

void TextInput::setFocused(bool newFocused) {
    bool wasFocused = focused;
    focused = newFocused && enabled;
    
    if (focused && !wasFocused && onFocus) {
        auto event = MouseEvent(EventType::WINDOW_FOCUS, x, y);
        onFocus(event);
    } else if (!focused && wasFocused && onBlur) {
        auto event = MouseEvent(EventType::WINDOW_BLUR, x, y);
        onBlur(event);
        clearSelection();
    }
}

void TextInput::setEnabled(bool newEnabled) {
    enabled = newEnabled;
    if (!enabled) {
        setFocused(false);
        active = false;
    }
}

bool TextInput::insertCharacter(char ch) {
    if (!enabled || !focused) return false;
    
    // Check if character is allowed
    if (!allowedChars.empty() && allowedChars.find(ch) == std::string::npos) {
        return false;
    }
    
    if (!forbiddenChars.empty() && forbiddenChars.find(ch) != std::string::npos) {
        return false;
    }
    
    // Check printable character
    if (ch < 32 || ch > 126) return false;
    
    std::string oldText = text;
    
    // Delete selection first if exists
    if (hasSelection) {
        deleteSelection();
    }
    
    // Check max length
    if (maxLength > 0 && (int)text.length() >= maxLength) {
        return false;
    }
    
    // Insert character
    text.insert(cursorPos, 1, ch);
    cursorPos++;
    
    generateTextEvent(EventType::KEY_PRESS, oldText, text, ch);
    generateTextEvent(EventType::KEY_RELEASE, oldText, text, ch); // Character input event
    
    return true;
}

void TextInput::deleteCharacter() {
    if (!enabled || !focused) return;
    
    if (hasSelection) {
        deleteSelection();
        return;
    }
    
    if (cursorPos < (int)text.length()) {
        std::string oldText = text;
        text.erase(cursorPos, 1);
        generateTextEvent(EventType::KEY_PRESS, oldText, text);
    }
}

void TextInput::backspaceCharacter() {
    if (!enabled || !focused) return;
    
    if (hasSelection) {
        deleteSelection();
        return;
    }
    
    if (cursorPos > 0) {
        std::string oldText = text;
        text.erase(cursorPos - 1, 1);
        cursorPos--;
        generateTextEvent(EventType::KEY_PRESS, oldText, text);
    }
}

void TextInput::setColors(const std::string& border, const std::string& textCol, const std::string& background,
                         const std::string& focusedBorder, const std::string& placeholderCol, const std::string& cursor) {
    borderColor = border;
    textColor = textCol;
    if (!background.empty()) backgroundFill = background;
    if (!focusedBorder.empty()) focusedBorderColor = focusedBorder;
    if (!placeholderCol.empty()) placeholderColor = placeholderCol;
    if (!cursor.empty()) cursorColor = cursor;
}

std::string TextInput::getVisibleText() const {
    std::string displayText = getDisplayText();
    int maxVisible = width - 2; // Account for borders
    
    if ((int)displayText.length() <= maxVisible) {
        const_cast<TextInput*>(this)->scrollOffset = 0;
        return displayText;
    }
    
    // Adjust scroll offset to keep cursor visible
    int effectiveOffset = scrollOffset;
    if (cursorPos < effectiveOffset) {
        effectiveOffset = cursorPos;
    } else if (cursorPos >= effectiveOffset + maxVisible) {
        effectiveOffset = cursorPos - maxVisible + 1;
    }
    
    const_cast<TextInput*>(this)->scrollOffset = effectiveOffset;
    
    return displayText.substr(effectiveOffset, maxVisible);
}

bool TextInput::contains(int mx, int my) const {
    if (!parentWindow) return false;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    return mx >= absX && mx < absX + width && my >= absY && my < absY + height;
}

void TextInput::updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight) {
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
    
    if (isHovering && enabled) {
        // Handle focus
        if (leftPressed && !wasLeftPressed) {
            setFocused(true);
            
            // Position cursor based on click position
            int absX = parentWindow->x + x;
            int clickPos = mouseX - absX - 1; // Account for border
            clickPos = clamp_value(clickPos + scrollOffset, 0, (int)text.length());
            setCursorPosition(clickPos);
            
            if (onClick) {
                auto event = MouseEvent(EventType::MOUSE_PRESS, mouseX, mouseY);
                onClick(event);
            }
            
            dragging = true;
            selectionStart = cursorPos;
        }
    } else if (leftPressed && !wasLeftPressed) {
        // Clicked outside - lose focus
        setFocused(false);
    }
    
    // Handle text selection dragging
    if (dragging && leftPressed && enabled && focused) {
        int absX = parentWindow->x + x;
        int dragPos = mouseX - absX - 1; // Account for border
        dragPos = clamp_value(dragPos + scrollOffset, 0, (int)text.length());
        
        if (dragPos != selectionStart) {
            selectRange(selectionStart, dragPos);
        }
    }
    
    if (!leftPressed && wasLeftPressed) {
        dragging = false;
    }
    
    wasLeftPressed = leftPressed;
}

void TextInput::handleKeyboard(char ch, int keyCode) {
    if (!enabled || !focused) return;
    
    // Handle special keys
    switch (keyCode) {
        case 127: // Backspace
            backspaceCharacter();
            break;
        case 9: // Tab - ignore for now
            break;
        case 10: case 13: // Enter - could trigger submit event
            break;
        default:
            if (ch >= 32 && ch <= 126) {
                insertCharacter(ch);
            }
            break;
    }
    
    if (onKeyPress) {
        auto event = KeyboardEvent(EventType::KEY_PRESS, ch, keyCode);
        onKeyPress(event);
    }
}

void TextInput::draw(UnicodeBuffer& buffer) {
    if (!visible || !parentWindow || !parentWindow->isVisible()) return;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    // Determine border color
    std::string currentBorderColor = focused ? focusedBorderColor : borderColor;
    
    // Draw border
    for (int row = 0; row < height; row++) {
        buffer.setCell(absX, absY + row, "|", currentBorderColor);
        buffer.setCell(absX + width - 1, absY + row, "|", currentBorderColor);
        
        // Fill background
        for (int col = 1; col < width - 1; col++) {
            buffer.setCell(absX + col, absY + row, backgroundFill, textColor);
        }
    }
    
    // Draw text content
    std::string visibleText = getVisibleText();
    std::string displayText = text.empty() && !focused ? placeholder : visibleText;
    std::string displayColor = text.empty() && !focused ? placeholderColor : textColor;
    
    if (!displayText.empty()) {
        buffer.drawStringClipped(absX + 1, absY, displayText, displayColor, absX + width - 1);
    }
    
    // Draw selection
    if (hasSelection && focused) {
        int visibleSelStart = std::max(0, selectionStart - scrollOffset);
        int visibleSelEnd = std::min((int)visibleText.length(), selectionEnd - scrollOffset);
        
        for (int i = visibleSelStart; i < visibleSelEnd; i++) {
            if (i >= 0 && i < (int)visibleText.length()) {
                std::string ch = visibleText.substr(i, 1);
                buffer.setCell(absX + 1 + i, absY, ch, selectionColor);
            }
        }
    }
    
    // Draw cursor
    if (focused && !hasSelection) {
        int cursorX = absX + 1 + (cursorPos - scrollOffset);
        if (cursorX >= absX + 1 && cursorX < absX + width - 1) {
            std::string cursorChar = (cursorPos < (int)visibleText.length()) ? 
                                   visibleText.substr(cursorPos - scrollOffset, 1) : " ";
            buffer.setCell(cursorX, absY, cursorChar, cursorColor);
        }
    }
}

void TextInput::generateTextEvent(EventType type, const std::string& oldText, const std::string& newText, char ch) {
    auto self = std::shared_ptr<TextInput>(this, [](TextInput*) {}); // Non-owning shared_ptr
    auto event = std::unique_ptr<TextInputEvent>(new TextInputEvent(type, self, oldText, newText, ch));
    
    // Call local callbacks first
    switch (type) {
        case EventType::KEY_PRESS:
            if (onTextChange) onTextChange(*event);
            break;
        case EventType::KEY_RELEASE:
            if (onCharacterInput) onCharacterInput(*event);
            break;
        default:
            break;
    }
    
    // Dispatch to global event manager
    EventManager::getInstance().dispatchEvent(std::move(event));
}