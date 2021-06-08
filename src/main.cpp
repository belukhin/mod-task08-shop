#include <string>
#include <iostream>

#include "../include/task.h"

using namespace  std;


int main()
{
    char a;
    Shop shop(4, 20, 700, 7, 5);
    cout << shop.Act();
    cin >> a;
    return 0;
}