#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <string>
#include <vector>
#include <map>

struct UserProfile {
    std::string pseudo;
    int record = 0;
    std::string avatarId = "blue_square";
    std::string pinHash = ""; // 4-digit PIN stored as string for simplicity
    
    // Stats for Phase 2
    int matchesPlayed = 0;
    int nodesPurged = 0;
    std::vector<std::string> achievements;
};

class UserManager {
public:
    UserManager(const std::string& filename = "assets/users.json");

    bool isLoggedIn() const;
    bool loginOrCreate(const std::string& pseudo, const std::string& pin = "");
    bool verifyPin(const std::string& pseudo, const std::string& pin);
    void logout();

    void updateRecord(int score);
    void updateProfile(const UserProfile& profile);
    
    int getCurrentUserRecord() const;
    std::string getCurrentUserPseudo() const;
    UserProfile getCurrentUserProfile() const;
    
    std::vector<UserProfile> getTopRecords(int n = 3) const;
    bool userExists(const std::string& pseudo) const;

private:
    std::string filename;
    std::string currentUserPseudo;
    std::map<std::string, UserProfile> allUsers; // pseudo -> profile

    void loadUsers();
    void saveUsers();
    void migrateOldFormat();
    bool isValidPseudo(const std::string& pseudo) const;
};

#endif
