#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "Logger.h"


/**
 * @brief Tools
 * collection of helper functions used in GraphSTEP
**/


// typedefs
using Matrix = std::vector<std::vector<std::string>>;
// using AdjacentNodes = std::vector<std::pair<Node, std::string>>;
// using AdjacencyList = std::vector<std::pair<Node, AdjacentNodes>>;

typedef unsigned char BYTE;

struct Position {
    double x;
    double y;
    double z;
};

struct Quaternion {
    double w;
    double x;
    double y;
    double z;
};

inline std::string getCurrentDir() {
    std::filesystem::path full_path(__FILE__);
    return full_path.parent_path().string();
}

inline std::string getDataDir() {
    std::filesystem::path full_path(__FILE__);
    return full_path.parent_path().parent_path().parent_path().parent_path().parent_path().string() + "/data/";
}

class Stopwatch {
   public:
    Stopwatch() : m_startTime(std::chrono::high_resolution_clock::now()) {}
    ~Stopwatch() {}

    void restart() { m_startTime = std::chrono::high_resolution_clock::now(); }
    void stop() {
        m_endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            m_endTime - m_startTime);
        m_elapsedTime = duration.count() / 1000000.0;
        Logger::log("Time taken: " + std::to_string(m_elapsedTime) + " seconds");
    }
    double getElapsedTime() { return m_elapsedTime; }

   private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_endTime;
    double m_elapsedTime;
};

inline double degToRad(double degrees) { return degrees * 3.14159 / 180; }

inline std::string positionToString(Position position) {
    std::string positionString = "(";
    positionString += std::to_string(position.x) + ",";
    positionString += std::to_string(position.y) + ",";
    positionString += std::to_string(position.z) + ")";

    return positionString;
}

// compares two timestamps and returns true if newTime is really the new one
inline bool isNewer(time_t oldTime, time_t newTime) {
    if (difftime(oldTime, newTime) > 0) return true;

    return false;
}

inline std::string getCurrentTime() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d:%m:%Y_%H:%M:%S");
    return oss.str();
}

// converts a string separated by semicolons to a stringlist
inline std::vector<std::string> getListFromStrings(std::string str,
                                                   char separator = ';') {
    std::vector<std::string> list;
    std::istringstream f(str);
    std::string substring;
    while (getline(f, substring, separator)) {
        list.push_back(substring);
    }
    return list;
}

inline bool replace(std::string &str, const std::string &from,
                    const std::string &to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

inline void replaceAll(std::string &str, const std::string &from,
                       const std::string &to) {
    bool terminate = false;
    while (!terminate) terminate = !replace(str, from, to);
}

inline void filterString(std::string &str) {
    if (str.find("\\r") != std::string::npos) {
        std::string from = "\\r";
        std::string to = "";
        replaceAll(str, from, to);
    }

    if (str.find("\\n") != std::string::npos) {
        std::string from = "\\n";
        std::string to = "";
        replaceAll(str, from, to);
    }

    if (str.find("\\\\") != std::string::npos) {
        std::string from = "\\\\";
        std::string to = "\\";
        replaceAll(str, from, to);
    }
}

inline void writeFile(std::string path, std::string content) {
    std::ofstream matrix;
    matrix.open(path);
    matrix << content << std::endl;
}

inline bool isInteger(const std::string &str) {
    if (str.empty()) return false;

    for (auto c : str) {
        if (!std::isdigit(c)) {
            throw std::invalid_argument("string cannot be converted to an int");
            return false;
        }
    }
    return true;
}

inline bool isDouble(const std::string &str) {
    char *end = nullptr;
    double val = strtod(str.c_str(), &end);
    return end != str.c_str() && *end == '\0' && val != HUGE_VAL;
}

inline int getNumberFromString(std::string str) {
    std::string numberStr = str.substr(str.rfind('_') + 1, str.length());
    if (isInteger(numberStr)) {
        return std::stoi(numberStr);
    }

    return -1;
}

// compares the number at the end of each string
inline bool compareIntStrings(std::string s1, std::string s2) {
    if (getNumberFromString(s1) < getNumberFromString(s2)) return true;

    return false;
}

// converts a given string to a neo4j string
// "string" --> "'string'"
inline std::string makeString(const std::string str) {
    if (str[0] == '\'' || str[str.size()] == '\'') return str;

    return ("'" + str + "'");
}

inline std::string removeQuotation(std::string str) {
    //  if string contains "" or '' --> remove first and last character
    if (str.find("\"") != std::string::npos ||
        str.find("\"") != std::string::npos) {
        if (str.size() >= 2) str = str.substr(1, str.size() - 2);
    }

    if (str[0] == '\'' || str[str.size()] == '\'') {
        if (str.size() >= 2) str = str.substr(1, str.size() - 2);
    }

    return str;
}

inline std::string removeParentheses(std::string str) {
    if (str[0] == '(' || str[str.size()] == ')') {
        if (str.size() >= 2) str = str.substr(1, str.size() - 2);
    }

    return str;
}

inline Position stringToPosition(std::string positionString) {
    Position position;
    removeParentheses(positionString);
    auto list = getListFromStrings(positionString, ',');
    position.x = std::stod(list[0]);
    position.y = std::stod(list[1]);
    position.z = std::stod(list[2]);

    return position;
}

inline Quaternion stringToQuaternion(std::string quaternionString) {
    Quaternion quaternion;
    removeParentheses(quaternionString);
    auto list = getListFromStrings(quaternionString, ',');
    quaternion.w = std::stod(list[0]);
    quaternion.x = std::stod(list[1]);
    quaternion.y = std::stod(list[2]);
    quaternion.z = std::stod(list[3]);

    return quaternion;
}

// std::string "(0.,0.,0.)" --> std::vector<double> list
inline std::vector<double> strToVector(std::string list) {
    std::vector<double> ret;
    list = removeParentheses(list);
    std::vector<std::string> stringList = getListFromStrings(list, ',');

    for (auto &entry : stringList) {
        if (isDouble(entry)) ret.push_back(std::stod(entry));
    }
    return ret;
}

// convert matrix to csv string
inline std::string printMatrix(const Matrix &matrix) {
    std::string matrixString = "";
    for (auto &row : matrix) {
        for (auto entry = row.begin(); entry != row.end(); entry++) {
            matrixString += "\"" + *entry + "\"";

            if (entry != row.end()) {
                matrixString += ",";
            }
        }
        matrixString += "\n";
    }

    return matrixString;
}

//------BASE64 encode/decode ---------------//
static const std::string base64Chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

inline static bool isBase64(BYTE c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

inline std::string base64Encode(std::string data) {
    unsigned int bufLen = static_cast<unsigned int>(data.size());

    BYTE *buf = new BYTE[bufLen];

    for (unsigned int i = 0; i < bufLen; i++) buf[i] = data[i];

    std::string ret;
    int i = 0;
    int j = 0;
    BYTE charArray3[3];
    BYTE charArray4[4];

    while (bufLen--) {
        charArray3[i++] = *(buf++);
        if (i == 3) {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] =
                ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] =
                ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (i = 0; (i < 4); i++) ret += base64Chars[charArray4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) charArray3[j] = '\0';

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] =
            ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] =
            ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        charArray4[3] = charArray3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) ret += base64Chars[charArray4[j]];

        while ((i++ < 3)) ret += '=';
    }

    return ret;
}

inline std::vector<BYTE> base64Decode(std::string const &encodedString) {
    int in_len = encodedString.size();
    int i = 0;
    int j = 0;
    int in = 0;
    BYTE charArray4[4], charArray3[3];
    std::vector<BYTE> ret;

    while (in_len-- && (encodedString[in] != '=') &&
           isBase64(encodedString[in])) {
        charArray4[i++] = encodedString[in];
        in++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                charArray4[i] =
                    static_cast<BYTE>(base64Chars.find(charArray4[i]));

            charArray3[0] =
                (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
            charArray3[1] =
                ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
            charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

            for (i = 0; (i < 3); i++) ret.push_back(charArray3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) charArray4[j] = 0;

        for (j = 0; j < 4; j++)
            charArray4[j] = static_cast<BYTE>(base64Chars.find(charArray4[j]));

        charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
        charArray3[1] =
            ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
        charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(charArray3[j]);
    }

    return ret;
}

namespace uuid {
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> dis(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

inline std::string generateUuidV4() {
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis(gen);
    };
    return ss.str();
}
}  // namespace uuid