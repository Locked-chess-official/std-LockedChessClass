#include "LockedChessRobotClass.h"
#include "LockedChessClass.h"
#include <stdexcept>
#include <algorithm>

LockedChessRobotClass::LockedChessRobot::LockedChessRobot() : game(new LockedChessClass()) {}
void LockedChessRobotClass::LockedChessRobot::setGame(std::shared_ptr<LockedChessAllInterface> game) {this->game = game;}
bool LockedChessRobotClass::LockedChessRobot::dueToFail(std::vector<Operation>& chain) const {
    if (!game) {return true;}
    try {
        LockedChessClass::LockedChess temp;
        temp.loadsGame(game->returnGame());
        for (const auto& op : chain) {
            temp.operation(op);
        }
        std::vector<int> allX, allY;
        for (const auto& piece : temp.getGame()) {
            if (piece.getColor() == game->getOperationOppsite()) {
                allX.push_back(piece.getX());
                allY.push_back(piece.getY());
            }
        }
        int minusX = *std::max_element(allX.begin(), allX.end()) - *std::min_element(allX.begin(), allX.end());
        int minusY = *std::max_element(allY.begin(), allY.end()) - *std::min_element(allY.begin(), allY.end());
        return minusX >= 11 || minusY >= 11;
    } catch (...) {
        return true;
    }
}

bool LockedChessRobotClass::LockedChessRobot::justWin(std::vector<Operation>& chain) const {
    if (!game) {return false;} 
    try {
        LockedChessClass::LockedChess temp;
        temp.loadsGame(game->returnGame());
        for (const auto& op : chain) {
            temp.operation(op);
        }
        auto newChains = temp.calculateAllChains();
        LockedChessClass::LockedChess newTemp;
        for (const auto& newChain : newChains) {
            newTemp.loadsGame(temp.returnGame());
            for (const auto& op : newChain) {
                try {
                    newTemp.operation(op);
                } catch (...) {
                    goto end;
                }
            }
            if (newTemp.legalOperation().empty()) {
                return true; // Found a winning chain
            }
            end:{}
        }
        return false; // No winning chain found
    } catch (...) {
        return false;
    }
}

std::vector<Operation> LockedChessRobotClass::LockedChessRobot::getOperation() {
    std::unique_lock<std::recursive_mutex> locker(*game->getLock());
    auto allChains = game->calculateAllChains();
    if (allChains.empty()) {
        return {};
    }
    LockedChessClass::LockedChess temp;
    auto allChainsCopy = allChains;
    for (auto& chain : allChainsCopy) {
        temp.loadsGame(game->returnGame());
        for (const auto& op : chain) {
            try {
                temp.operation(op);
            } catch (...) {
                allChains.erase(
                    std::remove<std::vector<std::vector<Operation>>::iterator, std::vector<Operation>>(
                        allChains.begin(), 
                        allChains.end(), 
                        chain
                    ),
                    allChains.end()
                );
                goto end;
            }
        }
        if (temp.legalOperation().empty()) {
            return chain;
        }
        end:{}
    }
    std::vector<Operation> returnChain;
    do {
        returnChain = removeRandomElement(allChains);
    } while (dueToFail(returnChain) && !allChains.empty());
    return returnChain;
}

std::vector<Operation> LockedChessRobotClass::LockedChessRobot::getBetterOperation(bool useBetter) {
    std::unique_lock<std::recursive_mutex> locker(*game->getLock());
    auto allChains = game->calculateAllChains();
    if (allChains.empty()) {
        return {};
    }
    LockedChessClass::LockedChess temp;
    auto allChainsCopy = allChains;
    for (auto& chain : allChainsCopy) {
        temp.loadsGame(game->returnGame());
        for (const auto& op : chain) {
            try {
                temp.operation(op);
            } catch (...) {
                allChains.erase(
                    std::remove<std::vector<std::vector<Operation>>::iterator, std::vector<Operation>>(
                        allChains.begin(), 
                        allChains.end(), 
                        chain
                    ),
                    allChains.end()
                );
                goto end;
            }
        }
        if (temp.legalOperation().empty()) {
            return chain;
        }
        end:{}
    }
    std::vector<Operation> returnChain;
    bool needContinue;
    do {
        if (allChains.empty()) {
            break;
        }
        returnChain = removeRandomElement(allChains);
        if (justWin(returnChain)) {
            if (useBetter) {
                needContinue = true;
                continue;
            } else {
                return returnChain;
            }
        } else {
            if (!useBetter) {
                needContinue = true;
                continue;
            } else {
                return returnChain;
            }
        }
    } while (needContinue || (dueToFail(returnChain) && !allChains.empty()));
    return returnChain;
}

LockedChessRobotClass::LockedChessRobotClass() : robot(new LockedChessRobot()) {}
void LockedChessRobotClass::setGame(std::shared_ptr<LockedChessAllInterface> game) {robot->setGame(game);}
std::vector<Operation> LockedChessRobotClass::getOperation() {return robot->getOperation();}
std::vector<Operation> LockedChessRobotClass::getBetterOperation(bool useBetter) {return robot->getBetterOperation(useBetter);}
