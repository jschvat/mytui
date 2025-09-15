#pragma once

#include "buffer.h"
#include "mouse_handler.h"
#include "window.h"
#include <vector>
#include <memory>
#include <sys/ioctl.h>

class TUIApplication {
private:
    FastMouseHandler mouse;
    UnicodeBuffer* buffer;
    std::vector<std::shared_ptr<Window>> windows;
    int term_width, term_height;
    int frame;
    
    void setupTerminal();
    void restoreTerminal();
    void updateTerminalSize();
    void drawBackground();
    void drawStatusBar();
    
public:
    TUIApplication();
    ~TUIApplication();
    
    void addWindow(std::shared_ptr<Window> window);
    void removeWindow(std::shared_ptr<Window> window);
    void run();
    void quit();
    
    int getTermWidth() const { return term_width; }
    int getTermHeight() const { return term_height; }
};