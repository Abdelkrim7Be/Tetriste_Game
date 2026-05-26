#include "pieceDeclaration.h"

Piece::Piece(T_Color clr, T_Shape spe, Piece *nextPiece, Piece *shapePrev, Piece *shapeNext, Piece *colorPrev, Piece *colorNext)
{
    this->color = clr;
    this->shape = spe;
    this->nextPiece = nextPiece;
    this->colorNext = colorNext;
    this->colorPrev = colorPrev;
    this->shapeNext = shapeNext;
    this->shapePrev = shapePrev;
}

Piece::~Piece()
{
}
