// include/mercuryTrade/services/UserService.hpp
#pragma once
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace mercuryTrade {

class User {
public:
    User(const std::string& id, const std::string& email, const std::string& username)
        : m_id(id), m_email(email), m_username(username) {}
    
    std::string getId() const { return m_id; }
    std::string getEmail() const { return m_email; }
    std::string getUsername() const { return m_username; }

    nlohmann::json toJson() const {
        return {
            {"id", m_id},
            {"email", m_email},
            {"username", m_username}
        };
    }

private:
    std::string m_id;
    std::string m_email;
    std::string m_username;
};

class UserService {
public:
    std::shared_ptr<User> authenticate(const std::string& email, const std::string& password);
    User createUser(const std::string& email, const std::string& username, const std::string& password);
    void updateUser(const std::string& userId, const std::string& email, const std::string& username);
    void changePassword(const std::string& userId, const std::string& oldPassword, const std::string& newPassword);

private:
    // Would connect to user database
};

} // namespace mercuryTrade