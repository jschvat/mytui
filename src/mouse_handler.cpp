#include "../include/mouse_handler.h"
#include <iostream>
#include <sstream>
#include <signal.h>

struct termios orig_termios;
bool terminal_initialized = false;

void cleanup(int sig) {
    if (terminal_initialized) {
        std::cout << "\033[?1003l\033[?1006l\033[?1000l\033[?25h\033[2J\033[H\033[0m" << std::flush;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }
    exit(0);
}

void FastMouseHandler::processAllAvailableInput() {
    char largeChunk[1024];
    ssize_t bytes = read(STDIN_FILENO, largeChunk, sizeof(largeChunk));
    
    if (bytes <= 0) return;
    
    for (ssize_t i = 0; i < bytes; i++) {
        char ch = largeChunk[i];
        
        if (ch == 'q' || ch == 'Q') {
            cleanup(0);
        }
        
        if (ch == '\033') {
            inputBuffer.clear();
            inputBuffer += ch;
        } else if (!inputBuffer.empty()) {
            inputBuffer += ch;
            
            if (inputBuffer.length() > 30) {
                inputBuffer.clear();
                continue;
            }
            
            if (inputBuffer.length() >= 6 && inputBuffer.substr(0, 3) == "\033[<") {
                size_t mPos = inputBuffer.find('M');
                size_t lowerMPos = inputBuffer.find('m');
                
                if (mPos != std::string::npos || lowerMPos != std::string::npos) {
                    size_t endPos = (mPos != std::string::npos) ? mPos : lowerMPos;
                    bool isPress = (inputBuffer[endPos] == 'M');
                    
                    std::string data = inputBuffer.substr(3, endPos - 3);
                    inputBuffer.clear();
                    
                    parseMouseData(data, isPress);
                }
            }
        }
    }
}

void FastMouseHandler::parseMouseData(const std::string& data, bool isPress) {
    std::istringstream ss(data);
    std::string buttonStr, xStr, yStr;
    
    if (std::getline(ss, buttonStr, ';') &&
        std::getline(ss, xStr, ';') &&
        std::getline(ss, yStr)) {
        
        try {
            int button = std::stoi(buttonStr);
            int x = std::stoi(xStr) - 1;
            int y = std::stoi(yStr) - 1;
            
            if (x >= 0 && x < 200 && y >= 0 && y < 100) {
                bool isLeftButton = (button & 3) == 0;
                
                if (isLeftButton) {
                    currentX = x;
                    currentY = y;
                    
                    if (isPress && !leftPressed) {
                        leftPressed = true;
                    } else if (!isPress && leftPressed) {
                        leftPressed = false;
                    }
                }
            }
        } catch (...) {
            // Ignore parsing errors
        }
    }
}

void FastMouseHandler::enableMouse() {
    std::cout << "\033[?1000h\033[?1006h\033[?1003h" << std::flush;
}

void FastMouseHandler::updateMouse() {
    processAllAvailableInput();
}