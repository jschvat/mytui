#include "../include/tui_app.h"
#include "../include/dropdown_menu.h"
#include "../include/window.h"
#include <iostream>
#include <memory>

class DropdownDemoApp : public TUIApplication {
private:
    std::vector<std::shared_ptr<DropdownMenu>> menus;
    std::string statusMessage;
    std::string debugInfo;
    
public:
    DropdownDemoApp() {
        // Create sample dropdown menus
        createMenus();
        
        // Add some demo windows
        auto window1 = std::make_shared<Window>(10, 5, 30, 12, "Demo Window 1");
        auto window2 = std::make_shared<Window>(45, 8, 25, 10, "Demo Window 2");
        
        // Create scrollable test window with sample content
        auto scrollWindow = std::make_shared<Window>(15, 3, 50, 15, "Scrollable Content");
        createScrollableContent(scrollWindow);
        
        addWindow(window1);
        addWindow(window2);
        addWindow(scrollWindow);
        
        statusMessage = "Click on menu buttons to test dropdown functionality!";
    }
    
    void createMenus() {
        // File menu - better spaced across terminal width
        auto fileMenu = std::make_shared<DropdownMenu>(2, 1, "File");
        fileMenu->addItem("New", "Ctrl+N", [this]() { 
            statusMessage = "New file selected!"; 
        });
        fileMenu->addItem("Open", "Ctrl+O", [this]() { 
            statusMessage = "Open file selected!"; 
        });
        fileMenu->addItem("Save", "Ctrl+S", [this]() { 
            statusMessage = "Save file selected!"; 
        });
        fileMenu->addSeparator();
        fileMenu->addItem("Exit", "Alt+F4", [this]() { 
            statusMessage = "Exit selected! (Use Q to quit)"; 
        });
        
        // Edit menu - spaced at ~20% of terminal width
        auto editMenu = std::make_shared<DropdownMenu>(16, 1, "Edit");
        editMenu->addItem("Cut", "Ctrl+X", [this]() { 
            statusMessage = "Cut selected!"; 
        });
        editMenu->addItem("Copy", "Ctrl+C", [this]() { 
            statusMessage = "Copy selected!"; 
        });
        editMenu->addItem("Paste", "Ctrl+V", [this]() { 
            statusMessage = "Paste selected!"; 
        });
        editMenu->addSeparator();
        editMenu->addItem("Find", "Ctrl+F", [this]() { 
            statusMessage = "Find selected!"; 
        });
        editMenu->addItem("Replace", "Ctrl+H", [this]() { 
            statusMessage = "Replace selected!"; 
        });
        
        // View menu - spaced at ~40% of terminal width
        auto viewMenu = std::make_shared<DropdownMenu>(32, 1, "View");
        viewMenu->addItem("Zoom In", "Ctrl++", [this]() { 
            statusMessage = "Zoom In selected!"; 
        });
        viewMenu->addItem("Zoom Out", "Ctrl+-", [this]() { 
            statusMessage = "Zoom Out selected!"; 
        });
        viewMenu->addSeparator();
        viewMenu->addItem("Full Screen", "F11", [this]() { 
            statusMessage = "Full Screen selected!"; 
        });
        
        // Tools menu - spaced at ~60% of terminal width
        auto toolsMenu = std::make_shared<DropdownMenu>(48, 1, "Tools");
        toolsMenu->addItem("Options", [this]() { 
            statusMessage = "Options selected!"; 
        });
        toolsMenu->addItem("Preferences", [this]() { 
            statusMessage = "Preferences selected!"; 
        });
        toolsMenu->addSeparator();
        toolsMenu->addItem("Debug", [this]() { 
            statusMessage = "Debug mode activated!"; 
        });
        
        // Add all menus
        menus.push_back(fileMenu);
        menus.push_back(editMenu);
        menus.push_back(viewMenu);
        menus.push_back(toolsMenu);
    }
    
    void run() override {
        while (true) {
            // Handle mouse and keyboard input
            mouse.updateMouse();
            
            // Check for quit
            char ch;
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (ch == 'q' || ch == 'Q') {
                    break;
                }
            }
            
            // Update terminal size
            updateTerminalSize();
            if (buffer) {
                delete buffer;
                buffer = new UnicodeBuffer(term_width, term_height);
            }
            
            buffer->clear();
            drawBackground();
            
            // Update and draw menus first (so they appear on top)
            // Track which menu was just opened to close others
            std::vector<bool> wasOpen(menus.size());
            for (size_t i = 0; i < menus.size(); i++) {
                wasOpen[i] = menus[i]->isOpen();
            }
            
            for (auto& menu : menus) {
                menu->updateMouse(mouse, term_width, term_height);
            }
            
            // Debug: Check which menu is open and what selectedIndex is
            debugInfo = "";
            for (size_t i = 0; i < menus.size(); i++) {
                if (menus[i]->isOpen()) {
                    debugInfo = " | Menu " + std::to_string(i) + " selectedIndex: " + std::to_string(menus[i]->getSelectedIndex());
                    break;
                }
            }
            
            // Check if any menu just opened and close others
            for (size_t i = 0; i < menus.size(); i++) {
                if (!wasOpen[i] && menus[i]->isOpen()) {
                    // This menu just opened, close all others
                    for (size_t j = 0; j < menus.size(); j++) {
                        if (j != i && menus[j]->isOpen()) {
                            menus[j]->close();
                        }
                    }
                    break; // Only one menu can open per frame
                }
            }
            
            // Adjust menu positions to prevent overlap (simplified since only one menu can be open)
            DropdownMenu::adjustMenuPositions(menus, term_width);
            
            // Update windows
            for (auto& window : windows) {
                window->active = false;
            }
            
            // Update windows in reverse order (top window gets priority)
            for (int i = windows.size() - 1; i >= 0; i--) {
                if (windows[i]->isVisible()) {
                    windows[i]->updateMouse(mouse, term_width, term_height);
                    if (windows[i]->dragging || windows[i]->resizing) {
                        windows[i]->active = true;
                        // Move active window to front
                        if (i != windows.size() - 1) {
                            auto temp = windows[i];
                            windows.erase(windows.begin() + i);
                            windows.push_back(temp);
                        }
                        break;
                    }
                }
            }
            
            // Draw all visible windows
            for (auto& window : windows) {
                if (window->isVisible()) {
                    window->draw(*buffer);
                }
            }
            
            // Draw horizontal menu bar background first
            DropdownMenu::drawMenuBar(*buffer, 1, term_width);
            
            // Draw menus (on top of everything)
            for (auto& menu : menus) {
                menu->draw(*buffer);
            }
            
            // Draw enhanced mouse cursor
            drawMouseCursor();
            
            // Draw custom status bar with menu feedback
            drawCustomStatusBar();
            
            buffer->render();
            frame++;
            usleep(16000); // ~60 FPS
        }
    }
    
private:
    void drawCustomStatusBar() {
        // Clear status bar area
        for (int i = 0; i < term_width; i++) {
            buffer->setCell(i, term_height - 1, " ", Color::BLACK + Color::BG_BRIGHT_CYAN);
        }
        
        // Get current mouse position for debugging
        int mouseX = mouse.getMouseX();
        int mouseY = mouse.getMouseY();
        bool leftPressed = mouse.isLeftButtonPressed();
        
        // Draw status message with mouse debug info
        std::string displayMessage = " " + statusMessage + 
                                   " | Mouse: (" + std::to_string(mouseX) + "," + std::to_string(mouseY) + ")" +
                                   (leftPressed ? " [CLICK]" : "") + 
                                   debugInfo +
                                   " | Press Q to quit ";
        buffer->drawStringClipped(0, term_height - 1, displayMessage, 
                                Color::BLACK + Color::BG_BRIGHT_CYAN, term_width);
    }
    
    void createScrollableContent(std::shared_ptr<Window> window) {
        std::vector<std::string> content;
        
        // Create test content that demonstrates both horizontal and vertical scrolling
        content.push_back("=== SCROLLABLE WINDOW DEMONSTRATION ===");
        content.push_back("");
        content.push_back("This window demonstrates the new scrollbar functionality:");
        content.push_back("* Vertical scrolling when content exceeds window height");
        content.push_back("* Horizontal scrolling when lines exceed window width");
        content.push_back("* Scrollbars only appear when the window is active");
        content.push_back("* Click on this window to activate scrollbars");
        content.push_back("");
        content.push_back("VERTICAL SCROLLING TEST:");
        content.push_back("Line 1: The quick brown fox jumps over the lazy dog");
        content.push_back("Line 2: Pack my box with five dozen liquor jugs");
        content.push_back("Line 3: How vexingly quick daft zebras jump!");
        content.push_back("Line 4: Waltz, bad nymph, for quick jigs vex");
        content.push_back("Line 5: Sphinx of black quartz, judge my vow");
        content.push_back("Line 6: Two driven jocks help fax my big quiz");
        content.push_back("Line 7: Five quacking zephyrs jolt my wax bed");
        content.push_back("Line 8: The jay, pig, fox, zebra, and my wolves quack!");
        content.push_back("Line 9: Blowzy red vixens fight for a quick jump");
        content.push_back("Line 10: Joaquin Phoenix was quickly amazed by the five boxing wizards");
        content.push_back("");
        content.push_back("HORIZONTAL SCROLLING TEST - These lines are intentionally very long:");
        content.push_back("This is an extremely long line that should definitely exceed the width of most windows and trigger horizontal scrolling when displayed in the TUI interface. It contains enough text to demonstrate the horizontal scrollbar functionality.");
        content.push_back("Another super long line: ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789 !@#$%^&*()_+-=[]{}|;':\",./<>? This should force horizontal scrolling!");
        content.push_back("Yet another lengthy line with technical content: The implementation includes calculateContentDimensions(), scrollUp(), scrollDown(), scrollLeft(), scrollRight(), needsVerticalScrollbar(), needsHorizontalScrollbar(), and drawScrollbars() methods for complete scrolling support.");
        content.push_back("");
        content.push_back("SCROLLBAR CONTROLS:");
        content.push_back("* Mouse wheel (if supported) for vertical scrolling");
        content.push_back("* Arrow keys for directional scrolling");
        content.push_back("* Click and drag scrollbar thumbs");
        content.push_back("* Scrollbars automatically size based on content ratio");
        content.push_back("");
        content.push_back("MORE CONTENT TO SCROLL:");
        content.push_back("Line 20: Additional content line");
        content.push_back("Line 21: Even more content to scroll through");
        content.push_back("Line 22: Testing vertical scrolling capabilities");
        content.push_back("Line 23: The scrollbar thumb size reflects content ratio");
        content.push_back("Line 24: Scrollbar position shows current view location");
        content.push_back("Line 25: Final line of test content - END");
        
        window->setContent(content);
    }
};

int main() {
    try {
        DropdownDemoApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}