#ifndef PIECE_H_INCLUDED
#define PIECE_H_INCLUDED

#include "colorShape.h"
#include <string>

using namespace std;

class Piece
{
public:
    T_Color color;
    T_Shape shape;
    Piece* nextPiece;
    Piece* shapePrev;
    Piece* shapeNext;
    Piece* colorPrev;
    Piece* colorNext;
    string displayString;

    Piece(T_Color color, T_Shape shape, Piece* nextPiece, Piece* shapePrev, Piece* shapeNext, Piece* colorPrev, Piece* colorNext);
    ~Piece();

    string displayPiece();

};

#endif // PIECE_H_INCLUDED
