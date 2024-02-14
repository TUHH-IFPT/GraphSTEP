#include <iostream>
#include <cpr/cpr.h>
#include "RestTools.h"
#include "Tools.hpp"

std::string contentType = "application/json";

RestInterface::RestInterface() {}
RestInterface::RestInterface(std::string host, std::string username,
                             std::string password)
    : m_host(host) {
    setCredentials(username, password);
    setAccessMode(AccessMode::WRITE);
}

RestInterface::~RestInterface() {}

void RestInterface::setCredentials(const std::string& username,
                                   const std::string& password) {
    m_username = username;
    m_password = password;

    std::string credentials = m_username + ":" + m_password;

    m_base64Credentials = base64Encode(credentials);
}

HttpState RestInterface::intToHttpState(const int state) {
    switch (state) {
        case 200:
            return HttpState::HTTP_OK;
        case 204:
            return HttpState::HTTP_NO_CONTENT;
        case 400:
            return HttpState::HTTP_BAD_REQUEST;
        case 401:
            return HttpState::HTTP_UNAUTHORIZED;
        case 404:
            return HttpState::HTTP_NOT_FOUND;
        case 409:
            return HttpState::HTTP_CONFLICT;
        case 415:
            return HttpState::HTTP_UNSUPPORTED_MEDIA_TYPE;
        case 503:
            return HttpState::HTTP_SERVICE_UNAVAILABLE;
        default:
            return HttpState::HTTP_UNDEFINED;
    }
}

HttpState RestInterface::getRequest(std::string& result) {
    cpr::Response response;
    if (!m_base64Credentials.empty()) {
        if (!contentType.empty()) {
            response = cpr::Get(
                cpr::Url{(m_host + m_path)},
                cpr::Header{{"Authorization", "Basic " + m_base64Credentials},
                            {"Content-Type", (contentType)}});
        } else {
            response = cpr::Get(
                cpr::Url{(m_host + m_path)},
                cpr::Header{{"Authorization", "Basic " + m_base64Credentials}});
        }
    } else {
        if (!contentType.empty()) {
            response = cpr::Get(cpr::Url{(m_host + m_path)},
                                cpr::Header{{"Content-Type", contentType}});
        } else {
            response = cpr::Get(cpr::Url{(m_host + m_path)});
        }
    }
    result = response.text;
    return intToHttpState(response.status_code);
}

HttpState RestInterface::postRequest(const std::string& jsonPayload,
                                     std::string& data) {
    cpr::Response response;

    if (!m_base64Credentials.empty()) {
        response = cpr::Post(
            cpr::Url{(m_host + m_path)},
            cpr::Header{{"Authorization", "Basic " + m_base64Credentials},
                        {"Content-Type", contentType},
                        {"Accept", contentType + ";charset=UTF-8"},
                        {"Access-Mode", "WRITE"}},
            cpr::Body{jsonPayload});

    } else {
        response =
            cpr::Post(cpr::Url{m_host + (m_path)}, cpr::Body{jsonPayload});
    }

    data = response.text;
    return intToHttpState(response.status_code);
}

HttpState RestInterface::deleteRequest() {
    cpr::Response response;
    if (!m_base64Credentials.empty()) {
        response = cpr::Delete(
            cpr::Url{(m_host + m_path)},
            cpr::Header{{"Authorization", "Basic " + m_base64Credentials}});
    } else {
        response = cpr::Delete(cpr::Url{m_host + (m_path)});
    }
    return intToHttpState(response.status_code);
}