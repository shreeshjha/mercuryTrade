#pragma once
#include <string>
#include <memory>

namespace mercuryTrade {

class User {
public:
    User(const std::string& email, const std::string& username, const std::string& password)
        : m_email(email), m_username(username), m_password(password) {}
    
    std::string getId() const { return m_id; }
    std::string getEmail() const { return m_email; }
    std::string getUsername() const { return m_username; }

private:
    std::string m_id;
    std::string m_email;
    std::string m_username;
    std::string m_password;
};

class UserService {
public:
    std::shared_ptr<User> authenticate(const std::string& email, const std::string& password);
    User createUser(const User& user);
private:
    // Add database connection later
};
}