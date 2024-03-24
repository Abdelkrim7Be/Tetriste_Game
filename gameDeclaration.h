#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED
#include "pieceDeclaration.h"
#include "colorShape.h"

using namespace std;

class Game
{
public:
    Piece *head;
    int score;

    int piecesCount;
    int colorCount;
    int shapeCount;

    Game(int colorCount, int shapeCount);
    ~Game();

    Game* initializeGame(int colorCount, int shapeCount);
    Piece *drawPiece(int colorCount, int shapeCount);
    string display(T_Color color, T_Shape shape);
    Piece* retrieveTail(Game* game);

    void insertPieceInRight(Game *game, Piece *piece);
    void insertPieceInLeft(Game *game, Piece *piece);
    void updateShapeAfterAdding(Piece* piece);
    void updateColorAfterAdding(Piece* piece);
    int similarSequenceTracker(Game* game, Piece *newPiece);
    int updateGame(Game* game);
    void removePieceFromRelationships(Piece* piece);

    void colorShifting(Game *game, T_Color color);
    void shapeShifting(Game *game, T_Shape shape);

    void nextPieces(Game *game, Piece *nextPieces);
    void updateScores(int score, string name);
};

#endif // GAME_H_INCLUDED
