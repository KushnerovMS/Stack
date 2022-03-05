#include "Stack.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

FILE* StackLogs = nullptr;

int getStackDataHash (const Stack* stack);

StackFunctionError StackCtor__ (Stack * stack, const size_t capacity, const size_t itemSize)
{
    if (StackNull (stack))
    {
        stack -> Data = (char*) calloc (capacity, itemSize);
        if (stack -> Data == nullptr)
            return MEMORY_ALLOCATION_ERROR;

        memset(stack -> Data, 0, capacity * itemSize);

        stack -> Size           = 0;
        stack -> Capacity = capacity;
        stack -> ItemSize = itemSize;

        stack -> Canary1    = CANARY;
        stack -> Canary2   = CANARY;

        stack -> DataHash = getStackDataHash (stack);

        StackDump (stack);

        return SUCCESS;
    }
    else
    {
        StackDump (stack, ALL_INFO_WITHOUT_ITEMS, true,
                   "ERROR: you can't create a stack in place of a non-clean stack");
        assert (!"You can't create a stack in place of a non-clean stack");
        return NOT_CLEAN_STACK;
    }
}

StackFunctionError StackDtor (Stack * stack)
{
    bool StackOk = false;
    StackDump (stack, &StackOk);

    if (StackOk)
    {
        memset(stack -> Data, DATA_POISON, stack -> Capacity);

        free(stack -> Data);

        stack -> Data       = POINTER_POISON;
        stack -> Size       = SIZE_POISON;
        stack -> Capacity   = SIZE_POISON;
        stack -> ItemSize   = SIZE_POISON;

        stack -> Canary1 = POISONED_CANARY;
        stack -> Canary2 = POISONED_CANARY;

        stack -> DataHash = HASH_POISON;

        return SUCCESS;
    }
    else
    {
        return ERROR_STACK;
    }
}


StackFunctionError stackRealocate (Stack * stack, size_t capacity);

StackFunctionError StackPush (Stack * stack, const void* value)
{
    bool StackOk = false;
    StackDump (stack, &StackOk);

    if (StackOk)
    {
        StackFunctionError error = SUCCESS;
        if(stack -> Size >= stack -> Capacity)
        {
            error = stackRealocate(stack, stack -> Size + 1);
            if(error != SUCCESS)
                return error;
        }

        memcpy(stack -> Data + (stack -> Size ++) * stack -> ItemSize,
               value,
               stack -> ItemSize);

        stack -> DataHash = getStackDataHash (stack);

        return SUCCESS;
    }
    else
    {
        return ERROR_STACK;
    }
}

StackFunctionError StackPop (Stack * stack, void * value)
{
    bool StackOk = false;
    StackDump (stack, &StackOk);

    if (StackOk)
    {
        if (stack -> Size != 0)
        {
            memcpy(value,
                   stack -> Data + (-- stack -> Size) * stack -> ItemSize,
                   stack -> ItemSize);
            stackRealocate(stack, stack -> Size);

            stack -> DataHash = getStackDataHash (stack);

            return SUCCESS;
        }
        else
            return NO_ITEMS_ERROR;
    }
    else
    {
        return ERROR_STACK;
    }
}


void defaultPrint(FILE * file, const void * item, size_t size);

StackFunctionError stackRealocate (Stack * stack, size_t capacity)
{
    if(capacity > STACK_MAX_SIZE)
        return MAX_SIZE_REACHED;


    size_t calcCapacity = capacity;

    if(stack -> Capacity < capacity)
    {
        if(stack -> Capacity * STACK_STEP_INCREASE < STACK_MAX_STEP)
            calcCapacity = stack -> Capacity *(1 + STACK_STEP_INCREASE);
        else
            calcCapacity = stack -> Capacity + STACK_MAX_STEP;
    }

    if(stack -> Capacity > capacity)
    {
        if(stack -> Capacity * STACK_STEP_INCREASE / (1 + STACK_STEP_INCREASE) <= STACK_MAX_STEP)
        {
            if(stack -> Capacity / ((1 + STACK_STEP_INCREASE)*(1 + STACK_STEP_INCREASE)) < capacity)
                return SUCCESS;
            else
                calcCapacity = stack -> Capacity / (1 + STACK_STEP_INCREASE);
        }
        else
        {
            if(stack -> Capacity - 2 * STACK_MAX_STEP < capacity)
                return SUCCESS;
            else
                calcCapacity = stack -> Capacity - STACK_MAX_STEP;
        }
    }


    if(calcCapacity <= STACK_MIN_SIZE)
        return SUCCESS;
    if(calcCapacity > STACK_MAX_SIZE)
        calcCapacity = STACK_MAX_SIZE;

    char * buff = (char*) realloc (stack -> Data, calcCapacity * stack -> ItemSize);
    if(buff != nullptr)
    {
        stack -> Data = buff;
        stack -> Capacity = calcCapacity;
        memset (stack -> Data + stack -> ItemSize * stack -> Size,
                0,
                (stack -> Capacity - stack -> Size) * stack -> ItemSize);
        return SUCCESS;
    }
    else
        return MEMORY_ALLOCATION_ERROR;
}

void StackDump__ (const Stack * stack, bool* isOk)
{
    StackError err = {};
    if (StackOk (stack, &err))
    {
        StackDump__ (stack, MIN_INFO, true);
        if (isOk) *isOk = true;
    }
    else if (err.Poisoned)
    {
        StackDump__ (stack, ALL_INFO_WITHOUT_ITEMS, false, "ERROR: POISONED");
        if (isOk) *isOk = false;
    }
    else if (err.NullPointer)
    {
        StackDump__ (stack, ALL_INFO, false, "ERROR: NULL_POINTER");
        if (isOk) *isOk = false;
    }
    else if (err.StackDamaged)
    {
        StackDump__ (stack, SECURITY_INFO, false, "ERROR: STACK_DAMAGED");
    }
    else if (err.DataDamaged)
    {
        StackDump__ (stack, SECURITY_INFO, false, "ERROR: DATA_DAMAGED");
    }
    else
    {
        char msg [256] = "ERRORS: ";
        if (err.SizeLargeThanMax)       strcat (msg, "SIZE_MORE_THAN_MAX, ");
        if (err.CapacityLargeThanMax)   strcat (msg, "CAPACITY_MORE_THAN_MAX, ");
        if (err.SizeMoreThenCapacity)   strcat (msg, "SIZE_MORE_THAN_CAPACITY, ");
        if (err.DataNullPointer)        strcat (msg, "DATA_NULL_POINTER, ");
        strcpy(msg + strlen (msg) - 2, ";");

        StackDump__ (stack, ALL_INFO, false, msg);
        if (isOk) *isOk = false;
    }
}

void StackDump__ (const Stack * stack, DumpMode mode, const bool isOk, const char * message)
{
    if (StackLogs)
    {

#ifdef _DEBUG
        fprintf (StackLogs, "stack <%s> [0x%08X] %s ",
                        (stack -> dtype)? stack -> dtype : "?", stack, (isOk)? "ok" : "ERROR");

        if (stack) fprintf (StackLogs, "\"%s\" at %s() at %s (%d)\n",
                                    stack -> dname, stack -> dfunction, stack -> dfile, stack -> dline);
#else
    fprintf (StackLogs, "stack [0x%08X] %s\n", stack, (isOk)? "ok" : "ERROR");
#endif

        if (message) fprintf (StackLogs, "%s\n", message);

        if (stack && mode != MIN_INFO)
        {
            fprintf (StackLogs, "{\n");

            if (mode == SECURITY_INFO)
            {
                fprintf (StackLogs, "\tfirst canary = %u, second canary = %u\n",
                                    stack -> Canary1, stack -> Canary2);
                fprintf (StackLogs, "\tdata hash = 0x%04X\n\n", stack -> DataHash);
            }

            fprintf (StackLogs, "\tsize = %u\n", stack -> Size);
            fprintf (StackLogs, "\tcapacity = %u\n", stack -> Capacity);
            fprintf (StackLogs, "\titem size = %u\n", stack -> ItemSize);
            fprintf (StackLogs, "\tdata[0x%08X]\n", stack -> Data);
            if (stack -> Data != nullptr && (mode == ALL_INFO ||  mode == SECURITY_INFO))
            {
                for (int i = 0; i < stack -> Capacity; i++)
                {
                    fprintf (StackLogs, "\t\t[%d] = ", i);
#ifdef _DEBUG
                    if (stack -> dprintItem != nullptr)
                        (*stack -> dprintItem) (StackLogs, (void*) (stack -> Data + i * stack -> ItemSize));
                    else
#endif
                        defaultPrint(StackLogs, stack -> Data + i * stack -> ItemSize, stack -> ItemSize);
                    fprintf (StackLogs, "\n");
                }

            }
            fprintf (StackLogs, "}\n");
        }
        fprintf (StackLogs, "\n");
    }
}

bool StackOk (const Stack * stack, StackError * err)
{
    bool ok = 1;
    if (err) *err = {};
    if (stack == nullptr)
    {
        ok = 0;
        if (err) err -> NullPointer = 1;
    }
    else if (stack -> Data          == POINTER_POISON   &&
             stack -> Size          == SIZE_POISON      &&
             stack -> Capacity      == SIZE_POISON      &&
             stack -> ItemSize      == SIZE_POISON      &&

             stack -> Canary1   == POISONED_CANARY  &&
             stack -> Canary2  == POISONED_CANARY)
    {
        ok = 0;
        if (err) err -> Poisoned = 1;
    }
    else if (stack -> Canary1  != CANARY ||
             stack -> Canary2  != CANARY)
    {
        ok = 0;
        if (err) err -> StackDamaged = 1;
    }
    else if (stack -> DataHash != getStackDataHash (stack))
    {
        ok = 0;
        if (err) err -> DataDamaged = 1;
    }
    else
    {
        if (stack -> Data == nullptr)
        {
            if (err) err -> DataNullPointer = 1;
            ok = 0;
        }

        if (stack -> Size > STACK_MAX_SIZE)
        {
            if (err) err -> SizeLargeThanMax = 1;
            ok = 0;
        }

        if (stack -> Capacity > STACK_MAX_SIZE)
        {
            if (err) err -> CapacityLargeThanMax = 1;
            ok = 0;
        }

        if (stack -> Size > stack -> Capacity)
        {
            if (err) err -> SizeMoreThenCapacity = 1;
            ok = 0;
        }
    }

    if (err) err -> Ok = ok;

    return ok;

}

bool StackNull (const Stack * stack)
{
    assert (stack != nullptr);

    return stack -> Canary1 == 0 &&
           stack -> Data == nullptr &&
           stack -> Capacity == 0 &&
           stack -> Size == 0 &&
           stack -> ItemSize == 0 &&
           stack -> DataHash == 0 &&
           stack -> Canary2 == 0;
}

int getStackDataHash (const Stack* stack)
{
    assert (stack);

    size_t i = stack -> Size * stack -> ItemSize;
    int Hash = 0;

    while (i >= sizeof (int))
    {
        i -= sizeof (int);
        Hash ^= *((int*) (stack -> Data + i));
    }

    if (i >= sizeof (short))
    {
        i -= sizeof (short);
        Hash ^= *((short*) (stack -> Data + i));
    }

    if (i >= sizeof (char))
    {
        i -= sizeof (char);
        Hash ^= *((char*) (stack -> Data + i));
    }
    return Hash;
}

void defaultPrint(FILE * file, const void * item, size_t size)
{
    for (int i = 0; i < size; i ++)
        fprintf(file, "%u ", ((unsigned char*) item)[i]);
}
