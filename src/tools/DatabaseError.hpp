#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

/**
 * @brief DatabaseError
 * error handling and logging module 
**/

class DatabaseError : public std::runtime_error {
    std::string msg;

   public:
    DatabaseError(const std::string &arg, const char *file, int line)
        : std::runtime_error(arg) {
        std::ostringstream o;
        o << file << ":" << line << ": " << arg;
        msg = o.str();
    }
    ~DatabaseError() throw() {}
    const char *what() const throw() { return msg.c_str(); }
};
#define throw_database_error(arg) throw DatabaseError(arg, __FILE__, __LINE__);

inline std::string extractErrorMessage(const std::string &queries) {
    // "errors": [
    // {
    //     "code": "Neo.ClientError.Request.InvalidFormat",
    //     "message": "Unable to deserialize request. Expected [START_OBJECT,
    //     FIELD_NAME, START_ARRAY], found [START_OBJECT, END_OBJECT, null]."
    // }

    // Extracted error message
    std::string message = "";

    int error_counter = 0;
    if (!queries.empty()) {
        // Parse json
        json jsonString = json::parse(queries);
        json errors = jsonString["errors"];

        for (auto &error : errors) {
            message += "error[" + std::to_string(error_counter) + "] \n";
            message += "	code: " + error["code"].dump() + "\n";
            message += "	message: " + error["message"].dump() + "\n\n";
            error_counter++;
        }
    }

    if (error_counter > 1)
        message = std::to_string(error_counter) + " error found: \n" + message;
    else
        message = std::to_string(error_counter) + " errors found: \n" + message;

    return message;
}

inline bool checkResponseForErrors(const std::string &repsonse) {
    if (repsonse.find("errors\":[]") == std::string::npos) {
        // Extract the error message and throw the exception ...
        std::string error = extractErrorMessage(repsonse);
        throw_database_error(error);
        return false;
    }
    return true;
}