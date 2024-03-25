#ifndef GENERALHEADER_H_INCLUDED
#define GENERALHEADER_H_INCLUDED

#include "gameDeclaration.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
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
    cout << "Game constructor success" << endl;
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
    cout << "Game destructor success" << endl;
}

string displayPieceAsString(Piece *piece)
{
    T_Color color = static_cast<T_Color>(static_cast<int>(piece->color));
    T_Shape shape = static_cast<T_Shape>(static_cast<int>(piece->shape));

    // Convert enums to strings
    string colorStr;
    string shapeStr;

    switch (color)
    {
    case T_Color::BLUE:
        colorStr = "Blue";
        break;
    case T_Color::YELLOW:
        colorStr = "Yellow";
        break;
    case T_Color::RED:
        colorStr = "Red";
        break;
    case T_Color::GREEN:
        colorStr = "Green";
        break;
    case T_Color::PURPLE:
        colorStr = "Purple";
        break;
    case T_Color::WHITE:
        colorStr = "White";
        break;
    }

    switch (shape)
    {
    case T_Shape::SQUARE:
        shapeStr = "Square";
        break;
    case T_Shape::DIAMOND:
        shapeStr = "Diamond";
        break;
    case T_Shape::CIRCLE:
        shapeStr = "Circle";
        break;
    case T_Shape::TRIANGLE:
        shapeStr = "Triangle";
        break;
    case T_Shape::STAR:
        shapeStr = "Star";
        break;
    case T_Shape::PLUS:
        shapeStr = "Plus";
        break;
    }

    // Combine color and shape strings
    string displayStr = colorStr + " " + shapeStr;

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
        cout << "Piece constructor success" << endl;
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
    cout << "Piece destructor success" << endl;
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

    cout << "Game initiation success" << endl;

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
    cout << "Game drawing success" << endl;
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
            cout << "Insertion success in right" << endl;
        }
        else
        {
            cout << "Failed to insert: Stack overflow" << endl;
            throw std::runtime_error("Stack overflow");
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
        cout << "Insertion success in left" << endl;
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
            printf("Combination found : %d\n", combinationSize);
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

                // Updating the linking for the shapes and colors
                Piece *nextPiece = nullptr;
                for (int i = 0; i < combinationSize; i++)
                {
                    currentPiece->shapePrev->shapeNext = currentPiece->shapeNext;
                    currentPiece->shapeNext->shapePrev = currentPiece->shapePrev;
                    currentPiece->colorPrev->colorNext = currentPiece->colorNext;
                    currentPiece->colorNext->colorPrev = currentPiece->colorPrev;
                    delete currentPiece;
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

#endif // GENERALHEADER_H_INCLUDED
