#pragma once

#include "json.hpp"
#include <iostream>
#include <string>

using json = nlohmann::json;
extern bool AUTH_SENT;

class jsonrpc : public json {
    public:
        jsonrpc(){
            (*this)["jsonrpc"] = "2.0",

            srand( time(NULL) );
            int number = rand();
            (*this)["id"] = number;
        }
};

namespace deribit_api {

    std::string process(const std::string &input);

    std::string authorize(const std::string &cmd);

    std::string sell(const std::string &input);

    std::string buy(const std::string &input);
}