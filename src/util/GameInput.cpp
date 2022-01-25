//
// Created by Leo Gogichaishvili on 18.01.22.
//

#include "GameInput.h"

bool GosChess::InputHandle::keyboard_key_pressed[KEYBOARD_KEY_COUNT];
bool GosChess::InputHandle::keyboard_key_released[KEYBOARD_KEY_COUNT];
bool GosChess::InputHandle::keyboard_key_hold[KEYBOARD_KEY_COUNT];
bool GosChess::InputHandle::mouse_button_pressed[MOUSE_BUTTON_COUNT];
bool GosChess::InputHandle::mouse_button_released[MOUSE_BUTTON_COUNT];
bool GosChess::InputHandle::mouse_button_hold[MOUSE_BUTTON_COUNT];


void GosChess::InputHandle::Listen() {
    for (int key = 0; key < GosChess::InputHandle::KEYBOARD_KEY_COUNT; key++) {
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key> (key))) {
            if (GosChess::InputHandle::keyboard_key_hold[key]) {
                GosChess::InputHandle::keyboard_key_pressed[key] = false;
            } else {
                GosChess::InputHandle::keyboard_key_pressed[key] = true;
                GosChess::InputHandle::keyboard_key_hold[key] = true;
            }
        } else {
            if ((GosChess::InputHandle::keyboard_key_hold[key])) {
                GosChess::InputHandle::keyboard_key_pressed[key] = false;
                GosChess::InputHandle::keyboard_key_hold[key] = false;
                GosChess::InputHandle::keyboard_key_released[key] = true;
            } else if (GosChess::InputHandle::keyboard_key_released[key]) {
                GosChess::InputHandle::keyboard_key_released[key] = false;
            }
        }

        for (int button = 0; button < GosChess::InputHandle::MOUSE_BUTTON_COUNT; button++) {
            if (sf::Mouse::isButtonPressed(static_cast <sf::Mouse::Button> (button))) {
                if (GosChess::InputHandle::mouse_button_hold[button]) {
                    GosChess::InputHandle::mouse_button_pressed[button] = false;
                } else {
                    GosChess::InputHandle::mouse_button_pressed[button] = true;
                    GosChess::InputHandle::mouse_button_hold[button] = true;
                }
            } else {
                if (GosChess::InputHandle::mouse_button_hold[button]) {
                    GosChess::InputHandle::mouse_button_pressed[button] = false;
                    GosChess::InputHandle::mouse_button_hold[button] = false;
                    GosChess::InputHandle::mouse_button_released[button] = true;
                } else if (GosChess::InputHandle::mouse_button_released[button]) {
                    GosChess::InputHandle::mouse_button_released[button] = false;
                }
            }
        }
    }
}


bool GosChess::InputHandle::KeyPressed(sf::Keyboard::Key _key) {
    return GosChess::InputHandle::keyboard_key_pressed[_key];
}

bool GosChess::InputHandle::KeyHold(sf::Keyboard::Key _key) {
    return GosChess::InputHandle::keyboard_key_hold[_key];
}

bool GosChess::InputHandle::KeyReleased(sf::Keyboard::Key _key) {
    return GosChess::InputHandle::keyboard_key_released[_key];
}

bool GosChess::InputHandle::MousePressed(sf::Mouse::Button _button) {
    return GosChess::InputHandle::mouse_button_pressed[_button];
}

bool GosChess::InputHandle::MouseHold(sf::Mouse::Button _button) {
    return GosChess::InputHandle::mouse_button_hold[_button];
}

bool GosChess::InputHandle::MouseReleased(sf::Mouse::Button _button) {
    return GosChess::InputHandle::mouse_button_released[_button];
}