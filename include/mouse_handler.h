#pragma once

#include <string>
#include <termios.h>
#include <unistd.h>

extern struct termios orig_termios;
extern bool terminal_initialized;

void cleanup(int sig);

class FastMouseHandler {
private:
    std::string inputBuffer;
    bool leftPressed = false;
    int currentX = 0, currentY = 0;
    
    void processAllAvailableInput();
    void parseMouseData(const std::string& data, bool isPress);
    
public:
    void enableMouse();
    void updateMouse();
    
    int getMouseX() const { return currentX; }
    int getMouseY() const { return currentY; }
    bool isLeftButtonPressed() const { return leftPressed; }
};