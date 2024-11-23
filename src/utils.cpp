#include <iostream>
#include <sstream>
#include <iomanip>
#include "utils.h"

#include <chrono>
#include <time.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "json.hpp"

#ifdef _WIN32
#include <conio.h> 
#else
#include <termios.h>
#include <unistd.h>
#endif

using json = nlohmann::json;

void utils::printcmd(std::string const &str){
    fmt::print(fg(fmt::rgb(219, 186, 221)), str);
}

void utils::printcmd(std::string const &str, int r, int g, int b){
    fmt::print(fg(fmt::rgb(r, g, b)), str);
}

void utils::printerr(std::string const &str){
    fmt::print(fg(fmt::rgb(255, 83, 29)) | fmt::emphasis::bold, str);
}

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
    // Can be used if the 'grant_type' is "client_signature" to get the SHA256 encoded signature
    std::string string_to_code = std::to_string(timestamp) + "\n" + nonce + "\n" + data;
    return utils::hmac_sha256(clientsecret, string_to_code);
}

std::string utils::pretty(std::string j) {
    json serialised = json::parse(j);
    return serialised.dump(4);
}

std::string utils::printmap(std::map<std::string, std::string> mpp) {
    std::ostringstream os;

    for (const auto& pair : mpp) {
        os << pair.first << " : " << pair.second << '\n'; // Format: Key : Value
    }

    return os.str();
}

std::string utils::getPassword() {
    std::string password;
    char ch;

    std::cout << "Enter access key: ";

    #ifdef _WIN32
        while ((ch = _getch()) != '\r') { 
            if (ch == '\b') {  
                if (!password.empty()) {
                    password.pop_back();
                    std::cout << "\b \b";
                }
            } else {
                password += ch;
                std::cout << '*'; 
            }
        }
        std::cout << std::endl;

    #else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt); 
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        std::cin >> password;

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        std::cout << std::endl;
    #endif

    return password;
}