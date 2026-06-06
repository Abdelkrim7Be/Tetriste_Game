#include "gameDeclaration.h"
#include <cmath>
#include <iostream>
#include <unordered_set>
#include <vector>

Game::Game(int colorIndex, int shapeIndex)
{
    this->head = nullptr;
    this->score = 0;
    this->globalComboMultiplier = 1;
    this->piecesCount = 0;
    this->colorIndex = colorIndex;
    this->shapeIndex = shapeIndex;
    this->nodesPurgedInLastUpdate = 0;
}

Game::~Game()
{
    std::cout << "DEBUG: Destroying Game, piecesCount=" << piecesCount << std::endl;
    // Circular references handled by vector ownership clearing
    head = nullptr;
    ownedPieces.clear();
    std::cout << "DEBUG: Game destroyed successfully" << std::endl;
}

Game* Game::initializeGame(int colorIndex, int shapeIndex)
{
    Game *newGame = new Game(colorIndex, shapeIndex);
    Piece *newPiece = newGame->drawPiece(colorIndex, shapeIndex);
    
    // Core structural invariant: even with 1 piece, it points to itself
    newPiece->nextPiece = newPiece;
    newPiece->shapePrev = newPiece;
    newPiece->shapeNext = newPiece;
    newPiece->colorPrev = newPiece;
    newPiece->colorNext = newPiece;

    newGame->head = newPiece;
    newGame->piecesCount = 1;

    return newGame;
}

Piece* Game::drawPiece(int colorIndex, int shapeIndex)
{
    // Important: Ownership is held by the vector. We return a weak raw pointer for use.
    ownedPieces.emplace_back(new Piece(static_cast<T_Color>(colorIndex), static_cast<T_Shape>(shapeIndex), nullptr, nullptr, nullptr, nullptr, nullptr));
    return ownedPieces.back().get();
}

Piece* Game::retrieveTail(Game *game)
{
    if (game->head == nullptr) return nullptr;
    Piece *current = game->head;
    while (current->nextPiece != game->head)
    {
        current = current->nextPiece;
    }
    return current;
}

bool Game::insertPieceInRight(Game *game, Piece *newPiece)
{
    if (game == nullptr || newPiece == nullptr) return false;

    if (game->piecesCount == 0) game->head = nullptr;

    Piece *tail = retrieveTail(game);
    if (tail == nullptr) {
        game->head = newPiece;
        newPiece->nextPiece = newPiece;
        newPiece->colorPrev = newPiece->colorNext = newPiece;
        newPiece->shapePrev = newPiece->shapeNext = newPiece;
    } else {
        tail->nextPiece = newPiece;
        newPiece->nextPiece = game->head;
    }
    game->updateColorAfterAdding(newPiece);
    game->updateShapeAfterAdding(newPiece);
    game->piecesCount++;
    return true;
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

bool Game::insertPieceInLeft(Game *game, Piece *newPiece)
{
    if (game->insertPieceInRight(game, newPiece)) {
        game->head = newPiece;
        return true;
    }
    return false;
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

static Piece *findRingPred(Game *game, Piece *piece) {
    if (game == nullptr || game->head == nullptr || piece == nullptr || game->piecesCount <= 1)
        return nullptr;
    Piece *p = game->head;
    do {
        if (p->nextPiece == piece) return p;
        p = p->nextPiece;
    } while (p != game->head);
    return nullptr;
}

static int consecutiveRun(Game *game, Piece *piece, bool byColor) {
    if (game == nullptr || piece == nullptr) return 1;
    int forward = 1;
    Piece *fwd = piece->nextPiece;
    for (int i = 1; i < game->piecesCount && fwd != piece; ++i) {
        bool match = byColor ? (fwd->color == piece->color) : (fwd->shape == piece->shape);
        if (!match) break;
        forward++;
        fwd = fwd->nextPiece;
    }
    int backward = 0;
    Piece *back = findRingPred(game, piece);
    for (int i = 0; i < game->piecesCount - 1 && back != nullptr && back != piece; ++i) {
        bool match = byColor ? (back->color == piece->color) : (back->shape == piece->shape);
        if (!match) break;
        backward++;
        back = findRingPred(game, back);
    }
    int total = forward + backward;
    return std::min(total, game->piecesCount);
}

int Game::similarSequenceTracker(Game *game, Piece *newPiece)
{
    int colorSequence = consecutiveRun(game, newPiece, true);
    int shapeSequence = consecutiveRun(game, newPiece, false);
    return (colorSequence > shapeSequence) ? colorSequence : shapeSequence;
}

static void buildVirtualRing(Game *game, Piece *incoming, bool insertLeft,
                             std::vector<Piece *> &ring, std::vector<bool> &isIncoming) {
    ring.clear();
    isIncoming.clear();
    if (game == nullptr || incoming == nullptr || game->piecesCount == 0) return;

    if (insertLeft) {
        ring.push_back(incoming);
        isIncoming.push_back(true);
    }
    Piece *p = game->head;
    for (int i = 0; i < game->piecesCount; ++i) {
        ring.push_back(p);
        isIncoming.push_back(false);
        p = p->nextPiece;
    }
    if (!insertLeft) {
        ring.push_back(incoming);
        isIncoming.push_back(true);
    }
}

static int virtualRunAt(const std::vector<Piece *> &ring, const std::vector<bool> &isIncoming,
                        int index, bool byColor) {
    if (ring.empty() || index < 0 || index >= (int)ring.size()) return 1;
    int m = (int)ring.size();
    T_Color c0 = ring[index]->color;
    T_Shape s0 = ring[index]->shape;
    (void)isIncoming;

    int forward = 1;
    for (int i = 1; i < m; ++i) {
        int j = (index + i) % m;
        bool match = byColor ? (ring[j]->color == c0) : (ring[j]->shape == s0);
        if (!match) break;
        forward++;
    }
    int backward = 0;
    for (int i = 1; i < m; ++i) {
        int j = (index - i + m) % m;
        bool match = byColor ? (ring[j]->color == c0) : (ring[j]->shape == s0);
        if (!match) break;
        backward++;
    }
    return std::min(forward + backward, m);
}

static int virtualComboAt(const std::vector<Piece *> &ring, const std::vector<bool> &isIncoming, int index) {
    int colorRun = virtualRunAt(ring, isIncoming, index, true);
    int shapeRun = virtualRunAt(ring, isIncoming, index, false);
    return (colorRun > shapeRun) ? colorRun : shapeRun;
}

/** Board nodes in the consecutive arc on the winning attribute (color or shape). */
static void collectWinningRunOnRing(const std::vector<Piece *> &ring, const std::vector<bool> &isIncoming,
                                    int anchorIndex, std::unordered_set<Piece *> &out) {
    int m = (int)ring.size();
    if (anchorIndex < 0 || anchorIndex >= m) return;

    int colorRun = virtualRunAt(ring, isIncoming, anchorIndex, true);
    int shapeRun = virtualRunAt(ring, isIncoming, anchorIndex, false);
    int combo = (colorRun > shapeRun) ? colorRun : shapeRun;
    if (combo < 3) return;

    bool byColor = colorRun > shapeRun;
    T_Color c0 = ring[anchorIndex]->color;
    T_Shape s0 = ring[anchorIndex]->shape;

    for (int i = 0; i < m; ++i) {
        int j = (anchorIndex + i) % m;
        if (i > 0) {
            bool match = byColor ? (ring[j]->color == c0) : (ring[j]->shape == s0);
            if (!match) break;
        }
        if (!isIncoming[j]) out.insert(ring[j]);
    }
    for (int i = 1; i < m; ++i) {
        int j = (anchorIndex - i + m) % m;
        bool match = byColor ? (ring[j]->color == c0) : (ring[j]->shape == s0);
        if (!match) break;
        if (!isIncoming[j]) out.insert(ring[j]);
    }
}

void Game::collectMatchPreview(Game *game, Piece *incoming, std::vector<Piece *> &out) {
    out.clear();
    if (game == nullptr || incoming == nullptr || game->piecesCount < 2) return;

    std::unordered_set<Piece *> marked;

    for (bool insertLeft : {true, false}) {
        std::vector<Piece *> ring;
        std::vector<bool> isIncoming;
        buildVirtualRing(game, incoming, insertLeft, ring, isIncoming);
        if (ring.empty()) continue;

        int scanCount = insertLeft ? 1 : game->piecesCount;
        for (int si = 0; si < scanCount; ++si) {
            int colorRun = virtualRunAt(ring, isIncoming, si, true);
            int shapeRun = virtualRunAt(ring, isIncoming, si, false);
            int combo = (colorRun > shapeRun) ? colorRun : shapeRun;
            if (combo >= 3) {
                collectWinningRunOnRing(ring, isIncoming, si, marked);
                break;
            }
        }
    }

    out.assign(marked.begin(), marked.end());
}

int Game::updateGame(Game *game)
{
    if (game->piecesCount < 3) return 0;
    int initialScore = game->score;
    int combo = 0;
    game->nodesPurgedInLastUpdate = 0;

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
                game->score += static_cast<int>(std::pow(combinationSize, combo)) * game->globalComboMultiplier;
                game->nodesPurgedInLastUpdate += combinationSize;

                if (game->piecesCount == combinationSize)
                {
                    std::cout << "DEBUG: Winning state reached, deleting all pieces" << std::endl;
                    std::vector<Piece*> toDel;
                    Piece* p = game->head;
                    for (int i = 0; i < combinationSize; i++) {
                        toDel.push_back(p);
                        p = p->nextPiece;
                    }
                    game->piecesCount = 0;
                    game->head = nullptr;
                    for (auto target : toDel) {
                        for (auto it = game->ownedPieces.begin(); it != game->ownedPieces.end(); ++it) {
                            if (it->get() == target) {
                                game->ownedPieces.erase(it);
                                break;
                            }
                        }
                    }
                    std::cout << "DEBUG: Winning state cleanup done" << std::endl;
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

                std::vector<Piece*> piecesToDelete;
                Piece* p = seqStart;
                for (int i = 0; i < combinationSize; i++) {
                    piecesToDelete.push_back(p);
                    p = p->nextPiece;
                }

                for (auto target : piecesToDelete) {
                    target->shapePrev->shapeNext = target->shapeNext;
                    target->shapeNext->shapePrev = target->shapePrev;
                    target->colorPrev->colorNext = target->colorNext;
                    target->colorNext->colorPrev = target->colorPrev;
                }

                for (auto target : piecesToDelete) {
                    for (auto it = game->ownedPieces.begin(); it != game->ownedPieces.end(); ++it) {
                        if (it->get() == target) {
                            game->ownedPieces.erase(it);
                            break;
                        }
                    }
                }
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
}

void switchingColors(Piece *piece){
    T_Color tempColor = piece->color;
    piece->color = piece->shapeNext->color;
    piece->shapeNext->color = tempColor;
}

void Game::colorShifting(Game *game, T_Color color, int) {
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
        while(current->colorNext != currentColor){
            switchingShapes(current);
            current = current->colorNext;
        }

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

void Game::shapeShifting(Game *game, T_Shape shape, int) {
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
        while(current->shapeNext != currentShape){
            switchingColors(current);
            current = current->shapeNext;
        }

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
