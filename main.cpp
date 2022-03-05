#include "Stack.h"
#include "Logs.h"


void print (FILE* output, void* item);

int main()
{
    OpenLogs();


    Stack stk = {};

    StackCtor (&stk, 10, int, print);

    StackDump (&stk);


    for(int i = 1; i < 1000 && StackPush(&stk, &i) == SUCCESS; i++)
        ;

    int a = 0;
    for(int i = 0; StackPop(&stk, &a) == SUCCESS; i++)
        printf("%d\n", a);

    StackDump (&stk);

    StackDtor(&stk);


    CloseLogs();

    return 0;
}

void print(FILE * output, void * item)
{
    fprintf(output, "%d", *((int*) item));
}

