#pragma once

#include "buffer.h"
#include "mouse_handler.h"
#include "colors.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

// Callback function type for menu items
using MenuCallback = std::function<void()>;

// Individual menu item
struct MenuItem {
    std::string text;
    std::string shortcut;  // Keyboard shortcut (e.g., "Ctrl+N", "F1")
    MenuCallback callback;
    bool enabled;
    bool separator;
    
    MenuItem(const std::string& text, const std::string& shortcut = "", MenuCallback callback = nullptr, bool enabled = true, bool separator = false)
        : text(text), shortcut(shortcut), callback(callback), enabled(enabled), separator(separator) {}
};

// Dropdown menu component
class DropdownMenu {
private:
    std::vector<MenuItem> items;
    int x, y;                    // Position of menu
    int width, height;           // Calculated dimensions
    bool visible;
    bool active;
    int selectedIndex;           // Currently highlighted item
    int triggerX, triggerY;      // Position of trigger area (for screen menus)
    int triggerWidth, triggerHeight;
    std::string title;           // Menu title/trigger text
    
    // Visual state
    bool wasLeftPressed;
    bool menuOpen;
    
    void calculateDimensions();
    void drawTrigger(UnicodeBuffer& buffer);
    void drawMenu(UnicodeBuffer& buffer);
    bool triggerContains(int mx, int my) const;
    bool menuContains(int mx, int my) const;
    int getItemAtPosition(int mx, int my) const;
    
public:
    DropdownMenu(int x, int y, const std::string& title);
    ~DropdownMenu() = default;
    
    // Menu management
    void addItem(const std::string& text, MenuCallback callback);
    void addItem(const std::string& text, const std::string& shortcut, MenuCallback callback);
    void addSeparator();
    void clearItems();
    
    // Interaction
    void updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight);
    void draw(UnicodeBuffer& buffer);
    void executeCallback(int index);
    
    // Static utility for drawing menu bar background
    static void drawMenuBar(UnicodeBuffer& buffer, int y, int termWidth);
    
    // State management
    void show() { visible = true; }
    void hide() { visible = false; menuOpen = false; }
    bool isVisible() const { return visible; }
    bool isOpen() const { return menuOpen; }
    void close() { menuOpen = false; selectedIndex = -1; }
    
    // Position management
    void setPosition(int newX, int newY) { x = newX; y = newY; calculateDimensions(); }
    void setTitle(const std::string& newTitle) { title = newTitle; calculateDimensions(); }
    
    // Getters
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};