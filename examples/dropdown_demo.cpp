#include "../include/tui_app.h"
#include "../include/dropdown_menu.h"
#include "../include/window.h"
#include <iostream>
#include <memory>

class DropdownDemoApp : public TUIApplication {
private:
    std::vector<std::shared_ptr<DropdownMenu>> menus;
    std::string statusMessage;
    
public:
    DropdownDemoApp() {
        // Create sample dropdown menus
        createMenus();
        
        // Add some demo windows
        auto window1 = std::make_shared<Window>(10, 5, 30, 12, "Demo Window 1");
        auto window2 = std::make_shared<Window>(45, 8, 25, 10, "Demo Window 2");
        
        addWindow(window1);
        addWindow(window2);
        
        statusMessage = "Click on menu buttons to test dropdown functionality!";
    }
    
    void createMenus() {
        // File menu
        auto fileMenu = std::make_shared<DropdownMenu>(2, 1, "File");
        fileMenu->addItem("New", [this]() { 
            statusMessage = "New file selected!"; 
        });
        fileMenu->addItem("Open", [this]() { 
            statusMessage = "Open file selected!"; 
        });
        fileMenu->addItem("Save", [this]() { 
            statusMessage = "Save file selected!"; 
        });
        fileMenu->addSeparator();
        fileMenu->addItem("Exit", [this]() { 
            statusMessage = "Exit selected! (Use Q to quit)"; 
        });
        
        // Edit menu
        auto editMenu = std::make_shared<DropdownMenu>(10, 1, "Edit");
        editMenu->addItem("Cut", [this]() { 
            statusMessage = "Cut selected!"; 
        });
        editMenu->addItem("Copy", [this]() { 
            statusMessage = "Copy selected!"; 
        });
        editMenu->addItem("Paste", [this]() { 
            statusMessage = "Paste selected!"; 
        });
        editMenu->addSeparator();
        editMenu->addItem("Find", [this]() { 
            statusMessage = "Find selected!"; 
        });
        editMenu->addItem("Replace", [this]() { 
            statusMessage = "Replace selected!"; 
        });
        
        // View menu
        auto viewMenu = std::make_shared<DropdownMenu>(18, 1, "View");
        viewMenu->addItem("Zoom In", [this]() { 
            statusMessage = "Zoom In selected!"; 
        });
        viewMenu->addItem("Zoom Out", [this]() { 
            statusMessage = "Zoom Out selected!"; 
        });
        viewMenu->addSeparator();
        viewMenu->addItem("Full Screen", [this]() { 
            statusMessage = "Full Screen selected!"; 
        });
        
        // Tools menu
        auto toolsMenu = std::make_shared<DropdownMenu>(26, 1, "Tools");
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
            for (auto& menu : menus) {
                menu->updateMouse(mouse, term_width, term_height);
            }
            
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
        
        // Draw status message
        std::string displayMessage = " " + statusMessage + " | Press Q to quit ";
        buffer->drawStringClipped(0, term_height - 1, displayMessage, 
                                Color::BLACK + Color::BG_BRIGHT_CYAN, term_width);
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