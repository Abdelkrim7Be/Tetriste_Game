#include <iostream>
#include <cstdlib>
#include <ctime>
#include "generalHeader.h"

using namespace std;

void displayMenu() {
    cout << "Main Menu:" << endl;
    cout << "j. Insert on the left" << endl;
    cout << "k. Insert on the right" << endl;
    cout << "c. Shift by color" << endl;
    cout << "s. Shift by shape" << endl;
    cout << "q. Quit" << endl;
}

void displayGamePieces(Piece **pieces, int numPieces) {
    for (int i = 0; i < numPieces; ++i) {
        cout << "Piece " << i + 1 << ": " << pieces[i]->displayPiece() << " ";
    }
    cout << endl;
}

int main() {
    // Initialization of the random number generator
    srand(time(NULL));

    // Initialize the game with custom color and shape counts
    Game *currentGame = initializeGame(4, 4); // Example: 4 colors, 4 shapes

    int continueGame = 1;
    cout <<"heyyyy" << endl;

    while (continueGame) {
        // Allocate memory for nextPieces array
        Piece **nextPieces = new Piece*[5];

        // Check if memory allocation is successful
        if (nextPieces == nullptr) {
            cerr << "Memory allocation failed. Exiting program." << endl;
            return 1;
        }

        for (int i = 0; i < 5; i++) {
            nextPieces[i] = currentGame->drawPiece(4, 4); // Example: 4 colors, 4 shapes
        }

        // Display current game state
        cout << "Current game state: ";
        displayGamePieces(nextPieces, 5);

        // Display next piece
        cout << "Next piece: " << nextPieces[4]->displayPiece() << endl;

        // Display menu and get player input
        displayMenu();
        char choice;
        cout << "Your choice: ";
        cin >> choice;
        cout << endl;

        switch (choice) {
            case 'j':
                currentGame->insertPieceInLeft(currentGame, nextPieces[4]);
                break;
            case 'k':
                currentGame->insertPieceInRight(currentGame, nextPieces[4]);
                break;
            // Other cases for color and shape shifting...
            case 'q':
                // Quit the game
                continueGame = 0;
                break;
            default:
                // Invalid choice
                cout << "Invalid choice. Please try again." << endl;
                break;
        }

        // Free memory for nextPieces array
        for (int i = 0; i < 5; i++) {
            delete nextPieces[i];
        }
        delete[] nextPieces;

        // Update game state
        int scoreChange = currentGame->updateGame(currentGame);

        // Check if the game is over
        if (scoreChange == -1) {
            cout << "Congratulations! You've won!" << endl;
            continueGame = 0;
        }

        // Debugging: Print score change
        cout << "Score change: " << scoreChange << endl;
    }

    delete currentGame;

    return 0;
}
