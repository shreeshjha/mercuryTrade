// src/services/UserService.cpp
#include "../../include/mercuryTrade/services/UserService.hpp"
#include <unordered_map>

namespace mercuryTrade {

namespace {
    // Mock user database
    std::unordered_map<std::string, std::pair<std::string, std::string>> users_;  // email -> (password, username)
    std::unordered_map<std::string, std::string> userIds_;  // email -> id
}

std::shared_ptr<User> UserService::authenticate(const std::string& email, const std::string& password) {
    auto it = users_.find(email);
    if (it == users_.end() || it->second.first != password) {
        return nullptr;
    }
    
    return std::make_shared<User>(
        userIds_[email],
        email,
        it->second.second
    );
}

User UserService::createUser(const std::string& email, const std::string& username, const std::string& password) {
    if (users_.find(email) != users_.end()) {
        throw std::runtime_error("User already exists");
    }
    
    static int nextId = 1;
    std::string userId = "USER-" + std::to_string(nextId++);
    
    users_[email] = {password, username};
    userIds_[email] = userId;
    
    return User(userId, email, username);
}

void UserService::updateUser(const std::string& userId, const std::string& email, const std::string& username) {
    // Implementation would update user details in database
}

void UserService::changePassword(const std::string& userId, const std::string& oldPassword, const std::string& newPassword) {
    // Implementation would update password in database
}

} // namespace