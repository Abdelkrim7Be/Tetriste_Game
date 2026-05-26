#ifndef GENERALHEADER_H_INCLUDED
#define GENERALHEADER_H_INCLUDED

#include "gameDeclaration.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif
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
}

// The destrcutor of the game
Game::~Game()
{
    if (head == nullptr) return;
    
    Piece* tail = head;
    int count = 1;
    while (tail->nextPiece != head && count < piecesCount) {
        tail = tail->nextPiece;
        count++;
    }
    tail->nextPiece = nullptr;
    
    Piece* current = head;
    while (current != nullptr)
    {
        Piece* next = current->nextPiece;
        delete current;
        current = next;
    }
    head = nullptr;
    piecesCount = 0;
}

string displayPieceAsString(Piece *piece)
{
   T_Color color = static_cast<T_Color>(static_cast<int>(piece->color));
   T_Shape shape = static_cast<T_Shape>(static_cast<int>(piece->shape));

    string colorStr;
    string shapeStr;

    switch (color) {
        case T_Color::BLUE:
            colorStr = "\033[34m";
            break;
        case T_Color::YELLOW:
            colorStr = "\033[33m";
            break;
        case T_Color::RED:
            colorStr = "\033[31m";
            break;
        case T_Color::GREEN:
            colorStr = "\033[32m";
            break;
        case T_Color::WHITE:
            colorStr = "\033[37m";
            break;
    }
    switch (shape) {
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
    }

    return colorStr + shapeStr + "\033[0m";
}

Piece::Piece(T_Color clr, T_Shape spe, Piece *nextPiece, Piece *shapePrev, Piece *shapeNext, Piece *colorPrev, Piece *colorNext)
{
    this->color = clr;
    this->shape = spe;
    this->nextPiece = nextPiece;
    this->colorNext = colorNext;
    this->colorPrev = colorPrev;
    this->shapeNext = shapeNext;
    this->shapePrev = shapePrev;
    this->displayString = displayPieceAsString(this);
}

string Piece::displayPiece()
{
    return displayString;
}

Piece::~Piece()
{
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

    return newGame;
}

int lengthList(Game *game)
{
    if (game->head == nullptr) return 0;
    int count = 1;
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
    return new Piece(static_cast<T_Color>(colorIndex), static_cast<T_Shape>(shapeIndex), nullptr, nullptr, nullptr, nullptr, nullptr);
}

Piece *Game::retrieveTail(Game *game)
{
    if (game->head == nullptr) return nullptr;
    Piece *current = game->head;
    while (current->nextPiece != game->head)
    {
        current = current->nextPiece;
    }
    return current;
}

void Game::insertPieceInRight(Game *game, Piece *newPiece)
{
    if (game->piecesCount < 15)
    {
        Piece *tail = retrieveTail(game);
        tail->nextPiece = newPiece;
        newPiece->nextPiece = game->head;
        game->updateColorAfterAdding(newPiece);
        game->updateShapeAfterAdding(newPiece);
        game->piecesCount++;
    }
}

Piece** Game::getPieces() {
    if (piecesCount == 0) return nullptr;
    Piece** pieces = new Piece*[piecesCount];
    Piece* current = head;
    for (int i = 0; i < piecesCount; ++i) {
        pieces[i] = current;
        current = current->nextPiece;
    }
    return pieces;
}

void Game::insertPieceInLeft(Game *game, Piece *newPiece)
{
    game->insertPieceInRight(game, newPiece);
    game->head = newPiece;
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
    for (int i = 1; i < game->piecesCount; i++)
    {
        if (current == game->head) break;
        if (colorSequence == i && current->color == newPiece->color)
            colorSequence++;
        if (shapeSequence == i && current->shape == newPiece->shape)
            shapeSequence++;
        current = current->nextPiece;
    }
    return (colorSequence > shapeSequence) ? colorSequence : shapeSequence;
}

int Game::updateGame(Game *game)
{
    if (game->piecesCount < 3) return 0;
    int initialScore = game->score;
    int combo = 0;

    bool matchFound;
    do {
        matchFound = false;
        Piece *currentPiece = game->head;
        Piece *beforeCurrent = nullptr;
        int checked = 0;
        int originalCount = game->piecesCount;

        while (checked < originalCount && currentPiece != nullptr)
        {
            int combinationSize = similarSequenceTracker(game, currentPiece);
            if (combinationSize >= 3)
            {
                matchFound = true;
                combo++;
                game->score += (int)pow(combinationSize, combo);

                if (game->piecesCount == combinationSize)
                {
                    Piece* toDel[20];
                    Piece* p = game->head;
                    for (int i = 0; i < combinationSize; i++) {
                        toDel[i] = p;
                        p = p->nextPiece;
                    }
                    game->piecesCount = 0;
                    game->head = nullptr;
                    for (int i = 0; i < combinationSize; i++) delete toDel[i];
                    return -1;
                }

                Piece* seqStart = currentPiece;
                Piece* nextAfterSeq = currentPiece;
                for (int i = 0; i < combinationSize; i++) nextAfterSeq = nextAfterSeq->nextPiece;

                if (beforeCurrent == nullptr) {
                    Piece* t = seqStart;
                    while (t->nextPiece != seqStart) t = t->nextPiece;
                    game->head = nextAfterSeq;
                    t->nextPiece = game->head;
                } else {
                    beforeCurrent->nextPiece = nextAfterSeq;
                }

                Piece* piecesToDelete[20];
                Piece* p = seqStart;
                for (int i = 0; i < combinationSize; i++) {
                    piecesToDelete[i] = p;
                    p = p->nextPiece;
                }

                for (int i = 0; i < combinationSize; i++) {
                    Piece* target = piecesToDelete[i];
                    target->shapePrev->shapeNext = target->shapeNext;
                    target->shapeNext->shapePrev = target->shapePrev;
                    target->colorPrev->colorNext = target->colorNext;
                    target->colorNext->colorPrev = target->colorPrev;
                }

                for (int i = 0; i < combinationSize; i++) delete piecesToDelete[i];
                game->piecesCount -= combinationSize;

                currentPiece = game->head;
                beforeCurrent = nullptr;
                checked = 0;
                originalCount = game->piecesCount;
            }
            else
            {
                beforeCurrent = currentPiece;
                currentPiece = currentPiece->nextPiece;
                checked++;
            }
        }
    } while (matchFound && game->piecesCount >= 3);

    return game->score - initialScore;
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

void Game::colorShifting(Game *game, T_Color color, int){
    if (game->head == nullptr) return;
    Piece *currentColor = nullptr;
    Piece *temp = game->head;
    do {
        if(temp->color == color){
            currentColor = temp;
            break;
        }
        temp = temp->nextPiece;
    } while(temp != game->head);

    if(currentColor == nullptr) return;

    if(currentColor != currentColor->colorPrev){
        Piece * current = currentColor;
        do {
            switchingShapes(current);
            current = current->colorNext;
        } while(current != currentColor);

        Piece *heads[6] = { nullptr };
        Piece *tails[6] = { nullptr };
        current = game->head;
        do {
            int i = static_cast<int>(current->shape);
            if (heads[i] == nullptr) heads[i] = tails[i] = current;
            else {
                tails[i]->shapeNext = current;
                current->shapePrev = tails[i];
                tails[i] = current;
            }
            current = current->nextPiece;
        } while (current != game->head);

        for (int i = 0; i < 6; i++) {
            if (heads[i] != nullptr) {
                tails[i]->shapeNext = heads[i];
                heads[i]->shapePrev = tails[i];
            }
        }
    }
}

void Game::shapeShifting(Game *game, T_Shape shape, int){
    if (game->head == nullptr) return;
    Piece *currentShape = nullptr;
    Piece *temp = game->head;
    do {
        if(temp->shape == shape){
            currentShape = temp;
            break;
        }
        temp = temp->nextPiece;
    } while(temp != game->head);

    if(currentShape == nullptr) return;

    if(currentShape != currentShape->shapePrev){
        Piece * current = currentShape;
        do {
            switchingColors(current);
            current = current->shapeNext;
        } while(current != currentShape);

        Piece *heads[6] = { nullptr };
        Piece *tails[6] = { nullptr };
        current = game->head;
        do {
            int i = static_cast<int>(current->color);
            if (heads[i] == nullptr) heads[i] = tails[i] = current;
            else {
                tails[i]->colorNext = current;
                current->colorPrev = tails[i];
                tails[i] = current;
            }
            current = current->nextPiece;
        } while (current != game->head);

        for (int i = 0; i < 6; i++) {
            if (heads[i] != nullptr) {
                tails[i]->colorNext = heads[i];
                heads[i]->colorPrev = tails[i];
            }
        }
    }
}

#endif // GENERALHEADER_H_INCLUDED
