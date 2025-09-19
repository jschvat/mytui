#pragma once

#include "buffer.h"
#include "mouse_handler.h"
#include "colors.h"
#include "event_system.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

// Forward declaration
class Window;

// Status bar segment/section
struct StatusBarSegment {
    std::string text;
    std::string color;
    int fixedWidth;      // -1 = auto, 0 = fill remaining, >0 = fixed width
    bool rightAligned;
    bool clickable;
    std::function<void()> onClick;
    
    StatusBarSegment(const std::string& text, const std::string& color = "", int width = -1, bool rightAlign = false, bool clickable = false)
        : text(text), color(color), fixedWidth(width), rightAligned(rightAlign), clickable(clickable) {}
};

// Status bar events
class StatusBarEvent : public Event {
public:
    std::weak_ptr<class StatusBar> statusBar;
    int segmentIndex;
    std::string segmentText;
    std::string action;
    
    StatusBarEvent(EventType type, std::shared_ptr<class StatusBar> sb, int index, const std::string& text, const std::string& action);
};

// Status bar component (typically at bottom or top of window/screen)
class StatusBar {
private:
    std::shared_ptr<Window> parentWindow;
    int x, y;                    // Position relative to parent
    int width, height;           // Dimensions
    std::vector<StatusBarSegment> segments;
    bool visible;
    bool active;
    
    // Visual properties
    std::string backgroundColor;
    std::string defaultTextColor;
    std::string separatorChar;
    std::string separatorColor;
    
    // Layout
    bool autoWidth;              // Auto-size to parent width
    bool showSeparators;
    
    // Mouse interaction
    bool wasLeftPressed;
    int hoveredSegment;
    
    void generateStatusEvent(EventType type, int segmentIndex, const std::string& action);
    void calculateDimensions();
    int getSegmentAtPosition(int mx, int my) const;
    std::vector<int> calculateSegmentPositions() const;
    
public:
    StatusBar(std::shared_ptr<Window> parent, int x, int y, int width, int height = 1);
    ~StatusBar() = default;
    
    // Segment management
    void addSegment(const std::string& text, const std::string& color = "", int width = -1, bool rightAlign = false, bool clickable = false);
    void addSegment(const StatusBarSegment& segment);
    void setSegmentText(int index, const std::string& text);
    void setSegmentColor(int index, const std::string& color);
    void setSegmentClickable(int index, bool clickable, std::function<void()> callback = nullptr);
    void removeSegment(int index);
    void clearSegments();
    
    // Special segment types
    void addTimeSegment(const std::string& format = "%H:%M:%S", bool rightAlign = true);
    void addProgressSegment(const std::string& label, double percentage, int width = 20);
    void addClickableSegment(const std::string& text, std::function<void()> callback, const std::string& color = "");
    
    // Content updates
    void setText(const std::string& text); // Sets single segment
    void updateSegment(int index, const std::string& text);
    void updateTimeSegments(); // Update all time segments
    
    // Layout configuration
    void setAutoWidth(bool enabled) { autoWidth = enabled; if (enabled) calculateDimensions(); }
    bool isAutoWidth() const { return autoWidth; }
    void setShowSeparators(bool show, const std::string& separator = "|", const std::string& color = "");
    
    // Visual configuration
    void setColors(const std::string& background, const std::string& defaultText, const std::string& separator = "");
    void setBackgroundColor(const std::string& color) { backgroundColor = color; }
    void setDefaultTextColor(const std::string& color) { defaultTextColor = color; }
    
    // Interaction
    void updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight);
    bool contains(int mx, int my) const;
    
    // Rendering
    void draw(UnicodeBuffer& buffer);
    void show() { visible = true; }
    void hide() { visible = false; }
    bool isVisible() const { return visible; }
    
    // Position management
    void setPosition(int newX, int newY) { x = newX; y = newY; }
    void setSize(int newWidth, int newHeight) { width = newWidth; height = newHeight; calculateDimensions(); }
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Content access
    int getSegmentCount() const { return (int)segments.size(); }
    const StatusBarSegment& getSegment(int index) const;
    std::string getSegmentText(int index) const;
    
    // Event callbacks
    std::function<void(const StatusBarEvent&)> onSegmentClick;
    std::function<void(const StatusBarEvent&)> onSegmentHover;
    std::function<void(const StatusBarEvent&)> onSegmentLeave;
    std::function<void(const MouseEvent&)> onHover;
    std::function<void(const MouseEvent&)> onLeave;
    
    // Event callback setters
    void setOnSegmentClick(std::function<void(const StatusBarEvent&)> callback) { onSegmentClick = callback; }
    void setOnSegmentHover(std::function<void(const StatusBarEvent&)> callback) { onSegmentHover = callback; }
    void setOnSegmentLeave(std::function<void(const StatusBarEvent&)> callback) { onSegmentLeave = callback; }
    void setOnHover(std::function<void(const MouseEvent&)> callback) { onHover = callback; }
    void setOnLeave(std::function<void(const MouseEvent&)> callback) { onLeave = callback; }
};