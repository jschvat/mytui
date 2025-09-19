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

// Text input events
class TextInputEvent : public Event {
public:
    std::weak_ptr<class TextInput> textInput;
    std::string oldText;
    std::string newText;
    char character;
    int cursorPosition;
    
    TextInputEvent(EventType type, std::shared_ptr<class TextInput> ti, const std::string& oldText, const std::string& newText, char ch = 0);
};

// Text input component
class TextInput {
protected:
    std::shared_ptr<Window> parentWindow;
    int x, y;                    // Position relative to parent
    int width, height;           // Dimensions
    std::string text;            // Current text content
    std::string placeholder;     // Placeholder text
    int cursorPos;               // Cursor position
    int scrollOffset;            // Horizontal scroll offset
    bool visible;
    bool active;
    bool enabled;
    bool focused;
    
    // Visual properties
    std::string borderColor;
    std::string textColor;
    std::string backgroundFill;
    std::string focusedBorderColor;
    std::string placeholderColor;
    std::string cursorColor;
    std::string selectionColor;
    
    // Input properties
    int maxLength;
    bool passwordMode;
    char passwordChar;
    std::string allowedChars;    // Empty = all chars allowed
    std::string forbiddenChars;  // Characters not allowed
    
    // Selection
    int selectionStart;
    int selectionEnd;
    bool hasSelection;
    
    // Mouse interaction
    bool wasLeftPressed;
    bool dragging;
    
    void generateTextEvent(EventType type, const std::string& oldText, const std::string& newText, char ch = 0);
    void calculateDimensions();
    void moveCursor(int newPos);
    void clearSelection();
    void deleteSelection();
    std::string getVisibleText() const;
    
public:
    TextInput(std::shared_ptr<Window> parent, int x, int y, int width, int height = 1);
    ~TextInput() = default;
    
    // Text management
    void setText(const std::string& newText);
    void appendText(const std::string& appendText);
    void insertText(const std::string& insertText);
    void clear();
    std::string getText() const { return text; }
    std::string getDisplayText() const;
    
    // Placeholder
    void setPlaceholder(const std::string& placeholder);
    std::string getPlaceholder() const { return placeholder; }
    
    // Input properties
    void setMaxLength(int length) { maxLength = length; }
    void setPasswordMode(bool enabled, char ch = '*') { passwordMode = enabled; passwordChar = ch; }
    void setAllowedChars(const std::string& chars) { allowedChars = chars; }
    void setForbiddenChars(const std::string& chars) { forbiddenChars = chars; }
    
    // Cursor management
    void setCursorPosition(int pos);
    int getCursorPosition() const { return cursorPos; }
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorHome();
    void moveCursorEnd();
    
    // Selection
    void selectAll();
    void selectRange(int start, int end);
    std::string getSelectedText() const;
    bool hasTextSelection() const { return hasSelection; }
    
    // Focus management
    void setFocused(bool focused);
    bool isFocused() const { return focused; }
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }
    
    // Character input
    bool insertCharacter(char ch);
    void deleteCharacter();
    void backspaceCharacter();
    
    // Visual configuration
    void setColors(const std::string& border, const std::string& text, const std::string& background = "", 
                   const std::string& focusedBorder = "", const std::string& placeholder = "", const std::string& cursor = "");
    
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
    std::function<void(const TextInputEvent&)> onTextChange;
    std::function<void(const TextInputEvent&)> onCharacterInput;
    std::function<void(const KeyboardEvent&)> onKeyPress;
    std::function<void(const MouseEvent&)> onFocus;
    std::function<void(const MouseEvent&)> onBlur;
    std::function<void(const MouseEvent&)> onHover;
    std::function<void(const MouseEvent&)> onLeave;
    std::function<void(const MouseEvent&)> onClick;
    
    // Event callback setters
    void setOnTextChange(std::function<void(const TextInputEvent&)> callback) { onTextChange = callback; }
    void setOnCharacterInput(std::function<void(const TextInputEvent&)> callback) { onCharacterInput = callback; }
    void setOnKeyPress(std::function<void(const KeyboardEvent&)> callback) { onKeyPress = callback; }
    void setOnFocus(std::function<void(const MouseEvent&)> callback) { onFocus = callback; }
    void setOnBlur(std::function<void(const MouseEvent&)> callback) { onBlur = callback; }
    void setOnHover(std::function<void(const MouseEvent&)> callback) { onHover = callback; }
    void setOnLeave(std::function<void(const MouseEvent&)> callback) { onLeave = callback; }
    void setOnClick(std::function<void(const MouseEvent&)> callback) { onClick = callback; }
};