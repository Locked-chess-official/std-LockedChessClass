#include "LockedChessClass.h"
#include "CacheCleaner.h"
#include <algorithm>
#include <nlohmann/json.hpp>

std::map<std::string, std::vector<Operation>> LockedChessClass::LockedChess::legalOperationCache;
std::map<std::string, int> LockedChessClass::LockedChess::legalOperationCacheCount;
std::map<std::string, std::chrono::system_clock::time_point> LockedChessClass::LockedChess::legalOperationCacheTime;
std::shared_mutex LockedChessClass::LockedChess::legalOperationCacheLock;
std::shared_mutex LockedChessClass::LockedChess::legalOperationCacheCountLock;
std::map<std::string, std::vector<std::vector<Operation>>> LockedChessClass::LockedChess::chainCache;
std::map<std::string, int> LockedChessClass::LockedChess::chainCacheCount;
std::map<std::string, std::chrono::system_clock::time_point> LockedChessClass::LockedChess::chainCacheTime;
std::shared_mutex LockedChessClass::LockedChess::chainCacheLock;
std::shared_mutex LockedChessClass::LockedChess::chainCacheCountLock;

using json = nlohmann::json;

LockedChessClass::LockedChess::LockedChess(): chooseChessLocate(-1, -1, "无"), lock(new std::recursive_mutex()) {
    gameStart();
}

void LockedChessClass::LockedChess::gameStart() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    game = {
        {8, 5, "白"},
        {5, 5, "黑"},
        {4, 4, "黑"},
        {9, 9, "黑"},
        {5, 8, "白"},
        {8, 8, "黑"},
        {9, 4, "白"},
        {4, 9, "白"}
    };
    operationNumber = 1;
    operationOppsite = "黑";

    allOperation = {
        {"u"},
        {"d"},
        {"l"},
        {"r"}
    };
    chooseChessLocate = {-1, -1, "无"};
    operationLastDirection = "无";
    hasOperated = false;
}

std::vector<Operation> LockedChessClass::LockedChess::legalOperation() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    std::vector<std::string> checkResult = checkGame();
    if (!(checkResult[0] == "000")) {
        throw std::runtime_error("Game is not valid: " + checkResult[0] + " " + checkResult[1]);
    }
    std::string stateKey = returnGameWithOutAllOperation();
    {
        std::shared_lock<std::shared_mutex> cacheLocker(legalOperationCacheLock);
        auto it = legalOperationCache.find(stateKey);
        if (it != legalOperationCache.end()) {
            {
                std::unique_lock<std::shared_mutex> countLocker(legalOperationCacheCountLock);
                legalOperationCacheCount[stateKey]++;
                legalOperationCacheTime[stateKey] = std::chrono::system_clock::now();
            }
            allOperation = it->second;
            return allOperation;
        }
    }
    allOperation.clear();
    std::vector<std::string> operationOrder = {"u", "d", "l", "r"};
    switch (operationNumber) {
        case 1:
        case 2: {
            std::map<std::string, bool> bloa, wloa;
            auto legalOperationAllResult = legalOperationAll();
            bloa = legalOperationAllResult[0];
            wloa = legalOperationAllResult[1];
            if (operationOppsite == "黑") {
                for (auto &i : bloa) {
                    if (i.second && (operationNumber == 1 || (
                        operationNumber == 2 && (
                            ((i.first == "u" || i.first == "d") && operationLastDirection == "左右") ||
                            ((i.first == "l" || i.first == "r") && operationLastDirection == "上下")
                        )))) {
                        allOperation.push_back(Operation(i.first));
                    }
                }
            } else {
                for (auto &i : wloa) {
                    if (i.second && (operationNumber == 1 || (
                        operationNumber == 2 && (
                            ((i.first == "u" || i.first == "d") && operationLastDirection == "左右") ||
                            ((i.first == "l" || i.first == "r") && operationLastDirection == "上下")
                        )))) {
                        allOperation.push_back(Operation(i.first));
                    }
                }
            } 
            std::sort(allOperation.begin(), allOperation.end(), [operationOrder](const Operation &a, const Operation &b) {
                return std::find(operationOrder.begin(), operationOrder.end(), a.getStringOperation()) <
                       std::find(operationOrder.begin(), operationOrder.end(), b.getStringOperation());
                }
            );
            break;
        }
        case 4:
        case 5: {
            std::map<ChessPiece, std::map<std::string, bool>> bloo, wloo;
            auto legalOperationOneResult = legalOperationOne();
            bloo = legalOperationOneResult[0];
            wloo = legalOperationOneResult[1];
            if (operationOppsite == "黑") {
                for (auto &i : bloo[chooseChessLocate]) {
                    if (i.second && (operationNumber == 4 || (
                        operationNumber == 5 && (
                            ((i.first == "u" || i.first == "d") && operationLastDirection == "左右") ||
                            ((i.first == "l" || i.first == "r") && operationLastDirection == "上下")
                        )))) {
                        allOperation.push_back(i.first);
                    }                    
                }
            } else {
                for (auto &i : wloo[chooseChessLocate]) {
                    if (i.second && (operationNumber == 4 || (
                        operationNumber == 5 && (
                            ((i.first == "u" || i.first == "d") && operationLastDirection == "左右") ||
                            ((i.first == "l" || i.first == "r") && operationLastDirection == "上下")
                        )))) {
                        allOperation.push_back(i.first);
                    }
                }
            }
            std::sort(allOperation.begin(), allOperation.end(), [operationOrder](const Operation &a, const Operation &b) {
                return std::find(operationOrder.begin(), operationOrder.end(), a.getStringOperation()) <
                       std::find(operationOrder.begin(), operationOrder.end(), b.getStringOperation());
                }
            );
            break;
        }
        case 3:{
            for (const auto& i: game) {
                if (i.getColor() == operationOppsite) {
                    allOperation.push_back(i);
                }
            }
            std::sort(allOperation.begin(), allOperation.end(), [](const Operation &a, const Operation &b) {
                return a.getChessPieceOperation().getX() < b.getChessPieceOperation().getX() ||
                       (a.getChessPieceOperation().getX() == b.getChessPieceOperation().getX() &&
                        a.getChessPieceOperation().getY() < b.getChessPieceOperation().getY());
                }
            );
            break;
        }
        default:
            throw std::runtime_error("Invalid operation number: " + std::to_string(operationNumber));
    }
    {
        std::unique_lock<std::shared_mutex> cacheLocker(legalOperationCacheLock);
        legalOperationCache[stateKey] = allOperation;
        std::unique_lock<std::shared_mutex> countLocker(legalOperationCacheCountLock);
        legalOperationCacheCount[stateKey] = 1;
        legalOperationCacheTime[stateKey] = std::chrono::system_clock::now();
    }
    return allOperation;
}

std::vector<std::string> LockedChessClass::LockedChess::checkGame() {
    try {
        if (game.empty() || operationNumber < 1 || operationNumber > 5 ||
            (operationOppsite != "黑" && operationOppsite != "白")) {
            return {"001","game has not init"};
        }
        std::set<std::string> allLocate;
        int numberBlack = 0, numberWhite = 0;
        for (const auto &i : game) {
            if (i.getX() < 0 || i.getY() < 0 || i.getX()>12 || i.getY() > 12 ||(i.getColor() != "黑" && i.getColor() != "白")) {
                return {"003","game has invalid chess"};
            }
            std::string locate = std::to_string(i.getX()) + "," + std::to_string(i.getY());
            if (allLocate.find(locate) != allLocate.end()) {
                return {"003","game has repeat locate"};
            }
            allLocate.insert(locate);
            if (i.getColor() == "黑") {
                numberBlack++;
            } else {
                numberWhite++;
            }
        }
        if (numberBlack < 3 || numberWhite < 3) {
            return {"003","game has not enough chess"};
        }
        if ((operationNumber == 2 || operationNumber == 5) &&
            operationLastDirection != "上下" && operationLastDirection != "左右") {
            return {"002", "operationLastDirection is invalid: must be 上下 or 左右"};
        }
        if ((operationNumber == 1 || operationNumber == 3 || operationNumber == 4) &&
            operationLastDirection != "无") {
            return {"002", "operationLastDirection is invalid: must be 无"};
        }
        if ((game.find(chooseChessLocate) == game.end()) && (operationNumber == 4 || operationNumber == 5)) {
            return {"002", "chooseChessLocate is invalid: must be a valid chess"};
        }
        return {"000", "game is valid"};
        
    } catch (const std::exception &e) {
        return {"004", e.what()};
    }
}

std::vector<std::map<std::string, bool>> LockedChessClass::LockedChess::legalOperationAll() {
    std::map<std::string, bool> bloa, wloa = {
        {"u", true}, {"d", true}, {"l", true}, {"r", true}
    };
    bloa = wloa;
    auto gameCopy = game;
    for (const auto& piece: gameCopy) {
        if (piece.getColor() == "黑") {
            if (piece.getY() == 1) bloa["u"] = false;
            if (piece.getY() == 12) bloa["d"] = false;
            if (piece.getX() == 1) bloa["l"] = false;
            if (piece.getX() == 12) bloa["r"] = false;
            for (const auto& piece2: gameCopy) {
                if (piece2.getColor() == "白") {
                    if (piece.getX() == piece2.getX() && piece.getY() == piece2.getY() + 1) {
                        bloa["u"] = false;
                        wloa["d"] = false;
                    }
                    if (piece.getX() == piece2.getX() && piece.getY() == piece2.getY() - 1) {
                        bloa["d"] = false;
                        wloa["u"] = false;
                    }
                    if (piece.getX() == piece2.getX() + 1 && piece.getY() == piece2.getY()) {
                        bloa["l"] = false;
                        wloa["r"] = false;
                    }
                    if (piece.getX() == piece2.getX() - 1 && piece.getY() == piece2.getY()) {
                        bloa["r"] = false;
                        wloa["l"] = false;
                    }
                }
            }
        } else {
            if (piece.getY() == 1) wloa["u"] = false;
            if (piece.getY() == 12) wloa["d"] = false;
            if (piece.getX() == 1) wloa["l"] = false;
            if (piece.getX() == 12) wloa["r"] = false;
        }
    }
    return {bloa, wloa};
}

std::vector<std::map<ChessPiece, std::map<std::string, bool>>> LockedChessClass::LockedChess::legalOperationOne() {
    std::map<ChessPiece, std::map<std::string, bool>> bloo, wloo;
    auto gameCopy = game;
    for (const auto& chess: gameCopy) {
        std::map<std::string, bool> dirs = {
            {"u", true}, {"d", true}, {"l", true}, {"r", true}
        };
        if (chess.getY() == 1) dirs["u"] = false;
        if (chess.getY() == 12) dirs["d"] = false;
        if (chess.getX() == 1) dirs["l"] = false;
        if (chess.getX() == 12) dirs["r"] = false;
        for (const auto& chess2: gameCopy) {
            if (chess.getX() == chess2.getX() && chess.getY() == chess2.getY() + 1) {
                dirs["u"] = false;
            }
            if (chess.getX() == chess2.getX() && chess.getY() == chess2.getY() - 1) {
                dirs["d"] = false;
            }
            if (chess.getX() == chess2.getX() + 1 && chess.getY() == chess2.getY()) {
                dirs["l"] = false;
            }
            if (chess.getX() == chess2.getX() - 1 && chess.getY() == chess2.getY()) {
                dirs["r"] = false;
            }
        }
        if (chess.getColor() == "黑") {
            bloo[chess] = dirs;
        } else {
            wloo[chess] = dirs;
        }
    }
    return {bloo, wloo};
}

std::string LockedChessClass::LockedChess::returnGameWithOutAllOperation() {
    int board[12][12];
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            board[i][j] = 0;
        }
    }
    for (auto &i : game) {
        board[i.getY()-1][i.getX()-1] = (i.getColor() == "黑") ? 1 : -1;
    }
    json j;
    j["all_locate"] = board;
    j["operation_number"] = operationNumber;
    j["operation_oppsite"] = operationOppsite;
    j["operation_last_direction"] = operationLastDirection;
    j["choose_chess_locate"] = {
        chooseChessLocate.getX(),
        chooseChessLocate.getY(),
        chooseChessLocate.getColor()
    };
    return j.dump();
}

void LockedChessClass::LockedChess::operation(const Operation& op) {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    if (legalOperation().empty()) {
        throw std::runtime_error("Game is over");
    }
    if (op.isStringOperation()) {
        if (op.getStringOperation() == "c") {
            handleCancelOperation();
            return;
        }
        if (operationNumber == 3) {
            handleStringSelection(op.getStringOperation());
            return;
        }
        if (std::find(allOperation.begin(), allOperation.end(), op) != allOperation.end()) {
            changeGame(op.getStringOperation());
        }
        else {
            throw std::runtime_error("Invalid operation: " + op.getStringOperation());
        }
    } else {        
        if (operationNumber == 3) {
            if (game.find(op.getChessPieceOperation()) == game.end()) {
                throw std::runtime_error("Invalid chess piece: " + std::to_string(op.getChessPieceOperation().getX()) + std::string(",") +
                                        std::to_string(op.getChessPieceOperation().getY()) + " " + op.getChessPieceOperation().getColor());
            } else if (op.getChessPieceOperation().getColor() != operationOppsite) {
                throw std::runtime_error("Invalid chess piece color: " + op.getChessPieceOperation().getColor() + ", expected: " + operationOppsite);
            } else {
                chooseChessLocate = op.getChessPieceOperation();
            }
        } else {
            throw std::runtime_error("Invalid operation type: ChessPiece, expected StringOperation");
        }
    }
    updateOperationState();
    hasOperated = true;
}

void LockedChessClass::LockedChess::handleCancelOperation() {
    if (operationNumber == 4 && hasOperated) {
        operationNumber = 3;
        chooseChessLocate = {-1, -1, "无"};
    } else {
        throw std::runtime_error("Invalid operation: c");
    }
}

void LockedChessClass::LockedChess::handleStringSelection(const std::string& x) {
    auto locate = splitString(x, ',');
    if (locate.size() != 2) {
        throw std::runtime_error("Invalid operation: " + x);
    }
    ChessPiece selectedChess = {std::stoi(locate[0]), std::stoi(locate[1]), operationOppsite};
    if (game.find(selectedChess) != game.end()) {
        chooseChessLocate = selectedChess;
    } else {
        throw std::runtime_error("Invalid chess piece: " + locate[0] + "," +
                                 locate[1] + " " + selectedChess.getColor());
    }
}

void LockedChessClass::LockedChess::changeGame(const std::string& x) {
    std::set<ChessPiece> newGame;
    for (const auto& chess: game) {
        if (operationNumber == 1 || operationNumber == 2) {
            if (chess.getColor() != operationOppsite) {
                newGame.insert(chess);
            } else {
                newGame.insert(movePiece(chess, x));
            }
        } else if (operationNumber == 3) {
            throw std::runtime_error("Cannot move chess when operation number is 3");
        } else if (operationNumber == 4 || operationNumber == 5) {
            if (chess == chooseChessLocate) {
                newGame.insert(chooseChessLocate=movePiece(chess, x));
            } else {
                newGame.insert(chess);
            }
        } else {
            throw std::runtime_error("Invalid operation number: " + std::to_string(operationNumber));
        }
    }
    game.clear();
    for (const auto& chess : newGame) {
        game.insert(chess);
    }
    if (operationNumber == 1 || operationNumber == 4) {
        operationLastDirection = (x == "u" || x == "d") ? "上下" : "左右";
    } else {
        operationLastDirection = "无";
    }
}

ChessPiece LockedChessClass::LockedChess::movePiece(const ChessPiece& chess, const std::string& direction) {
    int x = chess.getX();
    int y = chess.getY();
    if (direction == "u") {
        y--;
    } else if (direction == "d") {
        y++;
    } else if (direction == "l") {
        x--;
    } else if (direction == "r") {
        x++;
    } else {
        throw std::runtime_error("Invalid direction: " + direction);
    }
    return ChessPiece(x, y, chess.getColor());
}

void LockedChessClass::LockedChess::updateOperationState() {
    if (operationNumber < 5) {
        operationNumber++;
    } else {
        operationNumber = 1;
        chooseChessLocate = {-1, -1, "无"};
        operationOppsite = (operationOppsite == "黑") ? "白" : "黑";
    }
}

std::string LockedChessClass::LockedChess::returnGame() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    auto state = returnGameWithOutAllOperation();
    json j = json::parse(state);
    json j_array = json::array();
    legalOperation();
    for (const auto& op: allOperation) {
        if (op.isStringOperation()) {
            j_array.push_back(op.getStringOperation());
        } else {
            j_array.push_back({
                op.getChessPieceOperation().getX(),
                op.getChessPieceOperation().getY(),
                op.getChessPieceOperation().getColor()
            });
        }
    }
    j["all_operation"] = j_array;
    return j.dump();
}

void LockedChessClass::LockedChess::loadsGame(const std::string& x, bool restart_game) {    
    std::unique_lock<std::recursive_mutex> locker(*lock);
    json j = json::parse(x);
    operationNumber = j["operation_number"].get<int>();
    operationOppsite = j["operation_oppsite"].get<std::string>();
    operationLastDirection = j["operation_last_direction"].get<std::string>();
    chooseChessLocate = ChessPiece(
        j["choose_chess_locate"][0].get<int>(),
        j["choose_chess_locate"][1].get<int>(),
        j["choose_chess_locate"][2].get<std::string>()
    );
    game.clear();
    if (j.find("all_locate") != j.end()) {
        std::vector<std::vector<int>> board = j["all_locate"].get<std::vector<std::vector<int>>>();
        if (board.size() != 12) {
            throw std::runtime_error("Invalid board size: " + std::to_string(board.size()));
        }
        for (auto& row : board) {
            if (row.size() != 12) {
                throw std::runtime_error("Invalid board size: " + std::to_string(row.size()));
            }
        }
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 12; j++) {
                if (board[i][j] != 0) {
                    if (board[i][j] != 1 && board[i][j] != -1) {
                        throw std::runtime_error("Invalid chess piece value at " + std::to_string(i) + "," + std::to_string(j) + ": " + std::to_string(board[i][j]));
                    }
                    game.insert(ChessPiece(j+1, i+1, board[i][j] == 1 ? "黑" : "白"));
                }
            }
        }
    } else if (j.find("game") != j.end()) {
        std::vector<ChessPiece> game_vector;
        for (const auto& chess : j["game"]) {
            game_vector.push_back(ChessPiece(chess[0].get<int>(), chess[1].get<int>(), chess[2].get<std::string>()));
        }
        for (const auto& chess : game_vector) {
            game.insert(chess);
        }
    } else {
        throw std::runtime_error("Invalid game format");
    }
    legalOperation();
    if (restart_game) {
        hasOperated = false;
    }
}

std::vector<std::vector<Operation>> LockedChessClass::LockedChess::calculateAllChains() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    auto gameState = returnGameWithOutAllOperation();
    {
        std::shared_lock<std::shared_mutex> cacheLocker(chainCacheLock);
        auto it = chainCache.find(gameState);
        if (it != chainCache.end()) {
            {
                std::unique_lock<std::shared_mutex> countLocker(chainCacheCountLock);
                chainCacheCount[gameState]++;
                chainCacheTime[gameState] = std::chrono::system_clock::now();
            }
            return it->second;
        }
    }
    chainResult.clear();
    visitedChains.clear();
    int requiredSteps = 6 - operationNumber;
    dfs(gameState, {}, requiredSteps);
    std::vector<std::vector<Operation>> result;
    for (const auto& chain : chainResult) {
        result.push_back(chain);
    }
    chainResult.clear();
    visitedChains.clear();
    {
        std::unique_lock<std::shared_mutex> cacheLocker(chainCacheLock);
        chainCache[gameState] = result;
        std::unique_lock<std::shared_mutex> countLocker(chainCacheCountLock);
        chainCacheCount[gameState] = 1;
        chainCacheTime[gameState] = std::chrono::system_clock::now();
    }
    
    return result;
}

void LockedChessClass::LockedChess::dfs(const std::string& gameState, std::vector<Operation> path, int requireStep) {
    if (visitedChains.find(gameState) != visitedChains.end()) {
        return;
    }
    visitedChains.insert(gameState);
    LockedChess temp;
    temp.loadsGame(gameState);
    if (temp.getOperationNumber() == 1 && path.size() >= requireStep) {
        chainResult.push_back(std::vector<Operation>(path.begin(), path.begin() + requireStep));
        return;
    }
    auto legalOps = temp.legalOperation();
    if (legalOps.empty()) {
        return;
    }
    for (const auto& op : legalOps) {
        temp.operation(op);
        path.push_back(op);
        dfs(temp.returnGameWithOutAllOperation(), path, requireStep);
        temp.loadsGame(gameState); // Reset to the original game state
        path.pop_back();
    }
}

bool LockedChessClass::LockedChess::isEqualChains(std::vector<Operation>& chain1, std::vector<Operation>& chain2) {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    try {
        auto state1 = applyChain(chain1);
        auto state2 = applyChain(chain2);
        return state1 == state2;
    } catch (const std::exception& e) {
        throw std::runtime_error("Error comparing chains: " + std::string(e.what()));
    }
}

std::string LockedChessClass::LockedChess::applyChain(std::vector<Operation>& chain) {
    LockedChess temp;
    temp.loadsGame(returnGameWithOutAllOperation());
    for (const auto& op : chain) {
        temp.operation(op);
    }
    return temp.returnGameWithOutAllOperation();
}

std::unordered_set<ChessPiece> LockedChessClass::LockedChess::getGame() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    return game;
}

int LockedChessClass::LockedChess::getOperationNumber() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    return operationNumber;
}

std::string LockedChessClass::LockedChess::getOperationOppsite() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    return operationOppsite;
}

ChessPiece LockedChessClass::LockedChess::getChooseChessLocate() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    return chooseChessLocate;
}

std::string LockedChessClass::LockedChess::getOperationLastDirection() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    return operationLastDirection;
}

bool LockedChessClass::LockedChess::getHasOperated() {
    std::unique_lock<std::recursive_mutex> locker(*lock);
    return hasOperated;
}

std::shared_ptr<std::recursive_mutex> LockedChessClass::LockedChess::getLock() const {
    return lock;
}

LockedChessClass::LockedChessClass() : game(new LockedChess()){}
void LockedChessClass::gameStart() {game->gameStart();}
std::vector<Operation> LockedChessClass::legalOperation() {return game->legalOperation();}
void LockedChessClass::operation(const Operation& op) {game->operation(op);}
std::string LockedChessClass::returnGame() {return game->returnGame();}
void LockedChessClass::loadsGame(const std::string& x, bool restart_game) {game->loadsGame(x, restart_game);}
std::vector<std::vector<Operation>> LockedChessClass::calculateAllChains() {return game->calculateAllChains();}
bool LockedChessClass::isEqualChains(std::vector<Operation>& chain1, std::vector<Operation>& chain2) {return game->isEqualChains(chain1, chain2);}
std::unordered_set<ChessPiece> LockedChessClass::getGame() {return game->getGame();}
int LockedChessClass::getOperationNumber() {return game->getOperationNumber();}
std::string LockedChessClass::getOperationOppsite() {return game->getOperationOppsite();}
ChessPiece LockedChessClass::getChooseChessLocate() {return game->getChooseChessLocate();}
std::string LockedChessClass::getOperationLastDirection() {return game->getOperationLastDirection();}
bool LockedChessClass::getHasOperated() {return game->getHasOperated();}
std::shared_ptr<std::recursive_mutex> LockedChessClass::getLock() const {return game->getLock();}
void LockedChessClass::cleanLegalOperationCache() {
    CacheCleaner::cleanCache(
        LockedChessClass::LockedChess::legalOperationCache, 
        LockedChessClass::LockedChess::legalOperationCacheCount, 
        LockedChessClass::LockedChess::legalOperationCacheTime, 
        LockedChessClass::LockedChess::legalOperationCacheLock, 
        LockedChessClass::LockedChess::legalOperationCacheCountLock
    );
}
void LockedChessClass::cleanChainCache() {
    CacheCleaner::cleanCache(
        LockedChessClass::LockedChess::chainCache, 
        LockedChessClass::LockedChess::chainCacheCount, 
        LockedChessClass::LockedChess::chainCacheTime, 
        LockedChessClass::LockedChess::chainCacheLock, 
        LockedChessClass::LockedChess::chainCacheCountLock
    );
}
