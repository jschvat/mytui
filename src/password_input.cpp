#include "../include/password_input.h"
#include "../include/window.h"
#include "../include/component_clipping.h"
#include <algorithm>
#include <cctype>

// PasswordInput implementation
PasswordInput::PasswordInput(std::shared_ptr<Window> parent, int x, int y, int width, int height)
    : TextInput(parent, x, y, width, height), showPassword(false), capsLockWarning(true), strengthIndicator("Weak") {
    // Enable password mode by default
    setPasswordMode(true, '*');
    calculatePasswordStrength();
}

void PasswordInput::setShowPassword(bool show) {
    bool wasShowing = showPassword;
    showPassword = show;
    setPasswordMode(!show, '*');
    
    if (wasShowing != show && onPasswordToggle) {
        auto event = TextInputEvent(EventType::BUTTON_TOGGLE, 
                                   std::shared_ptr<TextInput>(this, [](TextInput*) {}), 
                                   "", getText());
        onPasswordToggle(event);
    }
}

void PasswordInput::togglePasswordVisibility() {
    setShowPassword(!showPassword);
}

std::string PasswordInput::getDisplayText() const {
    if (showPassword) {
        return getText();
    } else {
        // Use parent's password display logic
        return TextInput::getDisplayText();
    }
}

void PasswordInput::calculatePasswordStrength() {
    std::string password = getText();
    int score = 0;
    
    // Length scoring
    if (password.length() >= 8) score += 2;
    else if (password.length() >= 6) score += 1;
    
    // Character variety scoring
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    
    for (char c : password) {
        if (std::islower(c)) hasLower = true;
        else if (std::isupper(c)) hasUpper = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (std::ispunct(c)) hasSpecial = true;
    }
    
    if (hasLower) score++;
    if (hasUpper) score++;
    if (hasDigit) score++;
    if (hasSpecial) score++;
    
    // Additional length bonus
    if (password.length() >= 12) score++;
    if (password.length() >= 16) score++;
    
    // Set strength indicator
    if (score <= 2) strengthIndicator = "Weak";
    else if (score <= 4) strengthIndicator = "Fair";
    else if (score <= 6) strengthIndicator = "Good";
    else strengthIndicator = "Strong";
}

std::string PasswordInput::getStrengthColor() const {
    if (strengthIndicator == "Weak") return Color::RED + Color::BG_BLACK;
    else if (strengthIndicator == "Fair") return Color::YELLOW + Color::BG_BLACK;
    else if (strengthIndicator == "Good") return Color::BRIGHT_CYAN + Color::BG_BLACK;
    else return Color::GREEN + Color::BG_BLACK;
}

int PasswordInput::getPasswordScore() const {
    std::string password = getText();
    int score = 0;
    
    if (password.length() >= 8) score += 2;
    else if (password.length() >= 6) score += 1;
    
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (char c : password) {
        if (std::islower(c)) hasLower = true;
        else if (std::isupper(c)) hasUpper = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (std::ispunct(c)) hasSpecial = true;
    }
    
    if (hasLower) score++;
    if (hasUpper) score++;
    if (hasDigit) score++;
    if (hasSpecial) score++;
    
    if (password.length() >= 12) score++;
    if (password.length() >= 16) score++;
    
    return score;
}

void PasswordInput::draw(UnicodeBuffer& buffer) {
    if (!isVisible() || !parentWindow || !parentWindow->isVisible()) return;
    
    // Draw the base text input (this now includes clipping)
    TextInput::draw(buffer);
    
    if (!parentWindow) return;
    
    // Calculate clipping bounds for additional elements
    auto clipBounds = ComponentClipping::calculateClipBounds(parentWindow, getX(), getY(), getWidth(), getHeight());
    if (clipBounds.isEmpty) return;
    
    int absX = parentWindow->x + getX();
    int absY = parentWindow->y + getY();
    
    // Draw password strength indicator if there's space
    if (getHeight() > 1 || getWidth() > 20) {
        std::string strengthText = "(" + strengthIndicator + ")";
        std::string strengthColor = getStrengthColor();
        
        // Position strength indicator
        int strengthX = absX + getWidth() + 1;
        int strengthY = absY;
        
        // If not enough horizontal space, place below
        if (strengthX + (int)strengthText.length() > absX + getWidth() + 15) {
            strengthX = absX;
            strengthY = absY + 1;
            
            // Only draw if we have vertical space and it's within clip bounds
            if (getHeight() > 1 && ComponentClipping::shouldDraw(clipBounds, strengthX, strengthY)) {
                buffer.drawStringClipped(strengthX, strengthY, strengthText, strengthColor, clipBounds.endX);
            }
        } else {
            // Only draw if it's within clip bounds
            if (ComponentClipping::shouldDraw(clipBounds, strengthX, strengthY)) {
                buffer.drawStringClipped(strengthX, strengthY, strengthText, strengthColor, clipBounds.endX);
            }
        }
    }
    
    // Draw password visibility toggle hint
    if (isFocused() && getWidth() > 30) {
        std::string hint = showPassword ? "[Hide]" : "[Show]";
        int hintX = absX + getWidth() - (int)hint.length() - 1;
        int hintY = absY;
        
        // Only draw if it's within clip bounds
        if (ComponentClipping::shouldDraw(clipBounds, hintX, hintY)) {
            buffer.drawStringClipped(hintX, hintY, hint, Color::CYAN + Color::BG_BLACK, clipBounds.endX);
        }
    }
    
    // Draw caps lock warning if enabled and detected
    if (capsLockWarning && isFocused()) {
        // Simple caps lock detection based on recent input
        std::string text = getText();
        bool mightBeCapsLock = false;
        
        if (text.length() >= 3) {
            int upperCount = 0;
            for (size_t i = text.length() - 3; i < text.length(); i++) {
                if (std::isupper(text[i])) upperCount++;
            }
            mightBeCapsLock = (upperCount >= 2);
        }
        
        if (mightBeCapsLock) {
            std::string warning = "CAPS";
            int warningX = absX - 5;
            int warningY = absY;
            
            // Only draw if it's within clip bounds
            if (ComponentClipping::shouldDraw(clipBounds, warningX, warningY)) {
                buffer.drawStringClipped(warningX, warningY, warning, Color::BRIGHT_YELLOW + Color::BG_RED, clipBounds.endX);
            }
        }
    }
}

bool PasswordInput::insertCharacter(char ch) {
    std::string oldStrength = strengthIndicator;
    bool result = TextInput::insertCharacter(ch);
    
    if (result) {
        calculatePasswordStrength();
        
        if (oldStrength != strengthIndicator && onPasswordStrengthChange) {
            auto event = TextInputEvent(EventType::KEY_PRESS, 
                                       std::shared_ptr<TextInput>(this, [](TextInput*) {}), 
                                       oldStrength, strengthIndicator, ch);
            onPasswordStrengthChange(event);
        }
    }
    
    return result;
}

void PasswordInput::deleteCharacter() {
    std::string oldStrength = strengthIndicator;
    TextInput::deleteCharacter();
    calculatePasswordStrength();
    
    if (oldStrength != strengthIndicator && onPasswordStrengthChange) {
        auto event = TextInputEvent(EventType::KEY_PRESS, 
                                   std::shared_ptr<TextInput>(this, [](TextInput*) {}), 
                                   oldStrength, strengthIndicator);
        onPasswordStrengthChange(event);
    }
}

void PasswordInput::backspaceCharacter() {
    std::string oldStrength = strengthIndicator;
    TextInput::backspaceCharacter();
    calculatePasswordStrength();
    
    if (oldStrength != strengthIndicator && onPasswordStrengthChange) {
        auto event = TextInputEvent(EventType::KEY_PRESS, 
                                   std::shared_ptr<TextInput>(this, [](TextInput*) {}), 
                                   oldStrength, strengthIndicator);
        onPasswordStrengthChange(event);
    }
}

void PasswordInput::setText(const std::string& newText) {
    std::string oldStrength = strengthIndicator;
    TextInput::setText(newText);
    calculatePasswordStrength();
    
    if (oldStrength != strengthIndicator && onPasswordStrengthChange) {
        auto event = TextInputEvent(EventType::KEY_PRESS, 
                                   std::shared_ptr<TextInput>(this, [](TextInput*) {}), 
                                   oldStrength, strengthIndicator);
        onPasswordStrengthChange(event);
    }
}