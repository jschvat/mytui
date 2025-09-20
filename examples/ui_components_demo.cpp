#include "../include/tui_app.h"
#include "../include/progress_bar.h"
#include "../include/checkbox.h"
#include "../include/text_input.h"
#include "../include/password_input.h"
#include "../include/radio_buttons.h"
#include "../include/status_bar.h"
#include "../include/list_box.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>

class UIComponentsDemo : public TUIApplication {
private:
    std::shared_ptr<Window> mainWindow;
    std::shared_ptr<Window> formWindow;
    std::shared_ptr<Window> listWindow;
    
    // UI Components
    std::shared_ptr<ProgressBar> progressBar;
    std::shared_ptr<ProgressBar> animatedProgress;
    std::shared_ptr<Checkbox> checkbox1;
    std::shared_ptr<Checkbox> checkbox2;
    std::shared_ptr<TextInput> textInput;
    std::shared_ptr<PasswordInput> passwordInput;
    std::shared_ptr<RadioButtons> radioButtons;
    std::shared_ptr<StatusBar> statusBar;
    std::shared_ptr<ListBox> listBox;
    
    int progressValue = 0;
    int animationFrame = 0;
    
public:
    UIComponentsDemo() {
        setupWindows();
        setupComponents();
        setupEventHandlers();
        updateStatusBar();
    }
    
    void setupWindows() {
        // Main window for basic components
        mainWindow = std::make_shared<Window>(5, 2, 45, 20, "UI Components Demo");
        mainWindow->visible = true;
        windows.push_back(mainWindow);
        
        // Form window for input components  
        formWindow = std::make_shared<Window>(55, 2, 35, 15, "Form Controls");
        formWindow->visible = true;
        windows.push_back(formWindow);
        
        // List window
        listWindow = std::make_shared<Window>(5, 25, 85, 12, "List & Status Demo");
        listWindow->visible = true;
        windows.push_back(listWindow);
    }
    
    void setupComponents() {
        // Progress Bars
        progressBar = std::make_shared<ProgressBar>(mainWindow, 2, 2, 35, 1); // Back to original size
        progressBar->setRange(0, 100);
        progressBar->setValue(25);
        progressBar->setColors(Color::GREEN + Color::BG_BLACK, Color::CYAN + Color::BG_BLACK);
        progressBar->setShowPercentage(true);
        
        animatedProgress = std::make_shared<ProgressBar>(mainWindow, 2, 4, 35, 1); // Back to original size
        animatedProgress->setRange(0, 100);
        animatedProgress->setValue(60);
        animatedProgress->setAnimated(true);
        animatedProgress->setCustomText("Loading...");
        animatedProgress->setColors(Color::BLUE + Color::BG_BLACK, Color::WHITE + Color::BG_BLACK);
        
        // Checkboxes
        checkbox1 = std::make_shared<Checkbox>(mainWindow, 2, 6, "Enable notifications", true);
        checkbox1->setColors(Color::GREEN + Color::BG_BLACK, Color::BRIGHT_WHITE + Color::BG_BLACK);
        
        checkbox2 = std::make_shared<Checkbox>(mainWindow, 2, 7, "Auto-save settings", false);
        checkbox2->setColors(Color::BLUE + Color::BG_BLACK, Color::BRIGHT_WHITE + Color::BG_BLACK);
        
        // Radio Buttons
        radioButtons = std::make_shared<RadioButtons>(mainWindow, 2, 9, false);
        radioButtons->addItem("Small", "sm");
        radioButtons->addItem("Medium", "md");
        radioButtons->addItem("Large", "lg");
        radioButtons->setSelectedIndex(1); // Default to Medium
        radioButtons->setColors(Color::YELLOW + Color::BG_BLACK, Color::BRIGHT_WHITE + Color::BG_BLACK);
        
        // Text Input
        textInput = std::make_shared<TextInput>(formWindow, 2, 2, 25, 1);
        textInput->setPlaceholder("Enter your name...");
        textInput->setColors(Color::WHITE + Color::BG_BLACK, Color::BLACK + Color::BG_WHITE);
        
        // Password Input
        passwordInput = std::make_shared<PasswordInput>(formWindow, 2, 5, 25, 1);
        passwordInput->setPlaceholder("Enter password...");
        passwordInput->setColors(Color::WHITE + Color::BG_BLACK, Color::BLACK + Color::BG_WHITE);
        
        // List Box
        listBox = std::make_shared<ListBox>(listWindow, 2, 2, 35, 7);
        listBox->addItem("Apple", "apple", Color::RED + Color::BG_BLACK);
        listBox->addItem("Banana", "banana", Color::YELLOW + Color::BG_BLACK);
        listBox->addItem("Cherry", "cherry", Color::BRIGHT_RED + Color::BG_BLACK);
        listBox->addSeparator();
        listBox->addItem("Orange", "orange", Color::BRIGHT_YELLOW + Color::BG_BLACK);
        listBox->addItem("Grape", "grape", Color::MAGENTA + Color::BG_BLACK);
        listBox->addItem("Blueberry", "blueberry", Color::BLUE + Color::BG_BLACK);
        listBox->addItem("Strawberry", "strawberry", Color::BRIGHT_RED + Color::BG_BLACK);
        listBox->addItem("Pineapple", "pineapple", Color::YELLOW + Color::BG_BLACK);
        listBox->addItem("Mango", "mango", Color::BRIGHT_YELLOW + Color::BG_BLACK);
        listBox->addItem("Kiwi", "kiwi", Color::GREEN + Color::BG_BLACK);
        listBox->setSelectedIndex(0);
        
        // Status Bar
        statusBar = std::make_shared<StatusBar>(listWindow, 0, 8, 81, 1);
        statusBar->addSegment("Ready", Color::GREEN + Color::BG_BLUE);
        statusBar->addSegment("Demo Mode", Color::YELLOW + Color::BG_BLUE);
        statusBar->addProgressSegment("Progress", 25, 20);
        statusBar->addTimeSegment("%H:%M:%S", true);
        statusBar->setColors(Color::WHITE + Color::BG_BLUE, Color::BRIGHT_WHITE + Color::BG_BLUE);
    }
    
    void setupEventHandlers() {
        // Progress Bar Events
        progressBar->setOnValueChange([this](const ProgressBarEvent& e) {
            updateStatusSegment(0, "Progress: " + std::to_string((int)e.percentage) + "%");
        });
        
        progressBar->setOnComplete([this](const ProgressBarEvent& e) {
            updateStatusSegment(0, "Progress Complete!");
        });
        
        // Checkbox Events
        checkbox1->setOnStateChange([this](const CheckboxEvent& e) {
            std::string status = e.newState ? "Notifications ON" : "Notifications OFF";
            updateStatusSegment(1, status);
        });
        
        checkbox2->setOnStateChange([this](const CheckboxEvent& e) {
            std::string status = e.newState ? "Auto-save ON" : "Auto-save OFF";
            updateStatusSegment(1, status);
        });
        
        // Radio Button Events
        radioButtons->setOnSelectionChange([this](const RadioButtonEvent& e) {
            updateStatusSegment(1, "Size: " + e.newValue);
        });
        
        // Text Input Events
        textInput->setOnTextChange([this](const TextInputEvent& e) {
            if (!e.newText.empty()) {
                updateStatusSegment(1, "Hello, " + e.newText + "!");
            } else {
                updateStatusSegment(1, "Enter your name");
            }
        });
        
        // Password Input Events
        passwordInput->setOnPasswordStrengthChange([this](const TextInputEvent& e) {
            updateStatusSegment(1, "Password: " + e.newText); // newText contains strength
        });
        
        // List Box Events
        listBox->setOnSelectionChange([this](const ListBoxEvent& e) {
            updateStatusSegment(1, "Selected: " + e.itemText);
        });
        
        listBox->setOnItemDoubleClick([this](const ListBoxEvent& e) {
            updateStatusSegment(0, "Double-clicked: " + e.itemText);
        });
    }
    
    void updateStatusSegment(int index, const std::string& text) {
        if (statusBar && index < statusBar->getSegmentCount()) {
            statusBar->setSegmentText(index, text);
        }
    }
    
    void updateStatusBar() {
        if (statusBar) {
            statusBar->updateTimeSegments();
            statusBar->updateSegment(2, "Progress " + std::to_string(progressValue) + "%");
        }
    }
    
    void animateComponents() {
        animationFrame++;
        
        // Animate progress bars
        if (animationFrame % 30 == 0) { // Every 30 frames (~0.5 seconds at 60fps)
            progressValue = (progressValue + 5) % 101;
            progressBar->setValue(progressValue);
            
            // Update animated progress bar
            animatedProgress->updateAnimation();
            int animValue = (int)(50 + 30 * sin(animationFrame * 0.1));
            animatedProgress->setValue(animValue);
        }
        
        // Update status bar time every second
        if (animationFrame % 60 == 0) {
            updateStatusBar();
        }
    }
    
    void drawInstructions() {
        // Draw instructions on the main window
        std::vector<std::string> instructions = {
            "Instructions:",
            "• Click checkboxes to toggle",
            "• Select radio buttons",  
            "• Click progress bar to set value",
            "• Type in text/password fields",
            "• Click items in list box",
            "• Double-click list items",
            "• Watch animated progress",
            "",
            "Press Q to quit"
        };
        
        int startY = 13;
        for (size_t i = 0; i < instructions.size(); i++) {
            if (startY + i < mainWindow->h - 2) {
                mainWindow->content.push_back(instructions[i]);
            }
        }
    }
    
    void run() override {
        while (true) {
            // Handle input
            mouse.updateMouse();
            
            // Check for quit
            char ch;
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (ch == 'q' || ch == 'Q') {
                    break;
                }
                
                // Handle keyboard input for text components
                if (textInput->isFocused()) {
                    textInput->handleKeyboard(ch, ch);
                } else if (passwordInput->isFocused()) {
                    passwordInput->handleKeyboard(ch, ch);
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
            
            // Update windows
            for (auto& window : windows) {
                window->active = false;
            }
            
            // Update windows for mouse interaction
            for (int i = windows.size() - 1; i >= 0; i--) {
                if (windows[i]->isVisible()) {
                    windows[i]->updateMouse(mouse, term_width, term_height);
                    if (windows[i]->dragging || windows[i]->resizing) {
                        windows[i]->active = true;
                        if (i != windows.size() - 1) {
                            auto temp = windows[i];
                            windows.erase(windows.begin() + i);
                            windows.push_back(temp);
                        }
                        break;
                    }
                }
            }
            
            // Clear window content before adding instructions
            mainWindow->content.clear();
            formWindow->content.clear();
            formWindow->content.push_back("Name:");
            formWindow->content.push_back("");
            formWindow->content.push_back("");
            formWindow->content.push_back("Password:");
            
            drawInstructions();
            
            // Draw windows and their components in z-order to prevent overlap issues
            for (auto& window : windows) {
                if (window->isVisible()) {
                    // Draw the window first
                    window->draw(*buffer);
                    
                    // Then immediately draw all components belonging to this window
                    if (window == mainWindow) {
                        // Draw mainWindow components
                        if (progressBar) {
                            progressBar->updateMouse(mouse, term_width, term_height);
                            progressBar->draw(*buffer);
                        }
                        
                        if (animatedProgress) {
                            animatedProgress->updateMouse(mouse, term_width, term_height);
                            animatedProgress->draw(*buffer);
                        }
                        
                        if (checkbox1) {
                            checkbox1->updateMouse(mouse, term_width, term_height);
                            checkbox1->draw(*buffer);
                        }
                        
                        if (checkbox2) {
                            checkbox2->updateMouse(mouse, term_width, term_height);
                            checkbox2->draw(*buffer);
                        }
                        
                        if (radioButtons) {
                            radioButtons->updateMouse(mouse, term_width, term_height);
                            radioButtons->draw(*buffer);
                        }
                    }
                    else if (window == formWindow) {
                        // Draw formWindow components
                        if (textInput) {
                            textInput->updateMouse(mouse, term_width, term_height);
                            textInput->draw(*buffer);
                        }
                        
                        if (passwordInput) {
                            passwordInput->updateMouse(mouse, term_width, term_height);
                            passwordInput->draw(*buffer);
                        }
                    }
                    else if (window == listWindow) {
                        // Draw listWindow components
                        if (listBox) {
                            listBox->updateMouse(mouse, term_width, term_height);
                            listBox->draw(*buffer);
                        }
                        
                        if (statusBar) {
                            statusBar->updateMouse(mouse, term_width, term_height);
                            statusBar->draw(*buffer);
                        }
                    }
                }
            }
            
            // Animate components
            animateComponents();
            
            // Draw mouse cursor
            drawMouseCursor();
            
            buffer->render();
            frame++;
            usleep(16000); // ~60 FPS
        }
    }
};

int main() {
    try {
        UIComponentsDemo demo;
        demo.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}