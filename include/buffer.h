#pragma once

#include "colors.h"
#include <vector>
#include <string>

// Unicode string utilities
class UnicodeUtils {
public:
    static int getDisplayWidth(const std::string& text);
    static std::vector<std::string> splitIntoChars(const std::string& text);
    static std::string substring(const std::string& text, int start, int length);
};

class UnicodeBuffer {
private:
    int width, height;
    std::vector<std::vector<std::string>> cells;
    std::vector<std::vector<std::string>> colors;
    
public:
    UnicodeBuffer(int w, int h);
    
    void clear();
    void setCell(int x, int y, const std::string& ch, const std::string& color);
    void drawString(int x, int y, const std::string& text, const std::string& color);
    void drawStringClipped(int x, int y, const std::string& text, const std::string& color, int maxX);
    void drawBox(int x, int y, int w, int h, const std::string& color, bool rounded = false, bool heavy = false);
    void fillRect(int x, int y, int w, int h, const std::string& character, const std::string& color);
    void render();
};