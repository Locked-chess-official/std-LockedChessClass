#ifndef OPERATION_H
#define OPERATION_H

#include <string>
#include "chesspiece.h"


class Operation {
private:
    std::string stringOperation;
    bool isString = false;
    ChessPiece pieceOperation;
    bool isChessPiece = false;
    
public:
     Operation(const std::string& stringOperation);
     Operation(const ChessPiece& piece);
    bool  isStringOperation() const;
    bool  isChessPieceOperation() const;
    std::string  getStringOperation() const;
    ChessPiece  getChessPieceOperation() const;
    bool  operator==(const Operation& other) const;
    bool  operator==(const std::string& other) const;
    bool  operator==(const ChessPiece& other) const;
    Operation  operator=(const Operation& other);
};

namespace std {
    template<> 
    struct hash<Operation> {
        inline size_t operator()(const Operation& op) const {
            if (op.isStringOperation()) {
                return hash<std::string>()(op.getStringOperation());
            } else if (op.isChessPieceOperation()) {
                return hash<ChessPiece>()(op.getChessPieceOperation());
            }
            return 0;
        }
    };
}

#endif // OPERATION_H