#include "AuthSys.h"

User AuthSystem::registerUser(string id, string name, string email) {
    if (users.find(email) != users.end()) {
        cout << "User already registered.\n";
        return users[email];
    }
    users[email] = User(id, name, email);
    cout << "User registered successfully!\n";
    return users[email];
}

bool AuthSystem::loginUser(string email) {
    if (users.find(email) == users.end()) {
        cout << "User not found. Please register first.\n";
        return false;
    }
    cout << "Login successful! Welcome back, " << users[email].name << endl;
    return true;
}

bool AuthSystem::isRegistered(string email) {
    return users.find(email) != users.end();
}

void AuthSystem::displayUsers() {
    cout << "\n=== Registered Users ===\n";
    for (auto &u : users)
        cout << u.second.name << "  " << u.second.email << "  " << endl;
}
