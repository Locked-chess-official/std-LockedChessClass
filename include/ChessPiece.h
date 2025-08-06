#ifndef CHESSPIECE_H
#define CHESSPIECE_H

#include <string>
#include <functional>


class ChessPiece {
public:
    ChessPiece(int x, int y, std::string color);
    int  getX() const;
    int  getY() const;
    std::string  getColor() const;
    bool  operator==(const ChessPiece& other) const;
    bool  operator<(const ChessPiece& other) const;
    ChessPiece  operator=(const ChessPiece& other);
    
private:
    int x;
    int y;
    std::string color;
};

namespace std {
    template<> 
    struct hash<ChessPiece> {
        inline size_t operator()(const ChessPiece& piece) const {
            return hash<int>()(piece.getX()) ^ 
                   hash<int>()(piece.getY()) ^ 
                   hash<std::string>()(piece.getColor());
        }
    };
}

#endif // CHESSPIECE_H