#include <iostream>
#include <sstream>
#include <iomanip>
#include "utils.h"

#include <chrono>
#include <time.h>
#include <unistd.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "json.hpp"

using json = nlohmann::json;

long long utils::time_now(){
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    long long unix_timestamp_ms = epoch.count();

    return unix_timestamp_ms;
}

std::string utils::gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

std::string utils::to_hex_string(const unsigned char* data, unsigned int length) {
    std::ostringstream hex_stream;
    hex_stream << std::hex << std::uppercase << std::setfill('0');
    for (unsigned int i = 0; i < length; ++i) {
        hex_stream << std::setw(2) << static_cast<int>(data[i]);
    }
    return hex_stream.str();
}

std::string utils::hmac_sha256(const std::string& key, const std::string& data) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_length = 0;

    HMAC(EVP_sha256(), key.c_str(), key.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         result, &result_length);

    return utils::to_hex_string(result, result_length);
}

std::string utils::get_signature(long long timestamp, std::string nonce, std::string data, std::string clientsecret){

    std::string string_to_code = std::to_string(timestamp) + "\n" + nonce + "\n" + data;
    return utils::hmac_sha256(clientsecret, string_to_code);
}

std::string utils::pretty(std::string j) {
    json serialised = json::parse(j);
    return serialised.dump(4);
}