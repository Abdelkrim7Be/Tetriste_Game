#ifndef GENERALHEADER_H_INCLUDED
#define GENERALHEADER_H_INCLUDED

#include "gameDeclaration.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <math.h>

using namespace std;

// The constructor of the game
Game::Game(int colorIndex, int shapeIndex)
{

    this->head = nullptr;
    this->score = 0;
    this->piecesCount = 0;
    this->colorIndex = colorIndex;
    this->shapeIndex = shapeIndex;
    //cout << "Game constructor success" << endl;
}

// The destrcutor of the game
Game::~Game()
{
    Piece *current = head;
    while (current != nullptr)
    {
        Piece *next = current->nextPiece;
        delete current;
        current = next;
    }
    //cout << "Game destructor success" << endl;
}

string displayPieceAsString(Piece *piece)
{

   T_Color color = static_cast<T_Color>(static_cast<int>(piece->color));
   T_Shape shape = static_cast<T_Shape>(static_cast<int>(piece->shape));

    // Convert enums to strings
    string colorStr;
    string shapeStr;

    switch (color) {
        case T_Color::BLUE:
            colorStr = "\033[34m"; // Blue color
            break;
        case T_Color::YELLOW:
            colorStr = "\033[33m"; // Yellow color
            break;
        case T_Color::RED:
            colorStr = "\033[31m"; // Red color
            break;
        case T_Color::GREEN:
            colorStr = "\033[32m"; // Green color
            break;
        case T_Color::WHITE:
            colorStr = "\033[37m"; // White color
            break;
    }
    switch (shape) {
        case T_Shape::SQUARE:
            shapeStr = "Square"; // Square Unicode character
            break;
        case T_Shape::DIAMOND:
            shapeStr = "Diamond"; // Diamond Unicode character
            break;
        case T_Shape::CIRCLE:
            shapeStr = "Circle"; // Circle Unicode character
            break;
        case T_Shape::TRIANGLE:
            shapeStr = "Triangle"; // Triangle Unicode character
            break;
        case T_Shape::STAR:
            shapeStr = "Star"; // Star Unicode character
            break;
    }

    // Combine color and shape strings
    string displayStr = colorStr + shapeStr + "\033[0m";

    return displayStr;
}
// The constructor of the Piece class
//  The constructor of the Piece class
Piece::Piece(T_Color clr, T_Shape spe, Piece *nextPiece, Piece *shapePrev, Piece *shapeNext, Piece *colorPrev, Piece *colorNext)
{
    try
    {
        T_Color color = clr;
        T_Shape shape = spe;

        this->color = color;
        this->shape = shape;
        this->nextPiece = nextPiece;
        this->colorNext = colorNext;
        this->colorPrev = colorPrev;
        this->shapeNext = shapeNext;
        this->shapePrev = shapePrev;

        this->displayString = displayPieceAsString(this);
        //cout << "Piece constructor success" << endl;
    }
    catch (const std::exception &e)
    {
        std::cout << "Piece constructor failed: " << e.what() << std::endl;
        throw; // Re-throw the exception to propagate it
    }
}

string Piece::displayPiece()
{
    return displayString;
}

Piece::~Piece()
{
    //cout << "Piece destructor success" << endl;
}

Game *initializeGame(int colorIndex, int shapeIndex)
{
    Game *newGame = new Game(colorIndex, shapeIndex);
    Piece *newPiece = newGame->drawPiece(colorIndex, shapeIndex);
    newPiece->nextPiece = newPiece;
    newPiece->shapePrev = newPiece;
    newPiece->shapeNext = newPiece;
    newPiece->colorPrev = newPiece;
    newPiece->colorNext = newPiece;

    newGame->head = newPiece;
    newGame->piecesCount = 1;

    //cout << "Game initiation success" << endl;

    return newGame;
}

// Calculate the length of a list
int lengthList(Game *game)
{
    int count = 0;
    Piece *current = game->head;
    while (current->nextPiece != game->head)
    {
        count++;
        current = current->nextPiece;
    }
    return count;
}

Piece *Game::drawPiece(int colorIndex, int shapeIndex)
{
    T_Color color = static_cast<T_Color>(colorIndex);
    T_Shape shape = static_cast<T_Shape>(shapeIndex);

    Piece *newPiece = new Piece(color, shape, nullptr, nullptr, nullptr, nullptr, nullptr);
    //cout << "Game drawing success" << endl;
    return newPiece;
}

Piece *Game::retrieveTail(Game *game)
{
    Piece *current = game->head;
    while (current->nextPiece != game->head)
    {
        current = current->nextPiece;
    }
    return current;
}

// Inserting a piece in the right side
void Game::insertPieceInRight(Game *game, Piece *newPiece)
{
    try
    {
        if (game->piecesCount < 15)
        {
            Piece *tail = retrieveTail(game);
            tail->nextPiece = newPiece;
            newPiece->nextPiece = game->head;
            Game::updateColorAfterAdding(newPiece);
            Game::updateShapeAfterAdding(newPiece);
            game->piecesCount++;
            //cout << "Insertion success in right" << endl;
        }

    }
    catch (const std::exception &e)
    {
        std::cerr << "Insertion failed: " << e.what() << std::endl;
        throw; // Re-throw the exception to propagate it
    }
}

Piece** Game::getPieces() {
        // Allocate memory for pieces array
        Piece** pieces = new Piece*[piecesCount];
        if (pieces == nullptr) {
            cerr << "Memory allocation failed. Exiting program." << endl;
            exit(1);
        }

        // Copy pieces from the game board to the pieces array
        Piece* current = head;
        for (int i = 0; i < piecesCount; ++i) {
            pieces[i] = current;
            current = current->nextPiece;
        }

        return pieces;
}
// Inserting a piece in the left side
void Game::insertPieceInLeft(Game *game, Piece *newPiece)
{
    try
    {
        Game::insertPieceInRight(game, newPiece);
        game->head = newPiece;
        //cout << "Insertion success in left" << endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Insertion failed: " << e.what() << std::endl;
        throw; // Re-throw the exception to propagate it
    }
}

void Game::updateShapeAfterAdding(Piece* piece) {
    Piece* current = piece->nextPiece;
    while (current->shape != piece->shape) {
        current = current->nextPiece;
    }

    if (current == piece) {
        piece->shapePrev = piece;
        piece->shapeNext = piece;
    } else {
        piece->shapePrev = current->shapePrev;
        piece->shapePrev->shapeNext = piece;
        piece->shapeNext = current;
        piece->shapeNext->shapePrev = piece;
    }
}

// Function to update the doubly circular linked list of colors for a given piece that is added to the board
void Game::updateColorAfterAdding(Piece* piece) {
    Piece* current = piece->nextPiece;
    while (current->color != piece->color) {
        current = current->nextPiece;
    }

    if (current == piece) {
        piece->colorPrev = piece;
        piece->colorNext = piece;
    } else {
        piece->colorPrev = current->colorPrev;
        piece->colorPrev->colorNext = piece;
        piece->colorNext = current;
        piece->colorNext->colorPrev = piece;
    }
}

int Game::similarSequenceTracker(Game *game, Piece *newPiece)
{
    int colorSequence = 1, shapeSequence = 1;
    Piece *current = newPiece->nextPiece;
    for (int i = 1; i <= piecesCount; i++)
    {
        if (current == game->head)
            break;
        if (colorSequence == i && current->color == newPiece->color)
            colorSequence++;
        if (shapeSequence == i && current->shape == newPiece->shape)
            shapeSequence++;
        current = current->nextPiece;
    }
    if (colorSequence > shapeSequence)
        return colorSequence;
    return shapeSequence;
}

int Game::updateGame(Game *game)
{
    try
    {
        int initialScore = game->score;
        // No need to check if there are less than 3 pieces
        if (game->piecesCount < 3)
        {
            return 0;
        }

        Piece *currentPiece = game->head;
        Piece *beforeCurrent = nullptr;
        Piece *tail = retrieveTail(game);
        int combinationSize = 0;

        int combo = 0;
        while (currentPiece != nullptr && currentPiece->nextPiece != game->head)
        {
            combinationSize = similarSequenceTracker(game, currentPiece);
            //printf("Combination found : %d\n", combinationSize);
            // If there are at least 3 pieces of the same color or shape, delete them
            if (combinationSize >= 3)
            {
                // If there are combos, the score will augment exponentially (3^x or 4^x etc)
                combo++;
                game->score += pow(combinationSize, combo) * 1;

                // If all the pieces on the board are deleted, it's a win, the game will end so everything will be correctly freed
                if (game->piecesCount == combinationSize)
                {
                    return -1;
                }

                game->piecesCount -= combinationSize;



                for (int i = 0; i < combinationSize; i++)
                {
                    currentPiece->shapePrev->shapeNext = currentPiece->shapeNext;
                    currentPiece->shapeNext->shapePrev = currentPiece->shapePrev;
                    currentPiece->colorPrev->colorNext = currentPiece->colorNext;
                    currentPiece->colorNext->colorPrev = currentPiece->colorPrev;
                    currentPiece->~Piece();
                    currentPiece = currentPiece->nextPiece;
                }

                // Removing the pieces from the single circular linked list
                if (beforeCurrent == nullptr)
                { // In this case the head got removed
                    game->head = currentPiece;
                    tail->nextPiece = game->head;
                }
                else
                {
                    beforeCurrent->nextPiece = currentPiece;
                    if (currentPiece == game->head)
                        tail = beforeCurrent; // Checking whether the tail got removed
                }

                // Resetting beforeCurrent to nullptr as we need to start over the list
                beforeCurrent = nullptr;
                currentPiece = game->head;
            }
            else
            {
                beforeCurrent = currentPiece;
                currentPiece = currentPiece->nextPiece;
            }
        }

        return game->score - initialScore;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Update game failed: " << e.what() << std::endl;
        throw; // Re-throw the exception to propagate it
    }
}

void switchingShapes(Piece *piece){
    T_Shape tempShape = piece->shape;
    piece->shape = piece->colorNext->shape;
    piece->colorNext->shape = tempShape;

    string tempDisplay = piece->displayString;
    piece->displayString = piece->colorNext->displayString;
    piece->colorNext->displayString = tempDisplay;
}


void switchingColors(Piece *piece){
    T_Color tempColor = piece->color;
    piece->color = piece->shapeNext->color;
    piece->shapeNext->color = tempColor;

    string tempDisplay = piece->displayString;
    piece->displayString = piece->shapeNext->displayString;
    piece->shapeNext->displayString = tempDisplay;
}

void Game::colorShifting(Game *game, T_Color color, int countOfShapes){
    Piece *currentColor = game->head;
    Piece *itsPreviousColor = nullptr;
    int rs = 0;

    while(currentColor->nextPiece != game->head){
        if(currentColor->color == color){
            rs = 1;
            break;
        }

        currentColor = currentColor->nextPiece;
    }
    if(rs == 0){
        std::cout << "There is no piece having this color" << std::endl;
    }

    itsPreviousColor = currentColor->colorPrev;

    if(currentColor != itsPreviousColor){

        Piece * currentPiece = currentColor;

        while(currentPiece != itsPreviousColor){
            switchingShapes(currentPiece);

            currentPiece = currentPiece->colorNext;
        }

        Piece *heads[6] = { nullptr };
        Piece *tails[6] = { nullptr };

        currentPiece = game->head;
        do {
            T_Color clr = currentPiece->color;
            int i = static_cast<int>(clr);
            if (heads[i] == nullptr) {
                heads[i] = currentPiece;
                tails[i] = currentPiece;
            } else {
                tails[i]->shapeNext = currentPiece;
                currentPiece->shapePrev = tails[i];
                tails[i] = currentPiece;
            }
            currentPiece = currentPiece->nextPiece;
        } while (currentPiece != game->head);

        std::cout << "shape count : " << countOfShapes <<endl;

        for (int i = 0; i < countOfShapes; i++) {
            if (heads[i] != nullptr) {
                tails[i]->shapeNext = heads[i];
                heads[i]->shapePrev = tails[i];
            }
        }

    }
}

void Game::shapeShifting(Game *game, T_Shape shape, int countOfColors){

    Piece *currentShape = game->head;
    Piece *itsPreviousShape = nullptr;
    int rs = 0;

    while(currentShape->nextPiece != game->head){
        if(currentShape->shape == shape){
            rs = 1;
            break;
        }

        currentShape = currentShape->nextPiece;
    }
    if(rs == 0){
        std::cout << "There is no piece having this shape" << std::endl;
    }

    itsPreviousShape = currentShape->shapePrev;

    if(currentShape != itsPreviousShape){

        Piece * currentPiece = currentShape;

        while(currentPiece != itsPreviousShape){
            switchingColors(currentPiece);

            currentPiece = currentPiece->shapeNext;
        }

        Piece *heads[6] = { nullptr };
        Piece *tails[6] = { nullptr };

        currentPiece = game->head;
        do {
            T_Shape sh = currentPiece->shape;
            int i = static_cast<int>(sh);
            if (heads[i] == nullptr) {
                heads[i] = currentPiece;
                tails[i] = currentPiece;
            } else {
                tails[i]->colorNext = currentPiece;
                currentPiece->colorPrev = tails[i];
                tails[i] = currentPiece;
            }
            currentPiece = currentPiece->nextPiece;
        } while (currentPiece != game->head);

        std::cout << "color count : " << countOfColors<<endl;

        for (int i = 0; i < countOfColors; i++) {
            if (heads[i] != nullptr) {
                tails[i]->colorNext = heads[i];
                heads[i]->colorPrev = tails[i];
            }
        }

    }
}

#endif // GENERALHEADER_H_INCLUDED
