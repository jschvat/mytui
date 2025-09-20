#pragma once

#include "window.h"
#include <memory>

// Common clipping utilities for UI components
namespace ComponentClipping {
    
    struct ClipBounds {
        int startX, startY;
        int endX, endY;
        bool isEmpty;
        
        ClipBounds() : startX(0), startY(0), endX(0), endY(0), isEmpty(true) {}
        
        ClipBounds(int sx, int sy, int ex, int ey) 
            : startX(sx), startY(sy), endX(ex), endY(ey), isEmpty(sx >= ex || sy >= ey) {}
    };
    
    // Calculate clipping bounds for a component within its parent window
    inline ClipBounds calculateClipBounds(std::shared_ptr<Window> parentWindow, 
                                         int componentX, int componentY, 
                                         int componentWidth, int componentHeight) {
        if (!parentWindow) {
            return ClipBounds(); // Empty bounds
        }
        
        // Calculate component absolute position
        int absX = parentWindow->x + componentX;
        int absY = parentWindow->y + componentY;
        
        // Get window content boundaries (accounts for borders and scrollbars)
        int windowContentX = parentWindow->getContentX();
        int windowContentY = parentWindow->getContentY();
        int windowContentWidth = parentWindow->getContentWidth();
        int windowContentHeight = parentWindow->getContentHeight();
        
        // Calculate clipping bounds
        int clipStartX = std::max(absX, windowContentX);
        int clipStartY = std::max(absY, windowContentY);
        int clipEndX = std::min(absX + componentWidth, windowContentX + windowContentWidth);
        int clipEndY = std::min(absY + componentHeight, windowContentY + windowContentHeight);
        
        return ClipBounds(clipStartX, clipStartY, clipEndX, clipEndY);
    }
    
    // Check if a specific point should be drawn
    inline bool shouldDraw(const ClipBounds& bounds, int x, int y) {
        return !bounds.isEmpty && x >= bounds.startX && x < bounds.endX && 
               y >= bounds.startY && y < bounds.endY;
    }
}