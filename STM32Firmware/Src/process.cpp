#include <cstring>
#include <cstdlib>
#include "process.h"
#include "getopt.h"
#include "usart.h"
#include "Message.h"

#define PROCESS_NUM 3
const char *processName[PROCESS_NUM] = {"dump", "mode", "ATComm"};

#define stringizing(s) #s
#define xstr(s) stringizing(s)

#define printf(...) USART_Printf(&huart2, #__VA_ARGS__)
#define puts(str) USART_Puts(&huart2, str)
#define putc(char) USART_Putc(&huart2, char)
#define putchars(pucArray,size) USART_Putchars(&huart2, pucArray, size)

float originalInput[MAX_SAMPLE_NUM];
int numOfSample = INIT_SAMPLE_NUM;
int sample_cnt = INIT_SAMPLE_NUM;
int dumpChannel = 1;
PrintState stage = Normal;

static bool isOutputBinary = false;
static int outDigits = INIT_DIGITS;

extern ProtocolStream BLEStream;
extern ATModeMessage USART_AT_Proc;
extern FreqWave signal_400Hz_freq;
extern FreqWave signal_100Hz_freq;
extern FreqWave signal_35Hz_freq;
extern BatteryStatus BattStatus;

//
// getarg
//
// This function parses the input string to argc and argv. No memmory is allocated in the function,
// and char pointers in argv point to corresponding address in str[]. This function will change content
// in str, redundant space, indent and newline characters in str is excluded.
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

void dump_usage()
{
    puts("\nUsage: dump [options <parameter>]\n"
         "Record n ADC samples of the specified channel, and dump with the give format.\n"
         "Options:\n"
         "[-c <channel>]\t\t\t\tChannel selection.(1-RFID, 2-VIN, 3-IMON, default: 1)\n"
         "[-n <number>]\t\t\t\tNumber of samples to be dumped.(1~" xstr(MAX_SAMPLE_NUM) ", default: " xstr(INIT_SAMPLE_NUM) ")\n"
                                                                                                                            "\n"
                                                                                                                            "[-f <format>]\t\t\t\tData output format.( For channel 1, default: .3f, for channel 2,3, dedfault: u )\n"
                                                                                                                            "\tb\t\t\t\t32-bits float binary or 16-bits uint16_t binary, depending on the channel\n"
                                                                                                                            "\t.Xf\t\t\t\t32-bits float text with X(0~" xstr(MAX_DIGITS) ", default:" xstr(INIT_DIGITS) ") digits, for channel 1 only\n"
                                                                                                                            "\tu\t\t\t\t16-bits uint16_t text, for channel 2 or 3 only\n"
                                                                                                                                                                                                                        "\n"
                                                                                                                                                                                                                        "[-h]\t\t\t\t\tDisplay this information.\n");
}

int process_dump(int argc, const char *argv[])
{
    int ch;
    // Initialize global configurations.
    dumpChannel = 1;
    numOfSample = INIT_SAMPLE_NUM;
    isOutputBinary=false;
    outDigits=INIT_DIGITS;

    bool isPrintUsage = false;
    // Parse options.
    while ((ch = getopt(argc, argv, "c:f:n:h")) != -1)
    {
        switch (ch)
        {
        case 'h':
            isPrintUsage = true;
            break;
        case 'c':
            dumpChannel = atoi(optarg);
            if ((dumpChannel > 3) || (dumpChannel < 1))
            {
                printf("\nError： option -%c get channel %i, channel between 1~3 is required.\n", ch, dumpChannel);
                isPrintUsage = true;
            }
            break;
        case 'f':
            if (strcmp(optarg, "b") == 0)
            {
                isOutputBinary = true;
            }
            else if ((optarg[0] == '.') && ((optarg[1] >= '0') && (optarg[1] <= '9')) && (optarg[2] == 'f'))
            {
                isOutputBinary = false;
                if ((optarg[1] >= '0') && (optarg[1] <= ('0' + MAX_DIGITS)))
                    outDigits = optarg[1] - '0';
                else
                    printf("\nError： option -%c digits %i out of range 0~" xstr(MAX_DIGITS) ".\n", ch, optarg[1] - '0');
            }
            else
            {
                printf("\nError： option -%c get illegal format %s.\n", ch, optarg);
                isPrintUsage = true;
            }
            break;
        case 'n':
            numOfSample = atoi(optarg);
            if ((numOfSample > MAX_SAMPLE_NUM) || (numOfSample < 1))
            {
                printf("\nError： option -%c get number %i, sample between 1~" xstr(MAX_SAMPLE_NUM) " is required.\n", ch, numOfSample);
                isPrintUsage = true;
            }
            break;
        case '?':
            if ((optopt == 'c') || (optopt == 'f') || (optopt == 'n'))
                printf("Option -%c requires an argument.\n", optopt);
            else
                printf("Unknown option `-%c'.\n", optopt);
            isPrintUsage = true;
            break;
        }
    }
    // If command error ocurrs or '-h' option is selected, print usage and exit. 
    if (isPrintUsage)
    {
        dump_usage();
        return -1;
    }
    // Output configurations.
    printf("\nData recording......\nChannel: %i, number of sample: %i, format: %s", dumpChannel, numOfSample, isOutputBinary ? "binary" : "text");
    if (isOutputBinary)
        printf(", digits: %i\n", outDigits);
    else
        putc('\n');
    // Start sampling.
    sample_cnt = 0;
    stage = WaitSample;
    
    return 0;
}

int process_mode(int argc, const char *argv[])
{
    return 0;
}

int process_ATComm(int argc, const char *argv[])
{
    return 0;
}

int process_ls()
{
    puts("\n>bin/\n");
    for (int i = 0; i < PROCESS_NUM; i++)
    {
        printf("%s\n", processName[i]);
    }
    putc('\n');

    return 0;
}

void PrintLoop()
{
    static int print_cnt = 0;
    static int wait_cnt = 0;
    WavePara para;

    static int loop_cnt = 0;

    char *float2chars;

    switch (stage)
    {
    case Normal:
        if (!signal_400Hz_freq.freq.isEmpty())
        {
            para = signal_400Hz_freq.freq.pop_front();
            if ((channelEnable[0]) && (!USART_AT_Proc.isInATMode()))
            {
                BLEStream.send_frequency_info(1, para);
                printf("f1 = (%u.%03us, %.2f+-%.2fHz, %.2fmV)\r\n", (uint32_t)(BattStatus.t / 1000000), (uint32_t)((BattStatus.t / 1000) % 1000), para.freq, 1.0 / signal_400Hz_freq.deltaT, para.mag * 1000);
                //USART_Printf(&huart1, "f1 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_400Hz_freq.deltaT, para.mag * 1000);
            }
        }
        if (!signal_100Hz_freq.freq.isEmpty())
        {
            para = signal_100Hz_freq.freq.pop_front();
            if ((channelEnable[1]) && (!USART_AT_Proc.isInATMode()))
            {
                BLEStream.send_frequency_info(2, para);
                printf("f2 = (%u.%03us, %.2f+-%.2fHz, %.2fmV)\r\n", (uint32_t)(BattStatus.t / 1000000), (uint32_t)((BattStatus.t / 1000) % 1000), para.freq, 1.0 / signal_100Hz_freq.deltaT, para.mag * 1000);
                //USART_Printf(&huart1, "f2 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_100Hz_freq.deltaT, para.mag * 1000);
            }
        }
        if (!signal_35Hz_freq.freq.isEmpty())
        {
            para = signal_35Hz_freq.freq.pop_front();
            if ((channelEnable[2]) && (!USART_AT_Proc.isInATMode()))
            {
                BLEStream.send_frequency_info(3, para);
                printf("f3 = (%u.%03us, %.2f+-%.2fHz, %.2fmV)\r\n", (uint32_t)(BattStatus.t / 1000000), (uint32_t)((BattStatus.t / 1000) % 1000), para.freq, 1.0 / signal_35Hz_freq.deltaT, para.mag * 1000);
                //USART_Printf(&huart1, "f3 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_35Hz_freq.deltaT, para.mag * 1000);
            }
        }

        if ((loop_cnt == 0) && (!USART_AT_Proc.isInATMode()))
        {
            BLEStream.send_battery_info(BattStatus);
            printf("Time: %u.%03us, voltage: %humV, current: %humA, capacity: %.2lf%%\r\n", (uint32_t)(BattStatus.t / 1000000), (uint32_t)((BattStatus.t / 1000) % 1000), BattStatus.voltage, BattStatus.current, BattStatus.capacity);
        }
        break;

    case WaitSample:
        // Wait samples to complete.
        if (sample_cnt >= numOfSample)
        {
            stage = DumpSample;
            print_cnt = 0;
        }
        break;

    case DumpSample:
        // Dump one sample at a time.
        if (print_cnt < numOfSample)
        {
            if (isOutputBinary)
            {
                float2chars=(char*)&originalInput[print_cnt];
                if (dumpChannel==1)
                    putchars(float2chars,4);
                else if (dumpChannel==2)
                    putchars(float2chars,2);
                else if (dumpChannel==3)
                    putchars(float2chars,2);                
            }
            else
            {
                if (dumpChannel==1)
                    printf("%.*f\t", outDigits,originalInput[print_cnt++]);
                else if (dumpChannel==2)
                    printf("%.*f\t", outDigits,originalInput[print_cnt++]);
                else if (dumpChannel==3)
                    putchars(float2chars,2); 
                
            }
                
        }
        else
            stage = AfterDump;
        break;

    case AfterDump:
        // Add 3 seconds delay before output any regular data.
        if (wait_cnt < PRINTLOOP_FREQ * 3)
            wait_cnt++;
        else
        {
            wait_cnt = 0;
            stage = Normal;
        }
        break;

    default:

        break;
    }
    loop_cnt++;
    if (loop_cnt >= PRINTLOOP_FREQ)
        loop_cnt = 0;
}