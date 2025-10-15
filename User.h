#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User {
public:
    string userID, name, email;

    User(string id, string n, string e);
};

#endif
