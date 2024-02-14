#pragma once

/**
 * @brief RestTools
 * used for sending queries to the neo4j database via the rest api
**/

enum class HttpState {
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    HTTP_NO_CONTENT = 204,
    HTTP_BAD_REQUEST = 400,
    HTTP_UNAUTHORIZED = 401,
    HTTP_NOT_FOUND = 404,
    HTTP_CONFLICT = 409,
    HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_SERVICE_UNAVAILABLE = 503,
    HTTP_UNDEFINED = 0
};

// conversion: HttpState to std::string
inline std::string httpStateToStr(const HttpState state) {
    switch (state) {
        case HttpState::HTTP_OK:
            return "OK";
        case HttpState::HTTP_CREATED:
            return "CREATED";
        case HttpState::HTTP_NO_CONTENT:
            return "NO CONTENT";
        case HttpState::HTTP_BAD_REQUEST:
            return "BAD REQUEST";
        case HttpState::HTTP_UNAUTHORIZED:
            return "UNAUTHORIZED";
        case HttpState::HTTP_NOT_FOUND:
            return "NOT FOUND";
        case HttpState::HTTP_CONFLICT:
            return "CONFLICT";
        case HttpState::HTTP_UNSUPPORTED_MEDIA_TYPE:
            return "UNSUPPORTED MEDIA TYPE";
        case HttpState::HTTP_SERVICE_UNAVAILABLE:
            return "SERVICE UNAVAILABLE";
        default:
            break;
    }
    return "UNDEFINED";
}

enum class AccessMode { WRITE, READ };

class RestInterface {
   public:
    RestInterface();
    RestInterface(std::string host, std::string username, std::string password);

    ~RestInterface();

    HttpState getRequest(std::string& result);
    HttpState postRequest(const std::string& jsonPayload, std::string& data);
    HttpState deleteRequest();

    void setAccessMode(AccessMode accessMode) { m_accessMode = accessMode; }

    void setCredentials(const std::string& username,
                        const std::string& password);
    void setHost(const std::string& host) { m_host = host; }
    void setPath(const std::string path) { m_path = path; }
    
    std::string getHost() { return m_host; }

   private:
    // converts the statuscode to a string
    HttpState intToHttpState(const int state);

    AccessMode m_accessMode;  // e.g. READ or WRITE
    std::string m_host;       // e.g. http://localhost:7474/
    std::string m_path;       // e.g. db/neo4j/tx/commit/
    std::string m_username;   // e.g. neo4j
    std::string m_password;
    std::string m_base64Credentials;
};