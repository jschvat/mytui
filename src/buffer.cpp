#include "../include/buffer.h"
#include <iostream>
#include <sstream>

UnicodeBuffer::UnicodeBuffer(int w, int h) : width(w), height(h) {
    cells.resize(height, std::vector<std::string>(width, " "));
    colors.resize(height, std::vector<std::string>(width, Color::RESET));
}

void UnicodeBuffer::clear() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            cells[y][x] = " ";
            colors[y][x] = Color::RESET;
        }
    }
}

void UnicodeBuffer::setCell(int x, int y, const std::string& ch, const std::string& color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        cells[y][x] = ch;
        colors[y][x] = color;
    }
}

void UnicodeBuffer::drawString(int x, int y, const std::string& text, const std::string& color) {
    for (size_t i = 0; i < text.length() && x + (int)i < width; i++) {
        setCell(x + (int)i, y, std::string(1, text[i]), color);
    }
}

void UnicodeBuffer::drawStringClipped(int x, int y, const std::string& text, const std::string& color, int maxX) {
    for (size_t i = 0; i < text.length() && x + (int)i < width && x + (int)i < maxX; i++) {
        setCell(x + (int)i, y, std::string(1, text[i]), color);
    }
}

void UnicodeBuffer::drawBox(int x, int y, int w, int h, const std::string& color, bool rounded, bool heavy) {
    std::string topLeft, topRight, bottomLeft, bottomRight, horizontal, vertical;
    
    if (heavy) {
        topLeft = Unicode::HEAVY_TOP_LEFT;
        topRight = Unicode::HEAVY_TOP_RIGHT;
        bottomLeft = Unicode::HEAVY_BOTTOM_LEFT;
        bottomRight = Unicode::HEAVY_BOTTOM_RIGHT;
        horizontal = Unicode::HEAVY_HORIZONTAL;
        vertical = Unicode::HEAVY_VERTICAL;
    } else if (rounded) {
        topLeft = Unicode::ROUND_TOP_LEFT;
        topRight = Unicode::ROUND_TOP_RIGHT;
        bottomLeft = Unicode::ROUND_BOTTOM_LEFT;
        bottomRight = Unicode::ROUND_BOTTOM_RIGHT;
        horizontal = Unicode::HORIZONTAL;
        vertical = Unicode::VERTICAL;
    } else {
        topLeft = Unicode::DOUBLE_TOP_LEFT;
        topRight = Unicode::DOUBLE_TOP_RIGHT;
        bottomLeft = Unicode::DOUBLE_BOTTOM_LEFT;
        bottomRight = Unicode::DOUBLE_BOTTOM_RIGHT;
        horizontal = Unicode::DOUBLE_HORIZONTAL;
        vertical = Unicode::DOUBLE_VERTICAL;
    }
    
    // Top border
    setCell(x, y, topLeft, color);
    for (int i = 1; i < w - 1; i++) {
        setCell(x + i, y, horizontal, color);
    }
    setCell(x + w - 1, y, topRight, color);
    
    // Side borders
    for (int i = 1; i < h - 1; i++) {
        setCell(x, y + i, vertical, color);
        setCell(x + w - 1, y + i, vertical, color);
    }
    
    // Bottom border
    setCell(x, y + h - 1, bottomLeft, color);
    for (int i = 1; i < w - 1; i++) {
        setCell(x + i, y + h - 1, horizontal, color);
    }
    setCell(x + w - 1, y + h - 1, bottomRight, color);
}

void UnicodeBuffer::render() {
    std::ostringstream output;
    output << "\033[H";
    
    std::string currentColor = "";
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (colors[y][x] != currentColor) {
                output << colors[y][x];
                currentColor = colors[y][x];
            }
            output << cells[y][x];
        }
        if (y < height - 1) output << "\r\n";
    }
    
    output << Color::RESET;
    std::cout << output.str() << std::flush;
}

