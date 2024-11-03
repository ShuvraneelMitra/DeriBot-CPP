#pragma once

#include <iostream>

namespace utils {

    long long time_now();

    std::string gen_random(const int len);
    
    std::string get_signature(long long timestamp, std::string nonce, std::string data, std::string clientsecret);

    std::string to_hex_string(const unsigned char* data, unsigned int length);

    std::string hmac_sha256(const std::string& key, const std::string& data);

    std::string pretty(std::string j);
}