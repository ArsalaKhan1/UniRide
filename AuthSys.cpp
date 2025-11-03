#include "AuthSys.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <curl/curl.h>

AuthSystem::AuthSystem(std::shared_ptr<DatabaseManager> db) : dbManager(db) {}



bool AuthSystem::isRegistered(const std::string &email) const {
    std::lock_guard<std::mutex> lock(mtx);
    User user = dbManager->getUserByEmail(email);
    return !user.userID.empty();
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

crow::json::rvalue AuthSystem::verifyGoogleToken(const std::string& idToken) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://oauth2.googleapis.com/tokeninfo?id_token=" + idToken;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if(res == CURLE_OK && !readBuffer.empty()) {
            try {
                return crow::json::load(readBuffer);
            } catch(...) {
                return crow::json::load("{\"error\":\"Invalid token response\"}");
            }
        }
    }
    
    return crow::json::load("{\"error\":\"Network error\"}");
}

User AuthSystem::handleGoogleAuth(const std::string& idToken, const std::string& enrollmentId) {
    std::lock_guard<std::mutex> lock(mtx);
    
    crow::json::rvalue tokenInfo = verifyGoogleToken(idToken);
    
    if (tokenInfo.has("error")) {
        return User(); // Return empty user on error
    }
    
    std::string email = tokenInfo["email"].s();
    std::string name = tokenInfo["name"].s();
    std::string sub = tokenInfo["sub"].s();
    
    // Check if enrollment ID is valid
    if (!dbManager->isValidEnrollment(enrollmentId)) {
        return User(); // Return empty user if enrollment ID not found
    }
    
    User existingUser = dbManager->getUserByEmail(email);
    if (!existingUser.userID.empty()) {
        return existingUser;
    }
    
    User newUser(sub, name, email);
    if (dbManager->insertUser(newUser)) {
        return newUser;
    }
    
    return User();
}

std::string AuthSystem::generateSessionToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string AuthSystem::storeSessionToken(const std::string& userID) {
    std::lock_guard<std::mutex> lock(mtx);
    std::string token = generateSessionToken();
    sessionTokens[token] = userID;
    return token;
}

bool AuthSystem::validateSessionToken(const std::string& token, std::string& userID) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = sessionTokens.find(token);
    if (it != sessionTokens.end()) {
        userID = it->second;
        return true;
    }
    return false;
}

std::string AuthSystem::generateJWT(const User& user) {
    // Simplified JWT generation - in production use proper JWT library
    std::string header = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
    std::string payload = "{\"sub\":\"" + user.userID + "\",\"email\":\"" + user.email + "\"}";
    
    return header + "." + payload + ".signature";
}

bool AuthSystem::validateJWT(const std::string& token, std::string& userID) {
    // Simplified JWT validation - in production use proper JWT library
    size_t firstDot = token.find('.');
    size_t secondDot = token.find('.', firstDot + 1);
    
    if (firstDot == std::string::npos || secondDot == std::string::npos) {
        return false;
    }
    
    // Extract payload and decode (simplified)
    std::string payload = token.substr(firstDot + 1, secondDot - firstDot - 1);
    
    try {
        crow::json::rvalue payloadJson = crow::json::load(payload);
        userID = payloadJson["sub"].s();
        return true;
    } catch(...) {
        return false;
    }
}

crow::json::wvalue AuthSystem::toJson() const {
    crow::json::wvalue res;
    std::lock_guard<std::mutex> lock(mtx);
    
    std::vector<User> users = dbManager->getAllUsers();
    for (size_t i = 0; i < users.size(); ++i) {
        res["users"][i]["userID"] = users[i].userID;
        res["users"][i]["name"] = users[i].name;
        res["users"][i]["email"] = users[i].email;
    }
    
    return res;
}
