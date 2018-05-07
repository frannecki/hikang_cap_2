//
// Created by fran on 5/3/18.
//

int str_len(char *str)
{
    int i = 0;
    for(; str[i] != '\0'; ++i);
    return i;
}

#include "cap.h"
int main()
{
    char **a = new char*[3];
    for(int i = 0; i < 3; ++i)
        *a = new char[20];
    //a[0] = "192.168.1.104";
    //a[1] = "admin";
    //a[2] = "haikang#1";
    printf("IP: ");
    scanf("%s", a[0]);
    printf("Username: ");
    scanf("%s", a[1]);
    printf("Password: ");
    scanf("%s", a[2]);
    int shape[3] = {100, 100, 3};
    cap(10, shape, 0.01, 0.01, a);

    for (int i = 0; i < 3;)
         delete []a[i];
    delete []a;
    return 0;
}
