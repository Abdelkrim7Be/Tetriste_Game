#include <iostream>
#include <cassert>
#include <fstream>
#include "UserManager.h"

void test_loginOrCreate() {
    std::cout << "Testing loginOrCreate with PIN..." << std::endl;
    std::string testFile = "assets/test_users_profiles.json";
    std::remove(testFile.c_str());

    UserManager um(testFile);
    
    // Test creation with PIN
    assert(um.loginOrCreate("Karimo", "1234") == true);
    assert(um.isLoggedIn() == true);
    assert(um.getCurrentUserPseudo() == "Karimo");
    assert(um.verifyPin("Karimo", "1234") == true);
    assert(um.verifyPin("Karimo", "0000") == false);

    // Test record update
    um.updateRecord(10);
    assert(um.getCurrentUserRecord() == 10);

    // Test profile update
    UserProfile p = um.getCurrentUserProfile();
    p.avatarId = "red_circle";
    um.updateProfile(p);
    assert(um.getCurrentUserProfile().avatarId == "red_circle");

    um.logout();
    assert(um.isLoggedIn() == false);

    // Test loading existing with PIN verification
    UserManager um2(testFile);
    assert(um2.userExists("Karimo") == true);
    assert(um2.verifyPin("Karimo", "1234") == true);
    assert(um2.loginOrCreate("Karimo", "1234") == true);
    assert(um2.getCurrentUserRecord() == 10);
    assert(um2.getCurrentUserProfile().avatarId == "red_circle");

    std::remove(testFile.c_str());
    std::cout << "loginOrCreate passed!" << std::endl;
}

void test_topRecords() {
    std::cout << "Testing topRecords(3)..." << std::endl;
    std::string testFile = "assets/test_users_top_profiles.json";
    std::remove(testFile.c_str());

    UserManager um(testFile);
    um.loginOrCreate("User1", "1111"); um.updateRecord(10);
    um.loginOrCreate("User2", "2222"); um.updateRecord(50);
    um.loginOrCreate("User3", "3333"); um.updateRecord(30);
    um.loginOrCreate("User4", "4444"); um.updateRecord(5);
    um.loginOrCreate("User5", "5555"); um.updateRecord(100);
    um.loginOrCreate("User6", "6666"); um.updateRecord(20);

    auto top = um.getTopRecords(3);
    assert(top.size() == 3);
    assert(top[0].pseudo == "User5");
    assert(top[0].record == 100);
    assert(top[1].pseudo == "User2");
    assert(top[2].pseudo == "User3");

    std::remove(testFile.c_str());
    std::cout << "topRecords passed!" << std::endl;
}

void test_sessionPersistence() {
    std::cout << "Testing beginSession/endSession..." << std::endl;
    std::string testFile = "assets/test_users_session_profiles.json";
    std::remove(testFile.c_str());

    UserManager um(testFile);
    um.loginOrCreate("Operator", "4321");
    um.beginSession();
    um.addNodesPurged(3);
    um.endSession(42);

    UserManager reloaded(testFile);
    reloaded.loginOrCreate("Operator", "4321");
    UserProfile profile = reloaded.getCurrentUserProfile();
    assert(profile.matchesPlayed == 1);
    assert(profile.nodesPurged == 3);
    assert(profile.lastScore == 42);
    assert(profile.record == 42);

    reloaded.beginSession();
    reloaded.endSession(25);
    profile = reloaded.getCurrentUserProfile();
    assert(profile.matchesPlayed == 2);
    assert(profile.lastScore == 25);
    assert(profile.record == 42);

    std::remove(testFile.c_str());
    std::cout << "sessionPersistence passed!" << std::endl;
}

void test_migration() {
    std::cout << "Testing migration from old format..." << std::endl;
    std::ofstream legacy("assets/scores.txt");
    legacy << "LegacyUser 999" << std::endl;
    legacy.close();

    std::string testFile = "assets/test_migration_profiles.json";
    std::remove(testFile.c_str());

    UserManager um(testFile);
    assert(um.userExists("LegacyUser") == true);
    assert(um.loginOrCreate("LegacyUser") == true);
    assert(um.getCurrentUserRecord() == 999);
    // Legacy users have empty pinHash by default
    assert(um.verifyPin("LegacyUser", "") == true);

    std::remove(testFile.c_str());
    std::remove("assets/scores.txt");
    std::cout << "migration passed!" << std::endl;
}

int main() {
    std::remove("assets/scores.txt");
    test_loginOrCreate();
    test_topRecords();
    test_sessionPersistence();
    test_migration();
    std::cout << "All Phase 1 UserManager tests passed!" << std::endl;
    return 0;
}
