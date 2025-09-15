#pragma once

#include <string>

namespace Color {
    const std::string RESET = "\033[0m";
    const std::string CYAN = "\033[36m";
    const std::string BLUE = "\033[34m";
    const std::string WHITE = "\033[37m";
    const std::string BLACK = "\033[30m";
    const std::string GREEN = "\033[32m";
    const std::string RED = "\033[31m";
    const std::string YELLOW = "\033[33m";
    const std::string MAGENTA = "\033[35m";
    const std::string BRIGHT_CYAN = "\033[96m";
    const std::string BRIGHT_BLUE = "\033[94m";
    const std::string BRIGHT_WHITE = "\033[97m";
    const std::string BRIGHT_GREEN = "\033[92m";
    const std::string BRIGHT_RED = "\033[91m";
    const std::string BRIGHT_YELLOW = "\033[93m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string ORANGE = "\033[38;5;208m";  // 256-color orange
    const std::string BG_BLUE = "\033[44m";
    const std::string BG_WHITE = "\033[47m";
    const std::string BG_CYAN = "\033[46m";
    const std::string BG_GREEN = "\033[42m";
    const std::string BG_YELLOW = "\033[43m";
    const std::string BG_RED = "\033[41m";
    const std::string BG_MAGENTA = "\033[45m";
    const std::string BG_BLACK = "\033[40m";
    const std::string BG_BRIGHT_BLUE = "\033[104m";
    const std::string BG_BRIGHT_CYAN = "\033[106m";
    const std::string BG_BRIGHT_WHITE = "\033[107m";
}

namespace Unicode {
    // Single line box drawing
    const std::string TOP_LEFT = "┌";
    const std::string TOP_RIGHT = "┐";
    const std::string BOTTOM_LEFT = "└";
    const std::string BOTTOM_RIGHT = "┘";
    const std::string HORIZONTAL = "─";
    const std::string VERTICAL = "│";
    const std::string CROSS = "┼";
    const std::string T_DOWN = "┬";
    const std::string T_UP = "┴";
    const std::string T_RIGHT = "├";
    const std::string T_LEFT = "┤";
    
    // Double line box drawing
    const std::string DOUBLE_TOP_LEFT = "╔";
    const std::string DOUBLE_TOP_RIGHT = "╗";
    const std::string DOUBLE_BOTTOM_LEFT = "╚";
    const std::string DOUBLE_BOTTOM_RIGHT = "╝";
    const std::string DOUBLE_HORIZONTAL = "═";
    const std::string DOUBLE_VERTICAL = "║";
    
    // Heavy line box drawing
    const std::string HEAVY_TOP_LEFT = "┏";
    const std::string HEAVY_TOP_RIGHT = "┓";
    const std::string HEAVY_BOTTOM_LEFT = "┗";
    const std::string HEAVY_BOTTOM_RIGHT = "┛";
    const std::string HEAVY_HORIZONTAL = "━";
    const std::string HEAVY_VERTICAL = "┃";
    
    // Rounded corners
    const std::string ROUND_TOP_LEFT = "╭";
    const std::string ROUND_TOP_RIGHT = "╮";
    const std::string ROUND_BOTTOM_LEFT = "╰";
    const std::string ROUND_BOTTOM_RIGHT = "╯";
    
    // Shading and blocks
    const std::string LIGHT_SHADE = "░";
    const std::string MEDIUM_SHADE = "▒";
    const std::string DARK_SHADE = "▓";
    const std::string FULL_BLOCK = "█";
    
    // Symbols (using single-width characters)
    const std::string DIAMOND = "*";
    const std::string BULLET = "*";
    const std::string CIRCLE = "o";
    const std::string TRIANGLE_UP = "^";
    const std::string TRIANGLE_DOWN = "v";
    const std::string TRIANGLE_LEFT = "<";
    const std::string TRIANGLE_RIGHT = ">";
    const std::string CLOSE_X = "X";
    const std::string CHECK = "+";
    const std::string RESIZE_HANDLE = "#";
    const std::string MINIMIZE = "-";
    const std::string MAXIMIZE = "+";
    const std::string CLOSE_BUTTON = "X";
    
    // Mouse cursor styles
    const std::string CURSOR_DEFAULT = "+";
    const std::string CURSOR_POINTER = ">";
    const std::string CURSOR_HAND = "@";
    const std::string CURSOR_RESIZE = "#";
    const std::string CURSOR_MOVE = "*";
    const std::string CURSOR_TEXT = "|";
    const std::string CURSOR_CROSSHAIR = "+";
    
    // Arrows
    const std::string ARROW_UP = "^";
    const std::string ARROW_DOWN = "v";
    const std::string ARROW_LEFT = "<";
    const std::string ARROW_RIGHT = ">";
}