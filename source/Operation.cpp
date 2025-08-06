#include "Operation.h"

Operation::Operation(const std::string& stringOperation)
    : stringOperation(stringOperation), isString(true) , pieceOperation(-1, -1, "无"){}
Operation::Operation(const ChessPiece& piece)
    : pieceOperation(piece), isChessPiece(true), stringOperation("") {}
bool Operation::isStringOperation() const { return isString; }
bool Operation::isChessPieceOperation() const { return isChessPiece; }
std::string Operation::getStringOperation() const { return stringOperation; }
ChessPiece Operation::getChessPieceOperation() const { return pieceOperation; } 
bool Operation::operator==(const Operation& other) const {
    if (isString && other.isString) {
        return stringOperation == other.stringOperation;
    } else if (isChessPiece && other.isChessPiece) {
        return pieceOperation == other.pieceOperation;
    }
    return false;
}

bool Operation::operator==(const std::string& other) const {
    return isString && stringOperation == other;
}

bool Operation::operator==(const ChessPiece& other) const {
    return isChessPiece && pieceOperation == other;
}

Operation Operation::operator=(const Operation& other) {
    if (this != &other) {
        stringOperation = other.stringOperation;
        isString = other.isString;
        pieceOperation = other.pieceOperation;
        isChessPiece = other.isChessPiece;
    }
    return *this;
}