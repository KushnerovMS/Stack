#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "MacrosOverloaded.h"

enum StackFunctionError
{
    SUCCESS                 = 0,
    ERROR_STACK             = 1,
    MEMORY_ALLOCATION_ERROR = 2,
    EXPANSION_DENIED        = 3,
    NO_ITEMS_ERROR          = 4,
    MAX_SIZE_REACHED        = 5,
    NOT_CLEAN_STACK         = 6
};

typedef struct
{
    unsigned int Ok                     : 1;
    unsigned int Poisoned               : 1;
    unsigned int StackDamaged           : 1;
    unsigned int DataDamaged            : 1;
    unsigned int NullPointer            : 1;
    unsigned int SizeLargeThanMax       : 1;
    unsigned int CapacityLargeThanMax   : 1;
    unsigned int SizeMoreThenCapacity   : 1;
    unsigned int DataNullPointer        : 1;
}
StackError;

enum DumpMode
{
    ALL_INFO                    = 1,
    MIN_INFO                    = 2,
    ALL_INFO_WITHOUT_ITEMS      = 3,
    SECURITY_INFO               = 4
};

const int DATA_POISON = 666;
const size_t SIZE_POISON = (size_t)-666;
char *const POINTER_POISON = (char*)13;
const int HASH_POISON = -666;


const unsigned char CANARY = 0x0F;
const unsigned char POISONED_CANARY = 0x0D;

typedef struct
{
    unsigned char Canary1;

    char * Data;
    size_t Capacity;
    size_t Size;
    size_t ItemSize;

    int DataHash;

#ifdef _DEBUG

    const char * dname;
    const char * dtype;
    const char * dfunction;
    const char * dfile;
    void (*dprintItem)(FILE * output, void *);
    size_t dline;

#endif

    unsigned char Canary2;
}
Stack;

const int STACK_MAX_SIZE = 256;
const int STACK_MIN_SIZE = STACK_MAX_SIZE/64;
const int STACK_MAX_STEP = STACK_MAX_SIZE / 8;
const float STACK_STEP_INCREASE = 1;

StackFunctionError StackCtor__ (Stack * stack, const size_t capacity, const size_t itemSize);

#ifdef _DEBUG
#define StackCtor(stack, ...)                                   \
{                                                               \
    (stack) -> dprintItem = nullptr;                            \
                                                                \
    SELECT_OVERLOADED_MACROS(StackCtor_, stack, __VA_ARGS__);   \
}

#define StackCtor_3(stack, capacity, itemType)                  \
{                                                               \
    const char * name__  = #stack;                              \
    if(*name__ == '&') name__ ++;                               \
    (stack) -> dname = name__;                                  \
                                                                \
    const char * file__ = __FILE__;                             \
    const char * f__ = file__;                                  \
    while (*f__ != '\0')                                        \
    {                                                           \
        while (*f__ != '\0' && *(f__ ++) != '\\')               \
            ;                                                   \
        if(*(f__ - 1) == '\\')                                  \
            file__ = f__;                                       \
    }                                                           \
    (stack) -> dfile = file__;                                  \
                                                                \
    (stack) -> dline = __LINE__;                                \
                                                                \
    (stack) -> dfunction = __FUNCTION__;                        \
                                                                \
    (stack) -> dtype = #itemType;                               \
                                                                \
    StackCtor__(stack, capacity, sizeof(itemType));             \
}

#define StackCtor_4(stack, capacity, itemType, printItem)       \
{                                                               \
    (stack) -> dprintItem = printItem;                          \
    StackCtor_3(stack, capacity, itemType);                     \
}

#else

#define StackCtor(stack, capacity, itemType)\
    StackCtor__(stack, capacity, sizeof(itemType));

#endif

StackFunctionError StackDtor (Stack * stack);
StackFunctionError StackPush (Stack * stack, const void* value);
StackFunctionError StackPop (Stack * stack, void * value);

void StackDump__ (const Stack * stack, bool* isOk = nullptr);
void StackDump__ (const Stack * stack, DumpMode mode,
                  const bool isOk = true, const char * message = nullptr);
#define StackDump(...)                                          \
{                                                               \
    extern FILE * StackLogs;                                    \
    if (StackLogs)                                              \
    {                                                           \
        const char * file__ = __FILE__;                         \
        const char * f__ = file__;                              \
        while (*f__ != '\0')                                    \
        {                                                       \
            while (*f__ != '\0' && *(f__ ++) != '\\')           \
                ;                                               \
            if(*(f__ - 1) == '\\')                              \
                file__ = f__;                                   \
        }                                                       \
                                                                \
        fprintf (StackLogs, "%s at %s at (%i)\n",               \
                       __FUNCTION__, file__, __LINE__);         \
        StackDump__ (__VA_ARGS__);                              \
    }                                                           \
}
bool StackOk (const Stack * stack, StackError * err = nullptr);
bool StackNull (const Stack * stack);
#endif
