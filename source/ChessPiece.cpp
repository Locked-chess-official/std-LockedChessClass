#include "ChessPiece.h"

ChessPiece::ChessPiece(int x, int y, std::string color) 
    : x(x), y(y), color(std::move(color)) {}

int ChessPiece::getX() const {
    return x;
}

int ChessPiece::getY() const {
    return y;
}

std::string ChessPiece::getColor() const {
    return color;
}

bool ChessPiece::operator==(const ChessPiece& other) const {
    return x == other.x && y == other.y && color == other.color;
}

bool ChessPiece::operator<(const ChessPiece& other) const {
    return std::tie(x, y, color) < std::tie(other.x, other.y, other.color);
}

ChessPiece ChessPiece::operator=(const ChessPiece& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        color = other.color;
    }
    return *this;
}