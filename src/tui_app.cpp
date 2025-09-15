#include "../include/tui_app.h"
#include <iostream>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <algorithm>

TUIApplication::TUIApplication() : buffer(nullptr), frame(0) {
    setupTerminal();
    updateTerminalSize();
    buffer = new UnicodeBuffer(term_width, term_height);
    mouse.enableMouse();
}

TUIApplication::~TUIApplication() {
    delete buffer;
    restoreTerminal();
}

void TUIApplication::setupTerminal() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    
    if (tcgetattr(STDIN_FILENO, &orig_termios) != 0) {
        perror("tcgetattr");
        exit(1);
    }
    
    struct termios new_termios = orig_termios;
    new_termios.c_lflag &= ~(ECHO | ICANON);
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;
    
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios) != 0) {
        perror("tcsetattr");
        exit(1);
    }
    
    terminal_initialized = true;
    std::cout << "\033[2J\033[H\033[?25l" << std::flush;
}

void TUIApplication::restoreTerminal() {
    if (terminal_initialized) {
        std::cout << "\033[?1003l\033[?1006l\033[?1000l\033[?25h\033[2J\033[H\033[0m" << std::flush;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        terminal_initialized = false;
    }
}

void TUIApplication::updateTerminalSize() {
    struct winsize ws;
    term_width = 80;
    term_height = 24;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        term_width = ws.ws_col;
        term_height = ws.ws_row;
    }
}

void TUIApplication::drawBackground() {
    for (int y = 0; y < term_height; y++) {
        for (int x = 0; x < term_width; x++) {
            if ((x + y) % 20 == 0) {
                buffer->setCell(x, y, ".", Color::CYAN);
            }
        }
    }
}

void TUIApplication::drawStatusBar() {
    std::string statusBar = " UNICODE TUI v1.0 " + Unicode::BULLET + 
                           " DRAG: Title " + Unicode::BULLET + 
                           " RESIZE: # " + Unicode::BULLET + 
                           " CLOSE: [" + Unicode::FULL_BLOCK + "] " + Unicode::BULLET + 
                           " Q: Quit ";
    
    for (int i = 0; i < term_width && i < (int)statusBar.length(); i++) {
        if (i < statusBar.length()) {
            buffer->setCell(i, term_height - 1, std::string(1, statusBar[i]), 
                          Color::BLACK + Color::BG_BRIGHT_CYAN);
        }
    }
}

void TUIApplication::addWindow(std::shared_ptr<Window> window) {
    windows.push_back(window);
}

void TUIApplication::removeWindow(std::shared_ptr<Window> window) {
    windows.erase(std::remove(windows.begin(), windows.end(), window), windows.end());
}

void TUIApplication::run() {
    while (true) {
        mouse.updateMouse();
        
        // Update terminal size in case it changed
        updateTerminalSize();
        if (buffer) {
            delete buffer;
            buffer = new UnicodeBuffer(term_width, term_height);
        }
        
        buffer->clear();
        drawBackground();
        
        // Show mouse cursor
        buffer->setCell(mouse.getMouseX(), mouse.getMouseY(), Unicode::DIAMOND, Color::RED);
        
        // Reset window states
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
        
        drawStatusBar();
        buffer->render();
        
        frame++;
        usleep(16000); // ~60 FPS
    }
}

void TUIApplication::quit() {
    cleanup(0);
}