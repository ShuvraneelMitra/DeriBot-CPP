#include <iostream>
#include <string>
#include "password.h"

int Password::num_sets = 0;  

Password &Password::password() {
    static Password pwd;
    return pwd;
}

void Password::setAccessToken(const std::string& token) {
    if(num_sets > 1) {
        std::cout << "WARNING: Access token can only be set once per session" << std::endl;
        return;
    }
    access_token = token;
    num_sets++;
}

void Password::setAccessToken(int& token) {
    if(num_sets > 1) {
        std::cout << "WARNING: Access token can only be set once per session" << std::endl;
        return;
    }
    access_token = std::to_string(token);
    num_sets++;
}

std::string Password::getAccessToken() const {
    return access_token;
}
