#include "../include/status_bar.h"
#include "../include/window.h"
#include "../include/buffer.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

// StatusBarEvent implementation
StatusBarEvent::StatusBarEvent(EventType type, std::shared_ptr<StatusBar> sb, int index, const std::string& text, const std::string& action)
    : Event(type), statusBar(sb), segmentIndex(index), segmentText(text), action(action) {
}

// StatusBar implementation
StatusBar::StatusBar(std::shared_ptr<Window> parent, int x, int y, int width, int height)
    : parentWindow(parent), x(x), y(y), width(width), height(height),
      visible(true), active(false),
      backgroundColor(Color::WHITE + Color::BG_BLUE), defaultTextColor(Color::BRIGHT_WHITE + Color::BG_BLUE),
      separatorChar("|"), separatorColor(Color::CYAN + Color::BG_BLUE),
      autoWidth(true), showSeparators(true),
      wasLeftPressed(false), hoveredSegment(-1) {
    calculateDimensions();
}

void StatusBar::addSegment(const std::string& text, const std::string& color, int width, bool rightAlign, bool clickable) {
    StatusBarSegment segment(text, color.empty() ? defaultTextColor : color, width, rightAlign, clickable);
    segments.push_back(segment);
    calculateDimensions();
}

void StatusBar::addSegment(const StatusBarSegment& segment) {
    segments.push_back(segment);
    calculateDimensions();
}

void StatusBar::setSegmentText(int index, const std::string& text) {
    if (index >= 0 && index < (int)segments.size()) {
        segments[index].text = text;
    }
}

void StatusBar::setSegmentColor(int index, const std::string& color) {
    if (index >= 0 && index < (int)segments.size()) {
        segments[index].color = color;
    }
}

void StatusBar::setSegmentClickable(int index, bool clickable, std::function<void()> callback) {
    if (index >= 0 && index < (int)segments.size()) {
        segments[index].clickable = clickable;
        segments[index].onClick = callback;
    }
}

void StatusBar::removeSegment(int index) {
    if (index >= 0 && index < (int)segments.size()) {
        segments.erase(segments.begin() + index);
        calculateDimensions();
    }
}

void StatusBar::clearSegments() {
    segments.clear();
    calculateDimensions();
}

void StatusBar::addTimeSegment(const std::string& format, bool rightAlign) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), format.c_str());
    
    addSegment(oss.str(), defaultTextColor, -1, rightAlign, false);
}

void StatusBar::addProgressSegment(const std::string& label, double percentage, int width) {
    std::ostringstream oss;
    oss << label << " " << std::fixed << std::setprecision(1) << percentage << "%";
    addSegment(oss.str(), defaultTextColor, width, false, false);
}

void StatusBar::addClickableSegment(const std::string& text, std::function<void()> callback, const std::string& color) {
    StatusBarSegment segment(text, color.empty() ? defaultTextColor : color, -1, false, true);
    segment.onClick = callback;
    segments.push_back(segment);
    calculateDimensions();
}

void StatusBar::setText(const std::string& text) {
    clearSegments();
    addSegment(text);
}

void StatusBar::updateSegment(int index, const std::string& text) {
    setSegmentText(index, text);
}

void StatusBar::updateTimeSegments() {
    for (int i = 0; i < (int)segments.size(); i++) {
        // Check if this looks like a time segment (contains colons)
        if (segments[i].text.find(':') != std::string::npos) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
            segments[i].text = oss.str();
        }
    }
}

void StatusBar::setShowSeparators(bool show, const std::string& separator, const std::string& color) {
    showSeparators = show;
    if (!separator.empty()) separatorChar = separator;
    if (!color.empty()) separatorColor = color;
}

void StatusBar::setColors(const std::string& background, const std::string& defaultText, const std::string& separator) {
    backgroundColor = background;
    defaultTextColor = defaultText;
    if (!separator.empty()) separatorColor = separator;
    
    // Update existing segments that use default color
    for (auto& segment : segments) {
        if (segment.color.empty()) {
            segment.color = defaultTextColor;
        }
    }
}

void StatusBar::calculateDimensions() {
    if (autoWidth && parentWindow) {
        width = parentWindow->getContentWidth();
    }
    
    // Ensure minimum dimensions
    if (width < 1) width = 1;
    if (height < 1) height = 1;
}

std::vector<int> StatusBar::calculateSegmentPositions() const {
    std::vector<int> positions;
    if (segments.empty()) return positions;
    
    // First pass: calculate available space and fixed width segments
    int availableWidth = width;
    int separatorSpace = showSeparators ? (int)(segments.size() - 1) : 0;
    availableWidth -= separatorSpace;
    
    std::vector<int> segmentWidths(segments.size());
    int fillSegments = 0;
    
    for (int i = 0; i < (int)segments.size(); i++) {
        if (segments[i].fixedWidth > 0) {
            segmentWidths[i] = segments[i].fixedWidth;
            availableWidth -= segmentWidths[i];
        } else if (segments[i].fixedWidth == 0) {
            fillSegments++;
        } else {
            // Auto width - use text length
            segmentWidths[i] = (int)segments[i].text.length();
            availableWidth -= segmentWidths[i];
        }
    }
    
    // Distribute remaining space among fill segments
    int fillWidth = fillSegments > 0 ? std::max(1, availableWidth / fillSegments) : 0;
    for (int i = 0; i < (int)segments.size(); i++) {
        if (segments[i].fixedWidth == 0) {
            segmentWidths[i] = fillWidth;
        }
    }
    
    // Second pass: calculate positions
    int currentPos = 0;
    for (int i = 0; i < (int)segments.size(); i++) {
        positions.push_back(currentPos);
        currentPos += segmentWidths[i];
        
        if (i < (int)segments.size() - 1 && showSeparators) {
            currentPos += 1; // Separator space
        }
    }
    
    return positions;
}

int StatusBar::getSegmentAtPosition(int mx, int my) const {
    if (!parentWindow) return -1;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    if (my < absY || my >= absY + height) return -1;
    
    std::vector<int> positions = calculateSegmentPositions();
    int relativeX = mx - absX;
    
    for (int i = 0; i < (int)segments.size(); i++) {
        if (i >= (int)positions.size()) break;
        
        int segmentStart = positions[i];
        int segmentEnd = (i < (int)positions.size() - 1) ? positions[i + 1] : width;
        
        if (showSeparators && i < (int)segments.size() - 1) {
            segmentEnd -= 1; // Account for separator
        }
        
        if (relativeX >= segmentStart && relativeX < segmentEnd) {
            return i;
        }
    }
    
    return -1;
}

bool StatusBar::contains(int mx, int my) const {
    if (!parentWindow) return false;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    return mx >= absX && mx < absX + width && my >= absY && my < absY + height;
}

void StatusBar::updateMouse(FastMouseHandler& mouse, int termWidth, int termHeight) {
    if (!visible || !parentWindow || !parentWindow->isVisible()) return;
    
    int mouseX = mouse.getMouseX();
    int mouseY = mouse.getMouseY();
    bool leftPressed = mouse.isLeftButtonPressed();
    
    bool isHovering = contains(mouseX, mouseY);
    bool wasHovering = active;
    active = isHovering;
    
    int currentHoverSegment = isHovering ? getSegmentAtPosition(mouseX, mouseY) : -1;
    int prevHoverSegment = hoveredSegment;
    hoveredSegment = currentHoverSegment;
    
    // Generate hover/leave events for status bar
    if (isHovering && !wasHovering && onHover) {
        auto event = MouseEvent(EventType::MOUSE_ENTER, mouseX, mouseY);
        onHover(event);
    } else if (!isHovering && wasHovering && onLeave) {
        auto event = MouseEvent(EventType::MOUSE_LEAVE, mouseX, mouseY);
        onLeave(event);
    }
    
    // Generate hover/leave events for segments
    if (currentHoverSegment != prevHoverSegment) {
        if (prevHoverSegment >= 0 && onSegmentLeave) {
            generateStatusEvent(EventType::MOUSE_LEAVE, prevHoverSegment, "leave");
        }
        if (currentHoverSegment >= 0 && onSegmentHover) {
            generateStatusEvent(EventType::MOUSE_ENTER, currentHoverSegment, "hover");
        }
    }
    
    // Handle click events
    if (isHovering && leftPressed && !wasLeftPressed) {
        int clickedSegment = getSegmentAtPosition(mouseX, mouseY);
        
        if (clickedSegment >= 0) {
            const auto& segment = segments[clickedSegment];
            
            // Execute segment's onClick callback if it has one
            if (segment.clickable && segment.onClick) {
                segment.onClick();
            }
            
            // Generate segment click event
            generateStatusEvent(EventType::MOUSE_PRESS, clickedSegment, "click");
        }
    }
    
    wasLeftPressed = leftPressed;
}

void StatusBar::draw(UnicodeBuffer& buffer) {
    if (!visible || !parentWindow || !parentWindow->isVisible()) return;
    
    int absX = parentWindow->x + x;
    int absY = parentWindow->y + y;
    
    // Fill background
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            buffer.setCell(absX + col, absY + row, " ", backgroundColor);
        }
    }
    
    if (segments.empty()) return;
    
    std::vector<int> positions = calculateSegmentPositions();
    
    // Draw segments
    for (int i = 0; i < (int)segments.size(); i++) {
        if (i >= (int)positions.size()) break;
        
        const auto& segment = segments[i];
        int segmentX = absX + positions[i];
        int segmentEnd = (i < (int)positions.size() - 1) ? positions[i + 1] : width;
        
        if (showSeparators && i < (int)segments.size() - 1) {
            segmentEnd -= 1; // Account for separator
        }
        
        int segmentWidth = segmentEnd - positions[i];
        
        // Determine text color (use segment color or default)
        std::string textColor = segment.color.empty() ? defaultTextColor : segment.color;
        
        // Highlight if hovered and clickable
        if (i == hoveredSegment && segment.clickable) {
            textColor = Color::BLACK + Color::BG_BRIGHT_WHITE;
        }
        
        // Draw segment text
        std::string displayText = segment.text;
        int displayWidth = UnicodeUtils::getDisplayWidth(displayText);
        if (displayWidth > segmentWidth) {
            displayText = UnicodeUtils::substring(displayText, 0, segmentWidth);
            displayWidth = UnicodeUtils::getDisplayWidth(displayText);
        }
        
        if (segment.rightAligned && displayWidth < segmentWidth) {
            int padding = segmentWidth - displayWidth;
            segmentX += padding;
        }
        
        buffer.drawStringClipped(segmentX, absY, displayText, textColor, absX + width);
        
        // Draw separator
        if (showSeparators && i < (int)segments.size() - 1) {
            int separatorX = absX + segmentEnd;
            if (separatorX < absX + width) {
                buffer.setCell(separatorX, absY, separatorChar, separatorColor);
            }
        }
    }
}

const StatusBarSegment& StatusBar::getSegment(int index) const {
    static StatusBarSegment empty("", "");
    if (index >= 0 && index < (int)segments.size()) {
        return segments[index];
    }
    return empty;
}

std::string StatusBar::getSegmentText(int index) const {
    if (index >= 0 && index < (int)segments.size()) {
        return segments[index].text;
    }
    return "";
}

void StatusBar::generateStatusEvent(EventType type, int segmentIndex, const std::string& action) {
    auto self = std::shared_ptr<StatusBar>(this, [](StatusBar*) {}); // Non-owning shared_ptr
    
    std::string segmentText = (segmentIndex >= 0 && segmentIndex < (int)segments.size()) ? 
                             segments[segmentIndex].text : "";
    
    auto event = std::unique_ptr<StatusBarEvent>(new StatusBarEvent(type, self, segmentIndex, segmentText, action));
    
    // Call local callbacks first
    switch (type) {
        case EventType::MOUSE_ENTER:
            if (onSegmentHover) onSegmentHover(*event);
            break;
        case EventType::MOUSE_LEAVE:
            if (onSegmentLeave) onSegmentLeave(*event);
            break;
        case EventType::MOUSE_PRESS:
            if (onSegmentClick) onSegmentClick(*event);
            break;
        default:
            break;
    }
    
    // Dispatch to global event manager
    EventManager::getInstance().dispatchEvent(std::move(event));
}