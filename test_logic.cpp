#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include "gameDeclaration.h"
#include "colorShape.h"

// Helper to verify consistency of all three list threads
void verifyConsistency(Game* game) {
    if (game->piecesCount == 0) {
        assert(game->head == nullptr);
        return;
    }
    assert(game->head != nullptr);

    Piece* current = game->head;
    int count = 0;
    do {
        count++;
        // Verify nextPiece circularity
        assert(current->nextPiece != nullptr);
        
        // Verify color circularity
        assert(current->colorNext != nullptr);
        assert(current->colorPrev != nullptr);
        assert(current->colorNext->colorPrev == current);
        assert(current->colorPrev->colorNext == current);
        assert(current->colorNext->color == current->color);

        // Verify shape circularity
        assert(current->shapeNext != nullptr);
        assert(current->shapePrev != nullptr);
        assert(current->shapeNext->shapePrev == current);
        assert(current->shapePrev->shapeNext == current);
        assert(current->shapeNext->shape == current->shape);

        current = current->nextPiece;
    } while (current != game->head && count < game->piecesCount + 10); // Safety break
    
    if (count != game->piecesCount) {
        std::cerr << "Consistency error: piecesCount=" << game->piecesCount << ", actual count=" << count << std::endl;
        assert(count == game->piecesCount);
    }
}

void test_similarSequenceTracker() {
    std::cout << "Running test_similarSequenceTracker..." << std::endl;

    Game* game = new Game(5, 5);
    
    // P1(Blue, Square) -> P2(Blue, Circle) -> P3(Blue, Star)
    Piece* p1 = new Piece(T_Color::BLUE, T_Shape::SQUARE, nullptr, nullptr, nullptr, nullptr, nullptr);
    Piece* p2 = new Piece(T_Color::BLUE, T_Shape::CIRCLE, nullptr, nullptr, nullptr, nullptr, nullptr);
    Piece* p3 = new Piece(T_Color::BLUE, T_Shape::STAR,   nullptr, nullptr, nullptr, nullptr, nullptr);

    game->insertPieceInRight(game, p1);
    game->insertPieceInRight(game, p2);
    game->insertPieceInRight(game, p3);

    verifyConsistency(game);

    // Test sequence starting at p1
    int seq = game->similarSequenceTracker(game, p1);
    assert(seq == 3);

    // Add a different color
    Piece* p4 = new Piece(T_Color::RED, T_Shape::SQUARE, nullptr, nullptr, nullptr, nullptr, nullptr);
    game->insertPieceInRight(game, p4);

    verifyConsistency(game);

    seq = game->similarSequenceTracker(game, p1);
    assert(seq == 3);

    // Test Shape sequence (p4 is Square, p1 is Square)
    seq = game->similarSequenceTracker(game, p4);
    // p4(Red, Sq) -> p1(Blue, Sq) -> p2(Blue, Circ)
    assert(seq == 2);

    delete game;
    std::cout << "test_similarSequenceTracker passed!" << std::endl;
}

void test_colorShifting() {
    std::cout << "Running test_colorShifting..." << std::endl;
    Game* game = new Game(5, 5);
    
    Piece* p1 = new Piece(T_Color::BLUE, T_Shape::SQUARE, nullptr, nullptr, nullptr, nullptr, nullptr);
    Piece* p2 = new Piece(T_Color::BLUE, T_Shape::CIRCLE, nullptr, nullptr, nullptr, nullptr, nullptr);
    Piece* p3 = new Piece(T_Color::BLUE, T_Shape::STAR,   nullptr, nullptr, nullptr, nullptr, nullptr);
    
    game->insertPieceInRight(game, p1);
    game->insertPieceInRight(game, p2);
    game->insertPieceInRight(game, p3);
    
    verifyConsistency(game);
    
    game->colorShifting(game, T_Color::BLUE, 5);
    
    // Expected: Cyclic shift (S1, S2, S3) -> (S2, S3, S1)
    assert(p1->shape == T_Shape::CIRCLE);
    assert(p2->shape == T_Shape::STAR);
    assert(p3->shape == T_Shape::SQUARE);
    
    verifyConsistency(game);
    
    delete game;
    std::cout << "test_colorShifting passed!" << std::endl;
}

void test_shapeShifting() {
    std::cout << "Running test_shapeShifting..." << std::endl;
    Game* game = new Game(5, 5);
    
    Piece* p1 = new Piece(T_Color::RED,   T_Shape::SQUARE, nullptr, nullptr, nullptr, nullptr, nullptr);
    Piece* p2 = new Piece(T_Color::BLUE,  T_Shape::SQUARE, nullptr, nullptr, nullptr, nullptr, nullptr);
    Piece* p3 = new Piece(T_Color::GREEN, T_Shape::SQUARE, nullptr, nullptr, nullptr, nullptr, nullptr);
    
    game->insertPieceInRight(game, p1);
    game->insertPieceInRight(game, p2);
    game->insertPieceInRight(game, p3);
    
    verifyConsistency(game);
    
    game->shapeShifting(game, T_Shape::SQUARE, 5);
    
    // Expected: Cyclic shift (C1, C2, C3) -> (C2, C3, C1)
    assert(p1->color == T_Color::BLUE);
    assert(p2->color == T_Color::GREEN);
    assert(p3->color == T_Color::RED);
    
    verifyConsistency(game);
    
    delete game;
    std::cout << "test_shapeShifting passed!" << std::endl;
}

void test_updateGame_matching() {
    std::cout << "Running test_updateGame_matching..." << std::endl;
    Game* game = new Game(5, 5);
    
    // Create 3 Blue pieces in a row
    Piece* p1 = new Piece(T_Color::BLUE, T_Shape::SQUARE, nullptr, nullptr, nullptr, nullptr, nullptr);
    Piece* p2 = new Piece(T_Color::BLUE, T_Shape::CIRCLE, nullptr, nullptr, nullptr, nullptr, nullptr);
    Piece* p3 = new Piece(T_Color::BLUE, T_Shape::STAR,   nullptr, nullptr, nullptr, nullptr, nullptr);
    
    game->insertPieceInRight(game, p1);
    game->insertPieceInRight(game, p2);
    game->insertPieceInRight(game, p3);
    
    // Add one extra piece to prevent win state deletion of entire list (easier to test score)
    Piece* p4 = new Piece(T_Color::RED, T_Shape::SQUARE, nullptr, nullptr, nullptr, nullptr, nullptr);
    game->insertPieceInRight(game, p4);

    verifyConsistency(game);
    assert(game->piecesCount == 4);
    assert(game->score == 0);

    int scoreChange = game->updateGame(game);
    
    std::cout << "Score change: " << scoreChange << ", new piecesCount: " << game->piecesCount << std::endl;
    
    assert(scoreChange > 0);
    assert(game->piecesCount == 1); // Only Red piece remains
    assert(game->head == p4);
    
    verifyConsistency(game);
    
    delete game;
    std::cout << "test_updateGame_matching passed!" << std::endl;
}

int main() {
    test_similarSequenceTracker();
    test_colorShifting();
    test_shapeShifting();
    test_updateGame_matching();
    std::cout << "All logic tests passed!" << std::endl;
    return 0;
}
