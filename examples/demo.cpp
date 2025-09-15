#include "../include/tui_app.h"
#include <memory>

int main() {
    TUIApplication app;
    
    // Create some demo windows
    auto window1 = std::make_shared<Window>(5, 2, 30, 8, "Demo Window 1");
    auto window2 = std::make_shared<Window>(25, 12, 35, 10, "Demo Window 2");
    auto window3 = std::make_shared<Window>(45, 6, 25, 7, "Demo Window 3");
    
    app.addWindow(window1);
    app.addWindow(window2);
    app.addWindow(window3);
    
    app.run();
    
    return 0;
}