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
    char **b = new char*[3];
    for(int i = 0; i < 3; ++i)
    {
        a[i] = new char[25];
        b[i] = new char[25];
    }
        
    //a[0] = "192.168.1.104";
    //a[1] = "admin";
    //a[2] = "haikang#1";
    b[0] = "IP";
    b[1] = "Username";
    b[2] = "Password";
    for(int i = 0; i < 3; ++i)
    {
        printf("%s: ", b[i]);
        scanf("%s", a[i]);
        if(strlen(a[i]) > 20)
        {
            printf("\n须在20字符以内!\n");
            return -1;
        }
            
    }
    
    int shape[3] = {100, 100, 3};
    cap(10, shape, 0.01, 0.01, a);

    for (int i = 0; i < 3;)
         delete []a[i];
    delete []a;
    return 0;
}
