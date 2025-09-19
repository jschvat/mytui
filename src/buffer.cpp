#include "../include/buffer.h"
#include "../include/asm_optimized.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// Unicode utility functions
int UnicodeUtils::getDisplayWidth(const std::string& text) {
    // Simple heuristic: count characters, not bytes
    // For UTF-8, count the number of character starts (not continuation bytes)
    int width = 0;
    for (size_t i = 0; i < text.length(); i++) {
        // UTF-8 continuation bytes have the pattern 10xxxxxx
        if ((text[i] & 0xC0) != 0x80) {
            width++;
        }
    }
    return width;
}

std::vector<std::string> UnicodeUtils::splitIntoChars(const std::string& text) {
    std::vector<std::string> chars;
    for (size_t i = 0; i < text.length(); ) {
        // Find the end of this UTF-8 character
        size_t charStart = i;
        i++;
        while (i < text.length() && (text[i] & 0xC0) == 0x80) {
            i++;
        }
        chars.push_back(text.substr(charStart, i - charStart));
    }
    return chars;
}

std::string UnicodeUtils::substring(const std::string& text, int start, int length) {
    auto chars = splitIntoChars(text);
    if (start >= (int)chars.size()) return "";
    
    std::string result;
    int end = std::min(start + length, (int)chars.size());
    for (int i = start; i < end; i++) {
        result += chars[i];
    }
    return result;
}

UnicodeBuffer::UnicodeBuffer(int w, int h) : width(w), height(h) {
    cells.resize(height, std::vector<std::string>(width, " "));
    colors.resize(height, std::vector<std::string>(width, Color::RESET));
}

void UnicodeBuffer::clear() {
    // Use ASM optimization for larger buffers
    if (ASMOptimized::has_avx2() && width * height > 200) {
        ASMOptimized::fast_buffer_clear_optimized(cells, colors, width, height);
    } else {
        // Standard implementation for small buffers
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                cells[y][x] = " ";
                colors[y][x] = Color::RESET;
            }
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
    auto chars = UnicodeUtils::splitIntoChars(text);
    for (size_t i = 0; i < chars.size() && x + (int)i < width; i++) {
        setCell(x + (int)i, y, chars[i], color);
    }
}

void UnicodeBuffer::drawStringClipped(int x, int y, const std::string& text, const std::string& color, int maxX) {
    auto chars = UnicodeUtils::splitIntoChars(text);
    for (size_t i = 0; i < chars.size() && x + (int)i < width && x + (int)i < maxX; i++) {
        setCell(x + (int)i, y, chars[i], color);
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

void UnicodeBuffer::fillRect(int x, int y, int w, int h, const std::string& character, const std::string& color) {
    // Use ASM optimization for larger rectangles
    if (ASMOptimized::has_avx2() && w * h > 32) {
        ASMOptimized::fast_rect_fill(cells, colors, x, y, w, h, character, color);
    } else {
        // Standard implementation for small rectangles
        for (int row = y; row < y + h; row++) {
            for (int col = x; col < x + w; col++) {
                if (row >= 0 && row < height && col >= 0 && col < width) {
                    cells[row][col] = character;
                    colors[row][col] = color;
                }
            }
        }
    }
}

void UnicodeBuffer::render() {
    std::ostringstream output;
    output << "\033[H";
    
    std::string currentColor = "";
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Use SIMD-optimized string comparison for color changes
            if (ASMOptimized::has_sse2() && !ASMOptimized::fast_string_equal(colors[y][x], currentColor)) {
                output << colors[y][x];
                currentColor = colors[y][x];
            } else if (!ASMOptimized::has_sse2() && colors[y][x] != currentColor) {
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

