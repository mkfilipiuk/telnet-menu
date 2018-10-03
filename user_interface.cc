#include "user_interface.h"

#include <sstream>
#include <utility>

std::string User_interface::give_message() {
    std::stringstream ss;
    ss << "\u001B[";
    ss << "\033[2J";
    ss << "\033[H";
    for (size_t i = 0; i < menu.size(); ++i) {
        if (i == selected) ss << "\e[7m";
        ss << menu[i].get()->text << "\n\r";
        if (i == selected) ss << "\e[27m";
    }
    //if (!extra_text.empty()) ss << extra_text << "\n\r";
    //extra_text.clear();
    return ss.str();
}


void User_interface::set_parent(std::shared_ptr<User_interface> parent) {
    this->parent = parent;
}

std::shared_ptr<User_interface> User_interface::act(action_e a_e, 
						    std::shared_ptr<User_interface> t,
						    std::string& extra_text) {
    if (a_e == NONE) return t;
    if (a_e == ARROW_DOWN) {
        selected++;
        selected = std::min(selected, menu.size() - 1);
        return t;
    }
    if (a_e == ARROW_UP) {
        if (selected > 0)selected--;
        return t;
    }
    if (a_e == ENTER) {
        if (menu[selected]->text.compare("Wstecz") == 0) {
            int s = selected;
            selected = 0;
            return menu[s]->parent;
        }
        if (menu[selected]->text.compare("Koniec") == 0) return nullptr;
        if (menu[selected]->menu.size() == 0) {
            extra_text = menu[selected]->text_for_extra;
            return t;
        }
        return menu[selected];
    }
    return t;
}

void User_interface::destroy()
{
    auto p = parent;
    parent = nullptr;
    if(p.get() != nullptr)p->destroy();
    p = nullptr;
    auto m = menu;
    menu.clear();
    for(auto p : menu)p->destroy();
    m.clear();
}

