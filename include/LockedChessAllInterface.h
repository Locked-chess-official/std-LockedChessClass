#ifndef LOCKEDCHESSALLINTERFACE_H
#define LOCKEDCHESSALLINTERFACE_H

#include <vector>
#include <memory>
#include <mutex>
#include <unordered_set>
#include "ChessPiece.h"
#include "Operation.h"

class LockedChessAllInterface {
public:
    virtual ~LockedChessAllInterface() = default;
    virtual void gameStart() = 0;
    virtual std::vector<Operation> legalOperation() = 0;
    virtual void operation(const Operation& op) = 0;
    virtual std::string returnGame() = 0;
    virtual void loadsGame(const std::string& x, bool restart_game = false) = 0;
    virtual std::vector<std::vector<Operation>> calculateAllChains() = 0;
    virtual bool isEqualChains(std::vector<Operation>& chain1, std::vector<Operation>& chain2) = 0;
    virtual std::unordered_set<ChessPiece> getGame() = 0;
    virtual int getOperationNumber() = 0;
    virtual std::string getOperationOppsite() = 0;
    virtual ChessPiece getChooseChessLocate() = 0;
    virtual std::string getOperationLastDirection() = 0;
    virtual bool getHasOperated() = 0;
    virtual std::shared_ptr<std::recursive_mutex> getLock() const = 0;
};

class WriterLockedChessAllInterface: public LockedChessAllInterface {
public:
    virtual ~WriterLockedChessAllInterface() = default;
    virtual void setProtect() = 0;
    virtual void setUnprotect() = 0;
    virtual std::string getFinalResult() = 0;
    virtual bool getProtect() = 0;
    virtual bool getPeace() = 0;
    virtual bool getHasEnd() = 0;
};

class LockedChessRobotAllInterface {
public:
    virtual ~LockedChessRobotAllInterface() = default;
    virtual void setGame(std::shared_ptr<LockedChessAllInterface>) = 0;
    virtual std::vector<Operation> getOperation() = 0;
    virtual std::vector<Operation> getBetterOperation(bool useBetter) = 0;
};

inline std::vector<std::string> splitString(const std::string& str, char delimiter);
std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    for (char ch : str) {
        if (ch == delimiter) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += ch;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

#endif // LOCKEDCHESSALLINTERFACE_H