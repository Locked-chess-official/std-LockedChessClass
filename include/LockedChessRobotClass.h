#ifndef LockedChessRobotClass_H
#define LockedChessRobotClass_H

#include "LockedChessAllInterface.h"
#include <random>
#include <stdexcept>

class LockedChessRobotClass : public LockedChessRobotAllInterface {
public:
     LockedChessRobotClass();
    void  setGame(std::shared_ptr<LockedChessAllInterface> game) override;
    std::vector<Operation>  getOperation() override;
    std::vector<Operation>  getBetterOperation(bool useBetter) override;
private:
    struct LockedChessRobot: LockedChessRobotAllInterface {
    public:
        LockedChessRobot();
        void setGame(std::shared_ptr<LockedChessAllInterface> game) override;
        std::vector<Operation> getOperation() override;
        std::vector<Operation> getBetterOperation(bool useBetter) override;
    private:
        std::shared_ptr<LockedChessAllInterface> game;
        bool dueToFail(std::vector<Operation>& chain) const;
        bool justWin(std::vector<Operation>& chain) const;
    };
    std::shared_ptr<LockedChessRobot> robot;
};

template<typename T>
inline T  removeRandomElement(std::vector<T>& vec);
template<typename T>
T removeRandomElement(std::vector<T>& vec) {
    if (vec.empty()) {
        throw std::runtime_error("Cannot remove element from an empty vector.");
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, vec.size() - 1);
    int index = dis(gen);
    T ret = vec[index];
    vec.erase(vec.begin() + index);
    return ret;
}

#endif // LockedChessRobotClass_H