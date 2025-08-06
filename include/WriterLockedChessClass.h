#ifndef WriterLockedChessClass_H
#define WriterLockedChessClass_H
#include "LockedChessClass.h"

class WriterLockedChessClass : public WriterLockedChessAllInterface {
public:
     WriterLockedChessClass();
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
    void  setProtect() override;
    void  setUnprotect() override;
    std::string  getFinalResult() override;
    bool  getProtect() override;
    bool  getPeace() override;
    bool  getHasEnd() override;
private:
    using DictMapType = std::map<int, std::string>;
    struct WriterLockedChess: public LockedChessClass::LockedChess {
    public:
        WriterLockedChess();
        void gameStart();
        void operation(const Operation& op);
        void loadsGame(const std::string& x, bool restart_game = false);
        void setProtect();
        void setUnprotect();
        std::string getFinalResult();
        bool getProtect();
        bool getPeace();
        bool getHasEnd();
    private:
        void endWrite();
        void normalWrite();
        void handleOtherOperation(const std::string& x);
        bool isProtected = false;
        std::string boardString;
        std::string operationString;
        std::string timeString;
        std::string startTime;
        std::chrono::system_clock::time_point lastOperationTime;
        bool inPeace = false;
        bool endOfGame = false;
        static inline const WriterLockedChessClass::DictMapType dictMap = {
            {0, "0"},
            {1, "1"},
            {2, "2"},
            {3, "3"},
            {4, "4"},
            {5, "5"},
            {6, "6"},
            {7, "7"},
            {8, "8"},
            {9, "9"},
            {10, "A"},
            {11, "B"},
            {12, "C"}
        };
    };
    std::shared_ptr<WriterLockedChess> game;
};

#endif // WriterLockedChessClass_H