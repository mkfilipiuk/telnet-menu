#ifndef SIK_USER_INTERFACE_H
#define SIK_USER_INTERFACE_H

#include <string>
#include <vector>
#include <algorithm>
#include <memory>

enum action_e {
    ARROW_UP,
    ARROW_DOWN,
    ENTER,
    NONE
};

class User_interface {
private:
    std::vector<std::shared_ptr<User_interface>> menu;
    std::shared_ptr<User_interface> parent;
    std::string text;
    std::string text_for_extra;
    size_t selected;

public:


    User_interface(std::string text, std::string text_for_extra, std::vector<std::shared_ptr<User_interface>> menu)
            : menu(menu), text(text), text_for_extra(text_for_extra) {
        selected = 0;
        parent = nullptr;
    }

    User_interface(std::string text, std::vector<std::shared_ptr<User_interface>> menu)
            : User_interface(text, "", menu) {}

    User_interface(std::string text, std::string text_for_extra) : text(text), text_for_extra(text_for_extra) {
        menu = std::vector<std::shared_ptr<User_interface>>();
    }

    User_interface(std::string text) : User_interface(text, "", std::vector<std::shared_ptr<User_interface>>()) {}

    std::shared_ptr<User_interface> act(action_e a_e, std::shared_ptr<User_interface> t, std::string& extra_text);

    void set_parent(std::shared_ptr<User_interface> parent);

    std::string give_message();
    
    void destroy();
};

#endif
