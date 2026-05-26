#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct UserData {
    std::string password;
    int record;
};

struct UserRecord {
    std::string pseudo;
    int record;
};

class UserManager {
public:
    UserManager(const std::string& filename = "assets/users.json") : filename(filename) {
        loadUsers();
    }

    bool userExists(const std::string& pseudo) const {
        return allUsers.find(pseudo) != allUsers.end();
    }

    bool registerUser(const std::string& pseudo, const std::string& password) {
        if (userExists(pseudo)) return false;
        allUsers[pseudo] = {password, 0};
        saveUsers();
        return true;
    }

    bool login(const std::string& pseudo, const std::string& password) {
        auto it = allUsers.find(pseudo);
        if (it != allUsers.end() && it->second.password == password) {
            currentUserPseudo = pseudo;
            return true;
        }
        return false;
    }

    void logout() {
        currentUserPseudo = "";
    }

    bool isLoggedIn() const {
        return !currentUserPseudo.empty();
    }

    void updateRecord(int score) {
        if (currentUserPseudo.empty()) return;
        if (score > allUsers[currentUserPseudo].record) {
            allUsers[currentUserPseudo].record = score;
            saveUsers();
        }
    }

    int getCurrentUserRecord() const {
        if (currentUserPseudo.empty()) return 0;
        return allUsers.at(currentUserPseudo).record;
    }

    std::string getCurrentUserPseudo() const {
        return currentUserPseudo;
    }

    std::vector<UserRecord> getTop5() const {
        std::vector<UserRecord> records;
        for (auto const& pair : allUsers) {
            records.push_back({pair.first, pair.second.record});
        }
        
        std::sort(records.begin(), records.end(), [](const UserRecord& a, const UserRecord& b) {
            return a.record > b.record;
        });

        if (records.size() > 5) {
            records.resize(5);
        }
        return records;
    }

private:
    std::string filename;
    std::string currentUserPseudo;
    std::map<std::string, UserData> allUsers;

    void loadUsers() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            // Try to migrate from old scores.txt if it exists
            migrateOldFormat();
            return;
        }

        try {
            json j;
            file >> j;
            for (auto it = j.begin(); it != j.end(); ++it) {
                allUsers[it.key()] = {it.value()["password"], it.value()["record"]};
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading users: " << e.what() << std::endl;
        }
        file.close();
    }

    void saveUsers() {
        json j;
        for (auto const& pair : allUsers) {
            j[pair.first] = {{"password", pair.second.password}, {"record", pair.second.record}};
        }

        std::ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
    }

    void migrateOldFormat() {
        std::ifstream file("assets/scores.txt");
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string pseudo, password;
            int record;
            if (ss >> pseudo >> password >> record) {
                allUsers[pseudo] = {password, record};
            } else {
                ss.clear();
                ss.str(line);
                if (ss >> pseudo >> record) {
                    allUsers[pseudo] = {"default", record};
                }
            }
        }
        file.close();
        saveUsers();
        std::cout << "Successfully migrated legacy scores to JSON format." << std::endl;
    }
};

#endif
