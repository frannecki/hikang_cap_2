//
// Created by fran on 5/3/18.
//

#include "cap.h"
int main()
{
    char **a = new char*[3];
    for(int i = 0; i < 3; ++i)
        *a = new char[20];
    a[0] = "192.168.1.104";
    a[1] = "admin";
    a[2] = "haikang#1";

    int shape[3] = {100, 100, 3};
    cap(10, shape, 0.01, 0.01, a);
    return 0;
}