#ifndef LockedChessClass_H
#define LockedChessClass_H
#include "LockedChessAllInterface.h"
#include <Map>
#include <set>
#include <shared_mutex>
#include <chrono>


class LockedChessClass : public LockedChessAllInterface {
public:
     LockedChessClass();
    void  gameStart() override;
    std::vector<Operation>  legalOperation() override;
    void  operation(const Operation& op) override;
    std::string  returnGame() override;
    void  loadsGame(const std::string& x, bool restart_game = false) override;
    std::vector<std::vector<Operation>>  calculateAllChains() override;
    bool  isEqualChains(std::vector<Operation>& chain1, std::vector<Operation>& chain2) override;
    std::unordered_set<ChessPiece>  getGame() override;
    int  getOperationNumber() override;
    std::string  getOperationOppsite() override;
    ChessPiece  getChooseChessLocate() override;
    std::string  getOperationLastDirection() override;
    bool  getHasOperated() override;
    std::shared_ptr<std::recursive_mutex>  getLock() const override;
    static void  cleanLegalOperationCache();
    static void  cleanChainCache();
private:
    class LockedChess {
    public:
        LockedChess();
        void gameStart();
        std::vector<Operation> legalOperation();
        void operation(const Operation& op);
        std::string returnGame();
        void loadsGame(const std::string& x, bool restart_game = false);
        std::vector<std::vector<Operation>> calculateAllChains();
        bool isEqualChains(std::vector<Operation>& chain1, std::vector<Operation>& chain2);
        std::unordered_set<ChessPiece> getGame();
        int getOperationNumber();
        std::string getOperationOppsite();
        ChessPiece getChooseChessLocate();
        std::string getOperationLastDirection();
        bool getHasOperated();
        std::shared_ptr<std::recursive_mutex> getLock() const;
    protected:
        std::unordered_set<ChessPiece> game;
        std::vector<Operation> allOperation;
        int operationNumber;
        std::string operationOppsite;
        ChessPiece chooseChessLocate;
        std::string operationLastDirection;
        bool hasOperated;
        std::shared_ptr<std::recursive_mutex> lock;
    private:
        static std::map<std::string, std::vector<Operation>> legalOperationCache;
        static std::map<std::string, int> legalOperationCacheCount;
        static std::map<std::string, std::chrono::system_clock::time_point> legalOperationCacheTime;
        static std::shared_mutex legalOperationCacheLock;
        static std::shared_mutex legalOperationCacheCountLock;
        static std::map<std::string, std::vector<std::vector<Operation>>> chainCache;
        static std::map<std::string, int> chainCacheCount;
        static std::map<std::string, std::chrono::system_clock::time_point> chainCacheTime;
        static std::shared_mutex chainCacheLock;
        static std::shared_mutex chainCacheCountLock;
        std::vector<std::string> checkGame();
        std::vector<std::map<std::string, bool>> legalOperationAll();
        std::vector<std::map<ChessPiece, std::map<std::string, bool>>> legalOperationOne();
        std::string returnGameWithOutAllOperation();
        void handleCancelOperation();
        void handleStringSelection(const std::string& x);
        void changeGame(const std::string& x);
        static ChessPiece movePiece(const ChessPiece& chess, const std::string& direction);
        void updateOperationState();
        std::vector<std::vector<Operation>> chainResult;
        std::set<std::string> visitedChains;        
        friend class LockedChessClass;
        void dfs(const std::string& gameState, std::vector<Operation> path, int requireStep);
        std::string applyChain(std::vector<Operation>& chain);
    };
    friend class WriterLockedChessClass;
    friend class LockedChessRobotClass;
    std::shared_ptr<LockedChess> game;
};

#endif // LockedChessClass_H