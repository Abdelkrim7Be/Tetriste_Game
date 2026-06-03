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

bool UserManager::loginOrCreate(const std::string& pseudo, const std::string& pin) {
    std::string trimmed = pseudo;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

    if (!isValidPseudo(trimmed)) return false;

    if (allUsers.find(trimmed) == allUsers.end()) {
        UserProfile newUser;
        newUser.pseudo = trimmed;
        newUser.pinHash = pin; // Simple direct storage for this prototype
        allUsers[trimmed] = newUser;
        saveUsers();
    }
    
    currentUserPseudo = trimmed;
    return true;
}

bool UserManager::verifyPin(const std::string& pseudo, const std::string& pin) {
    if (allUsers.find(pseudo) == allUsers.end()) return false;
    // For a real app, use hashing. Here we use plain string for 4-digit PIN as requested for simplicity.
    return allUsers[pseudo].pinHash == pin;
}

void UserManager::logout() {
    currentUserPseudo = "";
}

void UserManager::updateRecord(int score) {
    if (currentUserPseudo.empty()) return;
    if (score > allUsers[currentUserPseudo].record) {
        allUsers[currentUserPseudo].record = score;
        saveUsers();
    }
}

void UserManager::updateProfile(const UserProfile& profile) {
    if (allUsers.find(profile.pseudo) != allUsers.end()) {
        allUsers[profile.pseudo] = profile;
        saveUsers();
    }
}

int UserManager::getCurrentUserRecord() const {
    if (currentUserPseudo.empty()) return 0;
    return allUsers.at(currentUserPseudo).record;
}

std::string UserManager::getCurrentUserPseudo() const {
    return currentUserPseudo;
}

UserProfile UserManager::getCurrentUserProfile() const {
    if (currentUserPseudo.empty()) return UserProfile();
    return allUsers.at(currentUserPseudo);
}

std::vector<UserProfile> UserManager::getTopRecords(int n) const {
    std::vector<UserProfile> records;
    for (auto const& pair : allUsers) {
        records.push_back(pair.second);
    }
    
    std::sort(records.begin(), records.end(), [](const UserProfile& a, const UserProfile& b) {
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

            UserProfile p;
            p.pseudo = name;

            if (it.value().is_number()) {
                p.record = it.value().get<int>();
            } else if (it.value().is_object()) {
                auto val = it.value();
                if (val.contains("record")) p.record = val["record"];
                if (val.contains("avatarId")) p.avatarId = val["avatarId"];
                if (val.contains("pinHash")) p.pinHash = val["pinHash"];
                if (val.contains("matchesPlayed")) p.matchesPlayed = val["matchesPlayed"];
                if (val.contains("nodesPurged")) p.nodesPurged = val["nodesPurged"];
            }
            allUsers[name] = p;
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
        const UserProfile& p = pair.second;
        json profileObj;
        profileObj["record"] = p.record;
        profileObj["avatarId"] = p.avatarId;
        profileObj["pinHash"] = p.pinHash;
        profileObj["matchesPlayed"] = p.matchesPlayed;
        profileObj["nodesPurged"] = p.nodesPurged;
        j[p.pseudo] = profileObj;
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
        if (ss >> pseudo >> password >> record) {
            UserProfile p; p.pseudo = pseudo; p.record = record;
            allUsers[pseudo] = p;
        } else {
            ss.clear();
            ss.str(line);
            if (ss >> pseudo >> record) {
                UserProfile p; p.pseudo = pseudo; p.record = record;
                allUsers[pseudo] = p;
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
    return std::all_of(pseudo.begin(), pseudo.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-';
    });
}
