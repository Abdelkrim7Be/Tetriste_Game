#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <string>
#include <vector>
#include <map>

struct UserRecord {
    std::string pseudo;
    int record;
};

class UserManager {
public:
    UserManager(const std::string& filename = "assets/users.json");

    bool isLoggedIn() const;
    bool loginOrCreate(const std::string& pseudo);
    void logout();

    void updateRecord(int score);
    int getCurrentUserRecord() const;
    std::string getCurrentUserPseudo() const;
    std::vector<UserRecord> getTopRecords(int n = 3) const;
    bool userExists(const std::string& pseudo) const;

private:
    std::string filename;
    std::string currentUserPseudo;
    std::map<std::string, int> allUsers; // pseudo -> record

    void loadUsers();
    void saveUsers();
    void migrateOldFormat();
    bool isValidPseudo(const std::string& pseudo) const;
};

#endif
