#include <iostream>
#include <cassert>
#include <fstream>
#include "UserManager.h"

void test_loginOrCreate() {
    std::cout << "Testing loginOrCreate..." << std::endl;
    std::string testFile = "assets/test_users.json";
    // Ensure clean state
    std::remove(testFile.c_str());

    UserManager um(testFile);
    
    // Test valid creation
    assert(um.loginOrCreate("Karimo") == true);
    assert(um.isLoggedIn() == true);
    assert(um.getCurrentUserPseudo() == "Karimo");
    assert(um.getCurrentUserRecord() == 0);

    // Test record update
    um.updateRecord(10);
    assert(um.getCurrentUserRecord() == 10);

    // Test logout
    um.logout();
    assert(um.isLoggedIn() == false);
    assert(um.getCurrentUserPseudo() == "");

    // Test loading existing
    UserManager um2(testFile);
    assert(um2.loginOrCreate("Karimo") == true);
    assert(um2.getCurrentUserRecord() == 10);

    // Test invalid pseudos
    assert(um2.loginOrCreate("") == false);
    assert(um2.loginOrCreate("   ") == false);
    assert(um2.loginOrCreate("ThisIsWayTooLongForAUsername") == false);
    assert(um2.loginOrCreate("Invalid@Char") == false);

    // Test trimming
    assert(um2.loginOrCreate("  TrimMe  ") == true);
    assert(um2.getCurrentUserPseudo() == "TrimMe");

    std::remove(testFile.c_str());
    std::cout << "loginOrCreate passed!" << std::endl;
}

void test_topRecords() {
    std::cout << "Testing topRecords(3)..." << std::endl;
    std::string testFile = "assets/test_users_top.json";
    std::remove(testFile.c_str());

    UserManager um(testFile);
    um.loginOrCreate("User1"); um.updateRecord(10);
    um.loginOrCreate("User2"); um.updateRecord(50);
    um.loginOrCreate("User3"); um.updateRecord(30);
    um.loginOrCreate("User4"); um.updateRecord(5);
    um.loginOrCreate("User5"); um.updateRecord(100);
    um.loginOrCreate("User6"); um.updateRecord(20);

    auto top = um.getTopRecords(3);
    assert(top.size() == 3);
    assert(top[0].pseudo == "User5");
    assert(top[0].record == 100);
    assert(top[1].pseudo == "User2");
    assert(top[1].record == 50);
    assert(top[2].pseudo == "User3");
    assert(top[2].record == 30);

    std::remove(testFile.c_str());
    std::cout << "topRecords passed!" << std::endl;
}

void test_migration() {
    std::cout << "Testing migration..." << std::endl;
    // Create legacy format file
    std::ofstream legacy("assets/scores.txt");
    legacy << "OldUser secret 123" << std::endl;
    legacy << "AnotherUser 456" << std::endl;
    legacy.close();

    std::string testFile = "assets/test_migration.json";
    std::remove(testFile.c_str());

    UserManager um(testFile);
    assert(um.loginOrCreate("OldUser") == true);
    assert(um.getCurrentUserRecord() == 123);
    
    assert(um.loginOrCreate("AnotherUser") == true);
    assert(um.getCurrentUserRecord() == 456);

    std::remove(testFile.c_str());
    std::cout << "migration passed!" << std::endl;
}

int main() {
    std::remove("assets/scores.txt");
    test_loginOrCreate();
    test_topRecords();
    test_migration();
    std::cout << "All UserManager tests passed!" << std::endl;
    return 0;
}
