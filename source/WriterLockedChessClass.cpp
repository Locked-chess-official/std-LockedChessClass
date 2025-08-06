#include "WriterLockedChessClass.h"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

WriterLockedChessClass::WriterLockedChess::WriterLockedChess() : LockedChessClass::LockedChess() {
    WriterLockedChess::gameStart();
}

void WriterLockedChessClass::WriterLockedChess::gameStart() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    LockedChessClass::LockedChess::gameStart();
    boardString = returnGame();
    operationString = "";
    timeString = "";
    auto startTimeT = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&startTimeT), "%Y-%m-%d %H:%M:%S");
    startTime = ss.str();
    lastOperationTime = std::chrono::system_clock::now();
    inPeace = false;
    endOfGame = false;
    isProtected = false;
}

void WriterLockedChessClass::WriterLockedChess::operation(const Operation& op) {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    if (isProtected) {
        return;
    }
    if (getHasEnd()) {
        throw std::runtime_error("Game has already ended.");
    }
    if (inPeace && (op.isChessPieceOperation() ||
                    (op.getStringOperation() != "y" &&
                     op.getStringOperation() != "n"))) {
        throw std::runtime_error("Cannot perform operation during peace mode.");
    }
    if (op.isStringOperation()) {
        auto strOp = op.getStringOperation();
        if (strOp == "f" || strOp == "o" || strOp == "p" || strOp == "y" || strOp == "n") {
            handleOtherOperation(strOp);
            return;
        }
    }
    LockedChessClass::LockedChess::operation(op);
    if ((operationOppsite=="黑" && operationNumber != 1) || operationOppsite=="白" && operationNumber == 1){
        operationString += "b";
    } else {
        operationString += "w";
    }
    if (op.isChessPieceOperation()) {
        auto piece = op.getChessPieceOperation();
        operationString += dictMap.at(piece.getX()) + dictMap.at(piece.getY());
    } else {
        std::string dir = op.getStringOperation();
        if (operationNumber == 4) {
            auto part = splitString(dir, ',');
            if (part.size() != 2) {
                throw std::runtime_error("Invalid operation: " + dir);
            }
            operationString += dictMap.at(std::stoi(part[0])) + dictMap.at(std::stoi(part[1]));
        } else {
            operationString += dir;
        }
    }
    normalWrite();
    auto legalOps = LockedChess::legalOperation();
    if (legalOps.empty()) {
        endOfGame = true;
        operationString += (operationOppsite=="黑") ? "ww": "bw";
        endWrite();
    }
    
}
void WriterLockedChessClass::WriterLockedChess::loadsGame(const std::string& x, bool restart_game) {        
    std::unique_lock<std::recursive_mutex> locker(*lock);
    LockedChessClass::LockedChess::loadsGame(x, restart_game);
    boardString = returnGame();
    operationString = "";
    timeString = "";
    auto startTimeT = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&startTimeT), "%Y-%m-%d %H:%M:%S");
    startTime = ss.str();
    lastOperationTime = std::chrono::system_clock::now();
    inPeace = false;
    endOfGame = false;
    isProtected = false;
}

void WriterLockedChessClass::WriterLockedChess::setProtect() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    isProtected = true;
}

void WriterLockedChessClass::WriterLockedChess::setUnprotect() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    isProtected = false;
}

std::string WriterLockedChessClass::WriterLockedChess::getFinalResult() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    if (!getHasEnd()) {
        throw std::runtime_error("Game has not ended yet.");
    }
    json j;
    j["board"] = boardString;
    j["operation"] = operationString;
    j["time"] = timeString;
    return j.dump();
}

bool WriterLockedChessClass::WriterLockedChess::getPeace() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    return inPeace;
}

bool WriterLockedChessClass::WriterLockedChess::getProtect() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    return isProtected;
}

bool WriterLockedChessClass::WriterLockedChess::getHasEnd() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    if (legalOperation().empty()) {
        endOfGame = true;
    }
    return endOfGame;
}

void WriterLockedChessClass::WriterLockedChess::normalWrite() {
    operationString += "#";
    auto nextOperationTime = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(nextOperationTime - lastOperationTime);
    lastOperationTime = nextOperationTime;
    timeString += std::to_string(duration.count()) + "#";
    boardString += "#" + returnGame();
}

void WriterLockedChessClass::WriterLockedChess::endWrite() {
    auto endTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&endTime), "%Y-%m-%d %H:%M:%S");
    timeString +=  startTime + "$" + oss.str();
    endOfGame = true;
}

void WriterLockedChessClass::WriterLockedChess::handleOtherOperation(const std::string& x) {
    if (x == "f") { // 认输
        operationString += (operationOppsite == "黑")? "bf": "wf";
        endWrite();
    } else if (x == "o") { // 超时
        operationString += (operationOppsite == "黑")? "bo": "wo";
        endWrite();
    } else if (x == "p") { // 求和
        operationString += (operationOppsite == "黑")? "bp": "wp";
        normalWrite();
        inPeace = true;
    } else if (x == "y") { // 同意
        if (!inPeace) {
            throw std::runtime_error("Cannot agree to peace without requesting it first.");
        }
        operationString += "pp";
        endWrite();
        inPeace = false;
    } else if (x == "n") {
        if (!inPeace) {
            throw std::runtime_error("Cannot refuse peace without requesting it first.");
        }
        operationString += (operationOppsite == "黑")? "bn": "wn";
        normalWrite();
        inPeace = false;
    } else {
        throw std::runtime_error("Invalid operation: " + x);
    }
}

WriterLockedChessClass::WriterLockedChessClass() : game(new WriterLockedChess()){}
void WriterLockedChessClass::gameStart() {game->gameStart();}
std::vector<Operation> WriterLockedChessClass::legalOperation() {return game->legalOperation();}
void WriterLockedChessClass::operation(const Operation& op) {game->operation(op);}
std::string WriterLockedChessClass::returnGame() {return game->returnGame();}
void WriterLockedChessClass::loadsGame(const std::string& x, bool restart_game) {game->loadsGame(x, restart_game);}
std::vector<std::vector<Operation>> WriterLockedChessClass::calculateAllChains() {return game->calculateAllChains();}
bool WriterLockedChessClass::isEqualChains(std::vector<Operation>& chain1, std::vector<Operation>& chain2) {return game->isEqualChains(chain1, chain2);}
std::unordered_set<ChessPiece> WriterLockedChessClass::getGame() {return game->getGame();}
int WriterLockedChessClass::getOperationNumber() {return game->getOperationNumber();}
std::string WriterLockedChessClass::getOperationOppsite() {return game->getOperationOppsite();}
ChessPiece WriterLockedChessClass::getChooseChessLocate() {return game->getChooseChessLocate();}
std::string WriterLockedChessClass::getOperationLastDirection() {return game->getOperationLastDirection();}
bool WriterLockedChessClass::getHasOperated() {return game->getHasOperated();}
std::shared_ptr<std::recursive_mutex> WriterLockedChessClass::getLock() const {return game->getLock();}
void WriterLockedChessClass::setProtect() {game->setProtect();}
void WriterLockedChessClass::setUnprotect() {game->setUnprotect();}
std::string WriterLockedChessClass::getFinalResult() {return game->getFinalResult();}
bool WriterLockedChessClass::getProtect() {return game->getProtect();}
bool WriterLockedChessClass::getPeace() {return game->getPeace();}
bool WriterLockedChessClass::getHasEnd() {return game->getHasEnd();}