#pragma once

#include <iostream>
#include <string>

class Password {

    private:
        static int num_sets;
        std::string access_token;
        
        Password() : access_token("") {}

    public:
        static Password &password();

        Password(const Password&) = delete;
        void operator=(const Password&) = delete;

        void setAccessToken(const std::string& token);
        void setAccessToken(int& token);

        std::string getAccessToken() const;
};
