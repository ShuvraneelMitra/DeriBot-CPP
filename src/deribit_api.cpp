#include "deribit_api.h"
#include "utils.h"
#include "json.hpp"
#include "password.h"

#include <iostream>
#include <string>
#include <sstream>

using json = nlohmann::json;
bool AUTH_SENT = false;

std::string deribit_api::process(const std::string &input) {

    std::istringstream s(input.substr(8));
    std::string cmd;
    s >> cmd;
    
    if (cmd == "authorize") {
        return deribit_api::authorize(input.substr(8));
    }
    else if (cmd == "sell") {
        return deribit_api::sell(input.substr(8));
    }
    else if (cmd == "buy") {
        return deribit_api::buy(input.substr(8));
    }
    else{
        utils::printerr("ERROR: Unrecognized command. Please enter 'help' to see available commands.\n");
        return "";
    }
}

std::string deribit_api::authorize(const std::string &input) {

    std::istringstream s(input);
    std::string auth;
    std::string id;
    std::string flag{""};
    std::string client_id;
    std::string secret;
    long long tm = utils::time_now();

    s >> auth >> id >> client_id >> secret >> flag;
    std::string nonce = utils::gen_random(10);
    std::string signature = utils::get_signature(tm, nonce, "", secret);

    jsonrpc j;
    j["method"] = "public/auth";
    j["params"] = {{"grant_type", "client_signature"}, 
                   {"client_id", client_id},
                   {"timestamp", tm},
                   {"signature", signature},
                   {"nonce", nonce},
                   {"scope", "connection"}
                   };
    if (flag == "-r" && j.dump() != "") AUTH_SENT = true;
    return j.dump();
}

std::string deribit_api::sell(const std::string &input) {
    std::string sell;
    std::string id;
    std::string instrument;
    std::string cmd;
    std::string access_key;
    std::string order_type;
    std::string label;
    std::string frc;
    int contracts{0};
    int amount{0};
    int price{0};

    std::istringstream s(input);
    s >> sell >> id >> instrument >> label;

    if (Password::password().getAccessToken() == "") {
        utils::printcmd("Enter the access token: ");
        std::cin >> access_key;
    }
    else {
        access_key = Password::password().getAccessToken();
    }

    utils::printcmd("\nEnter the amount or contracts: ");
    std::cin >> cmd;
    if (cmd == "contracts") {
        std::cin >> contracts;
    }
    else if (cmd == "amount") {
        std::cin >> amount;
    }
    else {
        utils::printerr("\nIncorrect syntax; couldn't place order\n");
        return "";
    }

    utils::printcmd("Enter the order type: ");
    std::cin >> order_type;
    std::vector<std::string> permitted_order_types = {"limit", "stop_limit", "take_limit", "market", "stop_market", "take_market", "market_limit", "trailing_stop"};

    if (!std::any_of(permitted_order_types.begin(), permitted_order_types.end(), [&](std::string val){ return val == order_type; }))
    {
        utils::printerr("\nIncorrect syntax; couldn't place order\nOrder type can be only one of \nlimit\nstop_limit\ntake_limit\nmarket\nstop_market\ntake_market\nmarket_limit\ntrailing_stop\n");
        return "";
    }
    else if (order_type == "limit" || order_type == "stop_limit") {
        utils::printcmd("\nEnter the price at which you want to sell: ");
        std::cin >> price;
    }

    utils::printcmd("Enter the time-in-force value: ");
    std::cin >> frc;

    std::vector<std::string> permitted_tif = {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"};
    if (!std::any_of(permitted_tif.begin(), permitted_tif.end(), [&](std::string val){ return val == frc; }))
    {
        utils::printerr("\nIncorrect syntax; couldn't place order\nTime-in-force value can be only one of \ngood_til_cancelled\ngood_til_day\nfill_or_kill\nimmediate_or_cancel\n");
        return "";
    } 

    jsonrpc j;
    
    j["method"] = "private/sell";
    j["params"] = {{"instrument_name", instrument},
                   {"access_token", access_key}};
    if (amount) { 
        j["params"]["amount"] = amount;
    }
    else {
        j["params"]["contracts"] = contracts;
    }
    j["params"]["type"] = order_type;
    j["params"]["label"] = label;
    j["params"]["time_in_force"] = frc;

    return j.dump();
}

std::string deribit_api::buy(const std::string &input) {
    std::string buy;
    std::string id;
    std::string instrument;
    std::string cmd;
    std::string access_key;
    std::string order_type;
    std::string label;
    std::string frc;
    int contracts{0};
    int amount{0};
    int price{0};

    std::istringstream s(input);
    s >> buy >> id >> instrument >> label;

    if (Password::password().getAccessToken() == "") {
        utils::printcmd("Enter the access token: ");
        std::cin >> access_key;
    }
    else {
        access_key = Password::password().getAccessToken();
    }

    utils::printcmd("\nEnter the amount or contracts: ");
    std::cin >> cmd;
    if (cmd == "contracts") {
        std::cin >> contracts;
    }
    else if (cmd == "amount") {
        std::cin >> amount;
    }
    else {
        utils::printerr("\nIncorrect syntax; couldn't place order\n");
        return "";
    }

    utils::printcmd("Enter the order type: ");
    std::cin >> order_type;
    std::vector<std::string> permitted_order_types = {"limit", "stop_limit", "take_limit", "market", "stop_market", "take_market", "market_limit", "trailing_stop"};

    if (!std::any_of(permitted_order_types.begin(), permitted_order_types.end(), [&](std::string val){ return val == order_type; }))
    {
        utils::printerr("\nIncorrect syntax; couldn't place order\nOrder type can be only one of \nlimit\nstop_limit\ntake_limit\nmarket\nstop_market\ntake_market\nmarket_limit\ntrailing_stop\n");
        return "";
    }
    else if (order_type == "limit" || order_type == "stop_limit") {
        utils::printcmd("\nEnter the price at which you want to sell: ");
        std::cin >> price;
    }

    utils::printcmd("Enter the time-in-force value: ");
    std::cin >> frc;

    std::vector<std::string> permitted_tif = {"good_til_cancelled", "good_til_day", "fill_or_kill", "immediate_or_cancel"};
    if (!std::any_of(permitted_tif.begin(), permitted_tif.end(), [&](std::string val){ return val == frc; }))
    {
        utils::printerr("\nIncorrect syntax; couldn't place order\nTime-in-force value can be only one of \ngood_til_cancelled\ngood_til_day\nfill_or_kill\nimmediate_or_cancel\n");
        return "";
    } 

    jsonrpc j;
    
    j["method"] = "private/buy";
    j["params"] = {{"instrument_name", instrument},
                   {"access_token", access_key}};
    if (amount) { 
        j["params"]["amount"] = amount;
    }
    else {
        j["params"]["contracts"] = contracts;
    }
    j["params"]["type"] = order_type;
    j["params"]["label"] = label;
    j["params"]["time_in_force"] = frc;

    return j.dump();
}