#include "Logs.h"

#include <stdio.h>
#include <assert.h>

extern FILE* StackLogs;

void OpenLogs (void)
{
    StackLogs = fopen (LOG_FILE_NAME, "a");
    fprintf (StackLogs, "Built on: %s at %s\n\n", __DATE__, __TIME__);
    assert (StackLogs);
}

void CloseLogs (void)
{
    fprintf (StackLogs, "\n");
    fclose (StackLogs);
}
