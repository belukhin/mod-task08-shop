#include <string>
#include <iostream>

#include "../include/task.h"

using namespace std;

unsigned int Client::gen_id = 1;

long Fact(long n)
{
    if (n == 0)
        return 1;
    else
        return n * Fact(n - 1);
}

/////////////////////////
/// Shop
/////////////////////////

Shop::Shop(int cashCount, int rate, double speed, int avgGoods, int maxCount)
{
    cash = cashCount;
    this->rate = rate;
    this->speed = speed;
    this->avgGoods = avgGoods;
    maxQueue = maxCount;

    completedCount = 0;
    rejectedCount = 0;
    queueLength = 0;
    procCounter = 0;

    cashes = vector<Cash_reg>();
    queue = deque<Client *>();

    for (int i = 0; i < cash; i++)
    {
        cashes.push_back(Cash_reg());
        cashes[i].thrd = thread(&Shop::Start, this, i);
    }
}

void Shop::Start(int index)
{
    while (completedCount + rejectedCount < rate)
    {
        mu.lock();
        queueLength += queue.size();
        procCounter++;
        if (queue.size() > 0)
        {
            queue.at(0)->ExitQ();
            int goods = queue.at(0)->GetGoods();
            unsigned int id = queue.at(0)->GetId();
            cashes[index].workStat += goods * speed;
            completedCount++;
            queue.at(0)->ExitCR(goods * speed);
            queue.pop_front();
            mu.unlock();
            while (goods > 0)
            {
                this_thread::sleep_for(chrono::milliseconds(speed));
                goods -= 1;
            }
            cout << "Client with ID#" << id << " served." << endl;
        }
        else
        {
            cashes[index].waitStat += 10;
            mu.unlock();
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
}

string Shop::Act()
{
    srand(time(0));

    string out = "";
    int countClients = 0;
    vector<Client> clients;

    for (int i = 0; i < rate; i++)
        clients.push_back(Client(avgGoods));

    double avgQLen = 0;

    while (countClients < rate){
        int time = rand() % 1000;
        this_thread::sleep_for(std::chrono::milliseconds(time));
        mu.lock();
        avgQLen += queue.size();
        if (queue.size() < maxQueue){
            queue.push_back(&clients.at(countClients));
            clients.at(countClients).EnterQ();
            mu.unlock();
        }
        else {
            mu.unlock();
            rejectedCount++;
            cout << "Client with ID#" << clients.at(countClients).GetId() << " not served." << endl;
        }
        countClients++;
    }

    double avgWait = 0.0;
    double avgWork = 0.0;
    double avgTimeQ = 0.0;
    double avgTimeCashBox = 0.0;
    double workTime = 0.0;

    for (int i = 0; i < cash; i++){
        cashes[i].thrd.join();
        avgWait += cashes[i].waitStat;
        avgWork += cashes[i].workStat;
    }
    for (int i = 0; i < rate; i++){
        if (clients.at(i).GetQtime() == 0 && clients.at(i).GetCRTime() == 0)
            continue;
        avgTimeQ += clients.at(i).GetQtime();
        avgTimeCashBox += clients.at(i).GetCRTime();
    }

    avgQLen /= (double)countClients;
    workTime = avgWork + avgWait;
    avgWait /= (double)cash;
    avgWork /= (double)cash;
    avgTimeQ /= (double)completedCount;
    avgTimeCashBox /= (double)completedCount;

    double lambda = rate * 1000.0 / avgWork;
    double m = 1000.0 / avgTimeCashBox;
    double r = lambda / m;
    double P0 = 1.0;
    for (int i = 1; i <= cash; i++)
        P0 += pow(r, i) / Fact(i);
    for (int i = cash + 1; i < cash + maxQueue; i++)
        P0 += pow(r, i) / (Fact(cash) * pow(cash, i - cash));

    P0 = 1.0 / P0;
    double Prej = pow(r, cash + maxQueue) * P0 / (double)(pow(cash, maxQueue) * Fact(cash));
    double Q = 1.0 - Prej;
    double A = lambda * Q;

    out += "\n\nStats:\n";

    out += "\nServed Clients: " + to_string(completedCount);
    out += "\nUnserved Clients: " + to_string(rejectedCount);
    out += "\nAvg queue: " + to_string(avgQLen);
    out += "\nAvg time in queue and on CR: " + to_string(avgTimeQ) + " + " + to_string(avgTimeCashBox);
    out += "\nAvg CR time: " + to_string(avgWork);
    out += "\nAvg CR downtime: " + to_string(avgWait);
    out += "\nSystem failure probability: " + to_string((double)rejectedCount / (double)rate);
    out += "\nRelative store throughput: " + to_string((double)completedCount / (double)rate);
    out += "\nAbsolute bandwidth: " + to_string(lambda * (double)completedCount / (double)rate);
    
    out += "\n";
    out += "\nProbability of failure: " + to_string(Prej);
    out += "\nRelative bandwidth: " + to_string(Q);
    out += "\nAbsolute bandwidth: " + to_string(A);

    return out;
}


//////////////////////////////////////////
/// Client
//////////////////////////////////////////

Client::Client(int avgGoods)
{
    Qtime = 0;
    CR = 0;
    goods = rand() % (2 * avgGoods);
    id = gen_id++;
}

unsigned int Client::GetId(){
    return id;
}

int Client::GetQtime(){
    return Qtime;
}

int Client::GetCRTime(){
    return CR;
}

void Client::EnterQ(){
    start_time = chrono::system_clock::now();
}

void Client::ExitQ(){
    Qtime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start_time).count();
}

void Client::ExitCR(int time){
    CR = time;
}

int Client::GetGoods(){
    return goods;
}

