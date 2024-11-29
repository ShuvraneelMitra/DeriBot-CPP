#include "deribit_api.h"
#include "utils.h"
#include "json.hpp"
#include "password.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using json = nlohmann::json;
bool AUTH_SENT = false;
std::vector<std::string> SUPPORTED_CURRENCIES = {"BTC", "ETH", "SOL", "XRP", "MATIC",
                                                "USDC", "USDT", "JPY", "CAD", "AUD", "GBP", 
                                                "EUR", "USD", "CHF", "BRL", "MXN", "COP", 
                                                "CLP", "PEN", "ECS", "ARS"};

std::string deribit_api::process(const std::string &input) {

    std::map<std::string, std::function<std::string(std::string)>> action_map = 
    {
        {"authorize", deribit_api::authorize},
        {"sell", deribit_api::sell},
        {"buy", deribit_api::buy},
        {"get_open_orders", deribit_api::get_open_orders},
        {"modify", deribit_api::modify},
        {"cancel", deribit_api::cancel}
    };

    std::istringstream s(input.substr(8));
    int id;
    std::string cmd;
    s >> id >> cmd;

    auto find = action_map.find(cmd);
    if (find == action_map.end()) {
        utils::printerr("ERROR: Unrecognized command. Please enter 'help' to see available commands.\n");
        return "";
    }
    return find->second(input.substr(8));
}

std::string deribit_api::authorize(const std::string &input) {

    std::istringstream s(input);
    std::string auth;
    std::string id;
    std::string flag{""};
    std::string client_id;
    std::string secret;
    long long tm = utils::time_now();

    s >> id >> auth >> client_id >> secret >> flag;
    std::string nonce = utils::gen_random(10);

    jsonrpc j;
    j["method"] = "public/auth";
    j["params"] = {{"grant_type", "client_credentials"}, 
    /* 
    NOTE: Changing 'grant_type' to client_signature may help in improving security,
    but will need to use the already provided function utils::get_signature()
    which might invite complexity and errors.
    */
                   {"client_id", client_id},
                   {"client_secret", secret},
                   {"timestamp", tm},
                   {"nonce", nonce},
                   {"scope", "session:name"}
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
    s >> id >> sell >> instrument >> label;

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

    jsonrpc j("private/sell");
    
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
    s >> id >> buy >> instrument >> label;

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

    jsonrpc j("private/buy");

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

std::string deribit_api::get_open_orders(const std::string &input) {
    std::istringstream is(input);

    int id;
    std::string cmd;
    std::string opt1;
    std::string opt2;
    is >> id >> cmd >> opt1 >> opt2;

    jsonrpc j;

    if (opt1 == "") {
        j["method"] = "private/get_open_orders";
    }
    else if (std::find(SUPPORTED_CURRENCIES.begin(), SUPPORTED_CURRENCIES.end(), opt1) == SUPPORTED_CURRENCIES.end()) {
        j["method"] = "private/get_open_orders_by_instrument";
        j["params"] = {{"instrument", opt1}};
    }
    else if ( opt2 == "" ) {
        j["method"] = "private/get_open_orders_by_currency";
        j["params"] = {{"currency", opt1}};
    }
    else {
        j["method"] = "private/get_open_orders_by_label";
        j["params"] = {{"currency", opt1},
                        {"label", opt2}};
    }
    return j.dump();
}

std::string deribit_api::modify(const std::string &input) {
    std::istringstream is;

    int id;
    std::string cmd;
    std::string ord_id;
    is >> id >> cmd >> ord_id;

    jsonrpc j;
    j["method"] = "private/edit";

    int amount;
    int price;

    std::cout << "Enter -1 to keep the below parameters the same\n";
    utils::printcmd("Enter the new price of the instrument at which you want to trade: ");
    std::cin >> price;
    utils::printcmd("Enter the new amount you'd like to trade: ");
    std::cin >> amount;

    j["params"] = {{"order_id", ord_id}};
    if (amount >= 0) j["params"]["amount"] = amount;
    if (price >= 0) j["params"]["price"] = price;

    return j.dump();
}

std::string deribit_api::cancel(const std::string &input) {
    std::istringstream iss;
    int id;
    std::string cmd;
    std::string ord_id;

    iss >> id >> cmd >> ord_id;
    if (ord_id == "") { 
        utils::printerr("Order ID cannot be blank. If you want to cancel all orders, use cancel_all instead.");
    }

    jsonrpc j("private/cancel");

    j["params"]["order_id"] = ord_id;
    return j.dump();
}

std::string deribit_api::cancel_all(const std::string &input) {
    std::istringstream iss;
    int id;
    std::string cmd;
    std::string option;
    std::string label;

    jsonrpc j;
    j["params"] = {};

    iss >> id >> cmd >> option >> label;
    if (option == "") { 
        j["method"] = "private/cancel_all";
    }
    else if (std::find(SUPPORTED_CURRENCIES.begin(), SUPPORTED_CURRENCIES.end(), option) == SUPPORTED_CURRENCIES.end()) {
        j["method"] = "private/cancel_all_by_instrument";
        j["params"]["instrument"] = option;
    }
    else if (option == "-r") {
        j["method"] = "private/cancel_by_label";
        j["params"]["label"] = label;
    }
    else { 
        j["method"] = "private/cancel_all_by_currency";
        j["params"]["currency"] = option;
    }
    
    return j.dump();
}