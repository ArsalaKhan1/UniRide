#ifndef AUTHSYS_H
#define AUTHSYS_H

#include <string>
#include <iostream>
#include <unordered_map>
#include "User.h"
using namespace std;

class AuthSystem {
private:
    unordered_map<string, User> users; // key: email

public:
    User registerUser(string id, string name, string email);
    bool loginUser(string email);
    bool isRegistered(string email);
    void displayUsers();
};

#endif
