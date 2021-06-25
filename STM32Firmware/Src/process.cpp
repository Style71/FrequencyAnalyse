#include "getopt.h"
#include "usart.h"
//
// getarg
//
// This function parses the input string to argc and argv. No memmory is allocated in the function,
// and char pointers in argv point to corresponding address in str[]. This function will change content
// in str, redundant space, indent and newline characters in str is excluded.
#define MAX_ARGX 16
void getarg(char str[], int &argc, char *argv[])
{
    int head = 0;
    int tail = 0;
    argc = 0;

    while (str[head] != '\0')
    {
        // Discard heading space, indent, newline characters in the string.
        while (((str[head] == ' ') || (str[head] == '\t') || (str[head] == '\n')) && (str[head] != '\0'))
            head++;

        if (str[head] != '\0')
        {
            // Copy argument address
            argv[argc++] = &str[tail];
            // Copy arguments
            while ((str[head] != ' ') && (str[head] != '\n') && (str[head] != '\t') && (str[head] != '\0'))
            {
                str[tail++] = str[head++];
            }
            // Check terminator first
            if ((str[head] == '\0') || ((argc + 1) >= MAX_ARGX))
            { // Parsing complete, add a terminator to the last argument string and exit.
                // We shoul first check str[head] == '\0' then add '\0' to str[tail], otherwise we may exit before parsing complete when head equals to tail.
                str[tail++] = '\0';
                break;
            }
            // Add a terminator to the last argument string.
            str[tail++] = '\0';
            // After tail increment, head may be less than tail and point to the padded '\0', so we need to increase head to skip this terminator.
            if (head < tail)
                head = tail;
        }
    }
    // Accoording to POSIX standard, argv[argc] should be a NULL pointer.
    argv[argc] = NULL;
}

#define PROCESS_NUM 3
char *processName[PROCESS_NUM] = {"dump", "mode", "ATComm"};

void dump_usage()
{
    USART_Puts(&huart2, "\nUsage: dump [options <parameter>]\n"
                        "Record n ADC samples of the specified channel, and dump with the give format.\n"
                        "Options:\n"
                        "[-c <channel>]\t\t\t\tChannel selection.(e.g. 1,2,3, default: 1)\n"
                        "[-n <number>]\t\t\t\tNumber of samples to be dumped.(1~40000, default: 4000)\n"
                        "\n"
                        "[-f <format>]\t\t\t\tData output format.(default: .3f)\n"
                        "\tb\t\t\t\t32-bits float binary\n"
                        "\t.Xf\t\t\t\t32-bits float text with X digits\n"
                        "\n"
                        "[-h]\t\t\t\t\tDisplay this information.\n");
}
int process_dump(argc, argv)
{

    return 0;
}

int process_mode(argc, argv)
{
    return 0;
}
int process_ATComm(argc, argv)
{
    return 0;
}

int process_ls()
{
    USART_Puts(&huart2, "\n>bin/\n");
    for (int i = 0; i < PROCESS_NUM; i++)
    {
        USART_Printf(&huart2, "%s\n", processName[i]);
    }
    USART_Putc(&huart2, '\n');

    return 0;
}