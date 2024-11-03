#include "deribit_api.h"
#include "utils.h"
#include "json.hpp"

#include <iostream>
#include <string>
#include <sstream>

using json = nlohmann::json;

std::string deribit_api::process(const std::string &input) {

    std::istringstream s(input.substr(8));
    std::string cmd;
    s >> cmd;
    
    if (cmd == "authorize") {
        return authorize(cmd);
    }
    else{
        return jsonrpc();
    }
}

std::string deribit_api::authorize(const std::string &cmd) {

    std::istringstream s(cmd);
    std::string auth;
    std::string id;
    std::string client_id;
    std::string secret;
    long long tm = utils::time_now();

    s >> auth >> id >> client_id >> secret;

    std::string nonce = utils::gen_random(10);
    std::string signature = utils::get_signature(tm, nonce, "", secret);

    jsonrpc j;
    j["method"] = "public/auth";
    j["params"] = {{"grant_type", "client_signature"}, 
                   {"client_id", client_id},
                   {"timestamp", tm},
                   {"signature", signature},
                   {"nonce", nonce}
                   };
    return j.dump();
}
