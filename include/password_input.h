#pragma once

#include "text_input.h"
#include <string>
#include <functional>
#include <memory>

// Password input component (inherits from TextInput)
class PasswordInput : public TextInput {
private:
    bool showPassword;
    bool capsLockWarning;
    std::string strengthIndicator;
    
    void calculatePasswordStrength();
    std::string getStrengthColor() const;
    
public:
    PasswordInput(std::shared_ptr<Window> parent, int x, int y, int width, int height = 1);
    ~PasswordInput() = default;
    
    // Password-specific functionality
    void setShowPassword(bool show);
    bool isPasswordVisible() const { return showPassword; }
    void togglePasswordVisibility();
    
    // Security features
    void setCapsLockWarning(bool enabled) { capsLockWarning = enabled; }
    bool isCapsLockWarningEnabled() const { return capsLockWarning; }
    
    // Password strength
    std::string getPasswordStrength() const { return strengthIndicator; }
    int getPasswordScore() const;
    
    // Override text display for password mode
    std::string getDisplayText() const;
    
    // Override draw to add password-specific visual elements
    void draw(UnicodeBuffer& buffer);
    
    // Override character input to add strength calculation
    bool insertCharacter(char ch);
    void deleteCharacter();
    void backspaceCharacter();
    void setText(const std::string& newText);
    
    // Additional password-specific callbacks
    std::function<void(const TextInputEvent&)> onPasswordStrengthChange;
    std::function<void(const TextInputEvent&)> onPasswordToggle;
    
    void setOnPasswordStrengthChange(std::function<void(const TextInputEvent&)> callback) { onPasswordStrengthChange = callback; }
    void setOnPasswordToggle(std::function<void(const TextInputEvent&)> callback) { onPasswordToggle = callback; }
};