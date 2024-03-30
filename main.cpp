
#include <iostream>
#include <ctime>
#include "generalHeader.h"

using namespace std;

void displayMenu()
{
  cout << "\t\t\t             ____________________________________________\n";
  cout << "\t\t\t            |                |                             |\n";
  cout << "\t\t\t            |       *******       *******                  |\n";
  cout << "\t\t\t            |       * T * E * T * R * I * S * T * E *       \n";
  cout << "\t\t\t            |        Menu Options:                         |\n"; // Clearer label
  cout << "\t\t\t            |           j) Insert Left                     |\n";
  cout << "\t\t\t            |           k) Insert Right                    |\n";  // Consistent formatting
  cout << "\t\t\t            |           c) Shift by Color                  |\n";
  cout << "\t\t\t            |           s) Shift by Shape                  |\n";  // Option "s" changed to "4"
  cout << "\t\t\t            |           q) Quit                            |\n";  // Added "5" and rephrased
  cout << "\t\t\t            |______________________________________________|\n\n\n\n\n";
}

//T_Color displayColorMenu(int purple, int white) {
T_Color displayColorMenu() {
    cout << "Which color do you want to shift?" << endl;
    cout << "b. Blue" << endl;
    cout << "y. Yellow" << endl;
    cout << "r. Red" << endl;
    cout << "g. Green" << endl;
    //if (purple) {
        cout << "p. Purple" << endl;
   // }
    //if (white) {
        cout << "w. White" << endl;
    //}
    cout << "0. Back" << endl;

    char colorChoice;
    cout << "Your choice: ";
    cin >> colorChoice;
    cout << endl;

    T_Color chosenColor;

    // Convert the character choice to T_Color
    switch (colorChoice) {
        case 'b':
            chosenColor = T_Color::BLUE;
            break;
        case 'y':
            chosenColor = T_Color::YELLOW;
            break;
        case 'r':
            chosenColor = T_Color::RED;
            break;
        case 'g':
            chosenColor = T_Color::GREEN;
            break;
        case 'p':
            chosenColor = T_Color::PURPLE;
            break;
        case 'w':
            chosenColor = T_Color::WHITE;
            break;
        default:
            // Handle invalid choice
            cout << "Invalid color choice." << endl;
            // You might want to ask the user to choose again or handle the error differently
            break;
    }

    return chosenColor;
}


//T_Shape displayShapeMenu(int star, int plus) {
T_Shape displayShapeMenu() {
    cout << "Which shape do you want to shift?" << endl;
    cout << "s. Square" << endl;
    cout << "d. Diamond" << endl;
    cout << "c. Circle" << endl;
    cout << "t. Triangle" << endl;
    //if (star) {
        cout << "a. Star" << endl;
   // }
    //if (plus) {
        cout << "p. Plus" << endl;
    //}
    cout << "0. Back" << endl;

    char shapeChoice;
    cout << "Your choice: ";
    cin >> shapeChoice;
    cout << endl;

    T_Shape chosenShape;

    // Convert the character choice to T_Shape
    switch (shapeChoice) {
        case 's':
            chosenShape = T_Shape::SQUARE;
            break;
        case 'd':
            chosenShape = T_Shape::DIAMOND;
            break;
        case 'c':
            chosenShape = T_Shape::CIRCLE;
            break;
        case 't':
            chosenShape = T_Shape::TRIANGLE;
            break;
        case 'a':
            chosenShape = T_Shape::STAR;
            break;
        case 'p':
            chosenShape = T_Shape::PLUS;
            break;
        default:
            // Handle invalid choice
            cout << "Invalid shape choice." << endl;
            // You might want to ask the user to choose again or handle the error differently
            break;
    }

    return chosenShape;
}


void displayGamePieces(Piece **pieces, int numPieces)
{
    for (int i = 0; i < numPieces; ++i)
    {
        cout << "Piece " << i + 1 << ": " << pieces[i]->displayPiece() << " ";
    }
    cout << endl;
}

void countOfColorsAndShapes(Game* game, int &countOfColors, int &countOfShapes) {
    countOfColors = 0;
    countOfShapes = 0;

    bool colorsCounted[6] = {false}; // 6 est le nombre total de couleurs possibles
    bool shapesCounted[6] = {false}; // 6 est le nombre total de formes possibles

    // Parcourir la liste de pièces du jeu
    Piece *currentPiece = game->head;
    do {
        // Vérifier si la couleur de la pièce a déjà été comptée
        if (!colorsCounted[static_cast<int>(currentPiece->color)]) {
            colorsCounted[static_cast<int>(currentPiece->color)] = true;
            countOfColors++; // Incrémenter le nombre de couleurs
        }

        // Vérifier si la forme de la pièce a déjà été comptée
        if (!shapesCounted[static_cast<int>(currentPiece->shape)]) {
            shapesCounted[static_cast<int>(currentPiece->shape)] = true;
            countOfShapes++; // Incrémenter le nombre de formes
        }

        // Passer à la pièce suivante
        currentPiece = currentPiece->nextPiece;
    } while (currentPiece->nextPiece != game->head);
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

    int colorCounter = 0, shapeCounter = 0;
    int scoreChange = currentGame->score;

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
        cout << "Score : " << scoreChange << endl;
        cout << "\n\n\n";

        randomColorIndex = rand() % static_cast<int>(T_Color::WHITE);
        randomShapeIndex = rand() % static_cast<int>(T_Shape::PLUS);

        Piece * nextPiece = currentGame->drawPiece(randomColorIndex, randomShapeIndex);


        // Display menu and get player input
        displayMenu();
        // Display current game state
        cout << "Current game state: ";
        displayGamePieces(currentGame->getPieces(), currentGame->piecesCount);

        // Display next piece
        cout << "Next piece: " << nextPiece->displayPiece() << endl;
        char choice;
        cout << "Your choice: ";
        cin >> choice;
        cout << endl;
        T_Color chosenColor;
        T_Shape chosenShape;

        countOfColorsAndShapes(currentGame, colorCounter, shapeCounter);

        switch (choice)
        {
        case 'j':
            currentGame->insertPieceInLeft(currentGame, nextPiece);
            break;
        case 'k':
            currentGame->insertPieceInRight(currentGame, nextPiece);
            break;
        case 'c':
            //chosenColor = displayColorMenu(currentGame->colorIndex > 4, currentGame->colorIndex == 6);
            chosenColor = displayColorMenu();
            currentGame->colorShifting(currentGame, chosenColor, shapeCounter);
            break;
        case 's':
            //chosenShape = displayShapeMenu(currentGame->shapeIndex > 4, currentGame->shapeIndex == 6);
            chosenShape = displayShapeMenu();
            currentGame->shapeShifting(currentGame, chosenShape, colorCounter);
            break;
        case 'q':
            // Quit the game
            continueGame = 0;
            break;
        default:
            // Invalid choice
            cout << "Invalid choice. Please try again." << endl;
            break;
        }

        //cout << currentGame->piecesCount <<endl;


        // Update game state
        scoreChange = currentGame->updateGame(currentGame);


        // Check if the game is over
        if (scoreChange == -1)
        {
            cout << "Congratulations! You've won!" << endl;
            continueGame = 0;
        }

        // Debugging: Print score change
        //cout << "Score change: " << scoreChange << endl;
    }

    delete currentGame;

    return 0;
}
