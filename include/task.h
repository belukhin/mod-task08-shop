#ifndef TASK_H
#define TASK_H

#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include <chrono>
#include <cmath>
#include <thread>

using namespace std;

class Client;

class Shop
{
    private:
        struct Cash_reg
        {
            int waitStat;
            int workStat;
            thread thrd;
        };
        int cash;
        vector<Cash_reg> cashes;
        int speed;
        int rate;
        int maxQueue;
        int avgGoods;

        deque<Client*> queue;
        mutex mu;

        int completedCount;
        int rejectedCount;

        int queueLength;
        int procCounter;

    public:
        Shop(int cashCount, int rate, double speed, int avgGoods, int maxCount);
        void Start(int index);
        string Act();
};

class Client
{
    private:
        chrono::system_clock::time_point start_time;
        int Qtime, CR, goods;
        unsigned int id;
        static unsigned int gen_id;
    
    public:
        Client(int avgGoods);
        unsigned int GetId();
        int GetQtime();
        int GetCRTime();
        void EnterQ();
        void ExitQ();
        void ExitCR(int time);
        int GetGoods();
};

#endif
