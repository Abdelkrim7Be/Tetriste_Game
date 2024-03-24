#ifndef GENERALHEADER_H_INCLUDED
#define GENERALHEADER_H_INCLUDED

#include "gameDeclaration.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <math.h>

using namespace std;

//The constructor of the game
Game::Game(int colorCount, int shapeCount)
{

    this->head = nullptr;
    this->score = 0;
    this->piecesCount = 0;
    this->colorCount = colorCount;
    this->shapeCount = shapeCount;
    cout << "Game constructor success" << endl;
}

//The destrcutor of the game
Game::~Game(){
    Piece* current = head;
    while (current != nullptr)
    {
        Piece* next = current->nextPiece;
        delete current;
        current = next;
    }
    cout << "Game destructor success" << endl;
}

string displayPieceAsString(Piece* piece) {
    string shapes[] = { "Square", "Diamond", "Circle", "Triangle", "Star", "Plus" };
    string colors[] = { "Blue", "Yellow", "Red", "Green", "Purple", "White" };

    int colorIndex = static_cast<int>(piece->color);
    int shapeIndex = static_cast<int>(piece->shape);

    string displayStr = colors[colorIndex] + " " + shapes[shapeIndex];

    return displayStr;
}



//The constructor of the Piece class
Piece::Piece(T_Color clr, T_Shape spe, Piece* nextPiece, Piece* shapePrev, Piece* shapeNext, Piece* colorPrev, Piece* colorNext) {
    T_Color color = static_cast<T_Color>(rand() % static_cast<int>(clr));
    T_Shape shape = static_cast<T_Shape>(rand() % static_cast<int>(spe));

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



string Piece::displayPiece() {
    return displayString;
}

Piece::~Piece() {
    cout << "Piece destructor success" << endl;
}


Game* initializeGame(int colorCount, int shapeCount) {
    Game* newGame = new Game(colorCount, shapeCount);
    Piece* newPiece = newGame->drawPiece(colorCount, shapeCount);
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


//Calculate the length of a list
int lengthList(Game *game) {
    int count = 0;
    Piece* current = game->head;
    while (current != nullptr) {
        count++;
        current = current->nextPiece;
    }
    return count;
}



Piece* Game::drawPiece(int colorCount, int shapeCount) {
        T_Color color = static_cast<T_Color>(rand() % static_cast<int>(colorCount));
        T_Shape shape = static_cast<T_Shape>(rand() % static_cast<int>(shapeCount));

        Piece* newPiece = new Piece(color, shape, nullptr, nullptr, nullptr, nullptr, nullptr);
        cout << "Game drawing success" << endl;
        return newPiece;
}

Piece* Game::retrieveTail(Game* game) {
    Piece* current = game->head;
    while (current->nextPiece != NULL) {
        current = current->nextPiece;
    }
    return current;
}

//Inserting a piece in the right side
void Game::insertPieceInRight(Game* game, Piece* newPiece) {
    if (game->piecesCount < 15) {
        newPiece->nextPiece = game->head;
        game->head = newPiece;
        Game::updateColorAfterAdding(newPiece);
        Game::updateShapeAfterAdding(newPiece);
        game->piecesCount++;
         cout << "insertion good in right" <<endl;
    }else{
        cout << "bad insertion" << endl;
    }


}

//Inserting a piece in the left side
void Game::insertPieceInLeft(Game* game, Piece* newPiece) {
    Game::insertPieceInRight(game, newPiece);
    game->head = newPiece;
    cout << "insertion good in left" <<endl;
}


//After inserting , the new piece should be added to the circular double linked list of its own color
void Game::updateColorAfterAdding(Piece* newPiece) {
    Piece* current = newPiece->nextPiece;
    while (newPiece->color != current->color) {
        current = current->nextPiece;
    }
    if (newPiece == current) {
        newPiece->colorNext = newPiece;
        newPiece->colorPrev = newPiece;
    }
    else {
        newPiece->colorPrev = current->colorPrev;
        newPiece->colorPrev->colorNext = newPiece;
        newPiece->colorNext = current;
        newPiece->colorNext->colorPrev = newPiece;
    }
}

void Game::updateShapeAfterAdding(Piece* newPiece) {
    Piece* current = newPiece->nextPiece;
    while (newPiece->shape != current->shape) {
        current = current->nextPiece;
    }
    if (newPiece == current) {
        newPiece->shapeNext = newPiece;
        newPiece->shapePrev = newPiece;
    }
    else {
        newPiece->shapePrev = current->shapePrev;
        newPiece->shapePrev->shapeNext = newPiece;
        newPiece->shapeNext = current;
        newPiece->shapeNext->shapePrev = newPiece;
    }
}

int Game::similarSequenceTracker(Game* game, Piece *newPiece) {
    int colorSequence = 1, shapeSequence = 1;
    Piece *current = newPiece->nextPiece;
    // It has been decided that the maximum length of a sequence is 5
    for (int i = 1; i < 5; i++) {
        if (current == game->head)
            break;
        if (colorSequence == i && current->color == newPiece->color) colorSequence++;
        if (shapeSequence == i && current->shape == newPiece->shape) shapeSequence++;
        current = current->nextPiece;
    }
    if (colorSequence > shapeSequence) return colorSequence;
    return shapeSequence;
}

int Game::updateGame(Game* game) {
    int initialScore = game->score;
    // No need to check if there are less than 3 pieces
    if (game->piecesCount < 3) {
        return 0;
    }

    Piece* currentPiece = game->head;
    Piece* beforeCurrent = nullptr;
    Piece* tail = retrieveTail(game);

    int combo = 0;
    while (currentPiece != nullptr && currentPiece->nextPiece != game->head) {
        int combinationSize = similarSequenceTracker(game, currentPiece);
#ifdef DEBUG
        printf("Combination found : %d\n", combinationSize);
#endif
        // If there are at least 3 pieces of the same color or shape, delete them
        if (combinationSize >= 3) {
            // If there are combos, the score will augment exponentially (3^x or 4^x etc)
            combo++;
            game->score += pow(combinationSize, combo) * 1;

            // If all the pieces on the board are deleted, it's a win, the game will end so everything will be correctly freed
            if (game->piecesCount == combinationSize) {
                return -1;
            }

            game->piecesCount -= combinationSize;

            // Updating the linking for the shapes and colors
            Piece* nextPiece = nullptr;
            for (int i = 0; i < combinationSize; i++) {
                nextPiece = currentPiece->nextPiece;
                currentPiece->shapePrev->shapeNext = currentPiece->shapeNext;
                currentPiece->shapeNext->shapePrev = currentPiece->shapePrev;
                currentPiece->colorPrev->colorNext = currentPiece->colorNext;
                currentPiece->colorNext->colorPrev = currentPiece->colorPrev;
                delete currentPiece;
                currentPiece = nextPiece;
            }

            // Removing the pieces from the single circular linked list
            if (beforeCurrent == nullptr) { // In this case the head got removed
                game->head = currentPiece;
                tail->nextPiece = game->head;
            } else {
                beforeCurrent->nextPiece = currentPiece;
                if (currentPiece == game->head) tail = beforeCurrent; // Checking whether the tail got removed
            }

            // Resetting beforeCurrent to nullptr as we need to start over the list
            beforeCurrent = nullptr;
            currentPiece = game->head;
        } else {
            beforeCurrent = currentPiece;
            currentPiece = currentPiece->nextPiece;
        }
    }

    return game->score - initialScore;
}


#endif // GENERALHEADER_H_INCLUDED
