/**
  ******************************************************************************
  * File Name          : process.h
  * Description        : This file contains all usart shell process which can be executed by USART command.  
	* Author						 : Qi Yang
	* Date							 : June 26, 2021
  ******************************************************************************
  */
#ifndef PROCESS_H_
#define PROCESS_H_

#define MAX_ARGX 16 // Maximun argguments (MAX_ARGX - 1) getarg() can parse in the string.

#define INIT_SAMPLE_NUM 4000
#define MAX_SAMPLE_NUM 40960
#define MAX_DIGITS 5
#define INIT_DIGITS 3

enum PrintState
{
  Normal = 0,
  WaitSample = 1,
  DumpSample = 2,
  AfterDump = 3
};

void getarg(char str[], int &argc, char *argv[]);

int process_dump(int argc, const char *argv[]);

int process_mode(int argc, const char *argv[]);

int process_ATComm(int argc, const char *argv[]);

int process_ls();

#define PRINTLOOP_FREQ 500
void PrintLoop();

#endif /* PROCESS_H_ */