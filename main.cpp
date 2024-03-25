#include <iostream>
#include <ctime>
#include "generalHeader.h"

using namespace std;

void displayMenu()
{
  cout << "            ____________________________________________\n";
  cout << "           |                |                             |\n";
  cout << "           |       *******       *******                  |\n";
  cout << "           |       * T * E * T * R * I * S * T * E *       \n";
  cout << "           |        Menu Options:                         |\n"; // Clearer label
  cout << "           |           1) Insert Left                     |\n";
  cout << "           |           2) Insert Right                    |\n";  // Consistent formatting
  cout << "           |           3) Shift by Color                  |\n";
  cout << "           |           4) Shift by Shape                  |\n";  // Option "s" changed to "4"
  cout << "           |           5) Quit                            |\n";  // Added "5" and rephrased
  cout << "           |______________________________________________|\n\n\n\n\n";
}

void displayGamePieces(Piece **pieces, int numPieces)
{
    for (int i = 0; i < numPieces; ++i)
    {
        cout << "Piece " << i + 1 << ": " << pieces[i]->displayPiece() << " ";
    }
    cout << endl;
}

int main()
{
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    // Generate random color and shape indices
    int randomColorIndex = rand() % static_cast<int>(T_Color::WHITE);
    int randomShapeIndex = rand() % static_cast<int>(T_Shape::PLUS);

    // Use the random indices to initialize the game
    Game *currentGame = initializeGame(randomColorIndex, randomShapeIndex);

    int continueGame = 1;

    // Generate and insert the initial five pieces into the game
    for (int i = 0; i < 5; ++i)
    {
        randomColorIndex = rand() % static_cast<int>(T_Color::WHITE);
        randomShapeIndex = rand() % static_cast<int>(T_Shape::PLUS);
        Piece *newPiece = currentGame->drawPiece(randomColorIndex, randomShapeIndex);
        currentGame->insertPieceInRight(currentGame, newPiece);
    }

    while (continueGame)
    {

        randomColorIndex = rand() % static_cast<int>(T_Color::WHITE);
        randomShapeIndex = rand() % static_cast<int>(T_Shape::PLUS);

        Piece * nextPiece = currentGame->drawPiece(randomColorIndex, randomShapeIndex);

        // Display current game state
        cout << "Current game state: ";
        displayGamePieces(currentGame->getPieces(), currentGame->piecesCount);

        // Display next piece
        cout << "Next piece: " << nextPiece->displayPiece() << endl;

        // Display menu and get player input
        displayMenu();
        char choice;
        cout << "Your choice: ";
        cin >> choice;
        cout << endl;

        switch (choice)
        {
        case 'j':
            currentGame->insertPieceInLeft(currentGame, nextPiece);
            break;
        case 'k':
            currentGame->insertPieceInRight(currentGame, nextPiece);
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

        cout << currentGame->piecesCount <<endl;

        delete nextPiece;

        // Update game state
        int scoreChange = currentGame->updateGame(currentGame);

        // Check if the game is over
        if (scoreChange == -1)
        {
            cout << "Congratulations! You've won!" << endl;
            continueGame = 0;
        }

        // Debugging: Print score change
        cout << "Score change: " << scoreChange << endl;
    }

    delete currentGame;

    return 0;
}
