#include "UserManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

UserManager::UserManager(const std::string& filename) : filename(filename) {
    loadUsers();
}

bool UserManager::isLoggedIn() const {
    return !currentUserPseudo.empty();
}

bool UserManager::loginOrCreate(const std::string& pseudo) {
    std::string trimmed = pseudo;
    // Basic trimming
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

    if (!isValidPseudo(trimmed)) return false;

    if (allUsers.find(trimmed) == allUsers.end()) {
        allUsers[trimmed] = 0;
        saveUsers();
    }
    
    currentUserPseudo = trimmed;
    return true;
}

void UserManager::logout() {
    currentUserPseudo = "";
}

void UserManager::updateRecord(int score) {
    if (currentUserPseudo.empty()) return;
    if (score > allUsers[currentUserPseudo]) {
        allUsers[currentUserPseudo] = score;
        saveUsers();
    }
}

int UserManager::getCurrentUserRecord() const {
    if (currentUserPseudo.empty()) return 0;
    return allUsers.at(currentUserPseudo);
}

std::string UserManager::getCurrentUserPseudo() const {
    return currentUserPseudo;
}

std::vector<UserRecord> UserManager::getTopRecords(int n) const {
    std::vector<UserRecord> records;
    for (auto const& pair : allUsers) {
        records.push_back({pair.first, pair.second});
    }
    
    std::sort(records.begin(), records.end(), [](const UserRecord& a, const UserRecord& b) {
        return a.record > b.record;
    });

    if (records.size() > (size_t)n) {
        records.resize(n);
    }
    return records;
}

bool UserManager::userExists(const std::string& pseudo) const {
    std::string trimmed = pseudo;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
    return allUsers.find(trimmed) != allUsers.end();
}

void UserManager::loadUsers() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        migrateOldFormat();
        return;
    }

    try {
        json j;
        file >> j;
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::string name = it.key();
            if (name.empty()) continue;

            if (it.value().is_number()) {
                allUsers[name] = it.value().get<int>();
            } else if (it.value().is_object()) {
                // Backward compatibility for { "password": "...", "record": X }
                if (it.value().contains("record")) {
                    allUsers[name] = it.value()["record"];
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading users: " << e.what() << std::endl;
    }
    file.close();
}

void UserManager::saveUsers() {
    json j;
    for (auto const& pair : allUsers) {
        if (pair.first.empty()) continue;
        j[pair.first] = pair.second;
    }

    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    }
}

void UserManager::migrateOldFormat() {
    std::ifstream file("assets/scores.txt");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string pseudo, password;
        int record;
        // Try to read: pseudo password record (legacy)
        if (ss >> pseudo >> password >> record) {
            allUsers[pseudo] = record;
        } else {
            // Try to read: pseudo record
            ss.clear();
            ss.str(line);
            if (ss >> pseudo >> record) {
                allUsers[pseudo] = record;
            }
        }
    }
    file.close();
    saveUsers();
    std::cout << "Successfully migrated legacy scores to modern JSON format." << std::endl;
}

bool UserManager::isValidPseudo(const std::string& pseudo) const {
    if (pseudo.empty()) return false;
    if (pseudo.length() > 15) return false;
    
    // Check if alphanumeric
    return std::all_of(pseudo.begin(), pseudo.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-';
    });
}
