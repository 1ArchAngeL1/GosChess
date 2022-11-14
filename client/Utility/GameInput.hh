//
// Created by Leo Gogichaishvili on 18.01.22.
//

#ifndef GOSCHESS_GAMEINPUT_HH
#define GOSCHESS_GAMEINPUT_HH

#include <SFML/Window.hpp>


namespace GosChess {


    namespace InputHandle {


        constexpr int KEYBOARD_KEY_COUNT = sf::Keyboard::Key::KeyCount;
        constexpr int MOUSE_BUTTON_COUNT = sf::Mouse::Button::ButtonCount;

        extern bool keyboard_key_pressed[KEYBOARD_KEY_COUNT];
        extern bool keyboard_key_released[KEYBOARD_KEY_COUNT];
        extern bool keyboard_key_hold[KEYBOARD_KEY_COUNT];
        extern bool mouse_button_pressed[MOUSE_BUTTON_COUNT];
        extern bool mouse_button_released[MOUSE_BUTTON_COUNT];
        extern bool mouse_button_hold[MOUSE_BUTTON_COUNT];

        void Listen();

        bool KeyPressed(sf::Keyboard::Key _key);

        bool KeyReleased(sf::Keyboard::Key _key);

        bool KeyHold(sf::Keyboard::Key _key);

        bool MousePressed(sf::Mouse::Button _button);

        bool MouseReleased(sf::Mouse::Button _button);

        bool MouseHold(sf::Mouse::Button _button);


    }
}


#endif //PROJECT_NAME_GAMEINPUT_H
