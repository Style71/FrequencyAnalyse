/*
 * Message.c
 *
 *  Created on: Mar 22, 2015
 *      Author: QiYang
 */
#include "Message.h"
#include "usart.h"
#include "DataStructure.h"
#include "DSP.h"
#include <cctype>
#include <cstring>

static unsigned char sucMessage_Flags[2] = {NO_MESSAGE, NO_MESSAGE};

static char scRXMessage[2][MESSAGE_SIZE];
extern Queue<unsigned char, 1024> USART1_RX_Stream;
extern Queue<unsigned char, 1024> USART2_RX_Stream;
extern bool channelEnable[3];
extern PrintState stage;
extern int oneshoot_count;
extern FreqWave signal_400Hz_freq;
extern FreqWave signal_100Hz_freq;
extern FreqWave signal_35Hz_freq;

void UpdateMessage()
{
	char cChar;
	static int sIndex = 0;
	Queue<unsigned char, 1024> *RXStream;

	for (int k = 0; k < 2; k++)
	{
		switch (k)
		{
		case 0:
			RXStream = &USART1_RX_Stream;
			break;
		case 1:
			RXStream = &USART2_RX_Stream;
			break;
		default:
			break;
		}
		//If the state machine is in idle state.
		if (sucMessage_Flags[k] == NO_MESSAGE)
		{
			while ((RXStream->size() > 0) && (sucMessage_Flags[k] == NO_MESSAGE))
			{
				//Get a character in the receiving buffer
				cChar = RXStream->pop_front();

				if (cChar == '@')
				{
					//Get a new message, change to busy state.
					sucMessage_Flags[k] = NEW_MESSAGE;
					//Reset the index.
					sIndex = 0;
				}
				//Discard the character if it is not an '@' symbol.
			}
		}

		while ((RXStream->size() > 0) && (sucMessage_Flags[k] == NEW_MESSAGE)) //While there are still characters in the buffer, and the state machine is in busy state.
		{
			cChar = RXStream->pop_front();
			//See if the message is end with '!' or the receiving buffer is full and we still don't get a '!'.
			if ((cChar == '!') || (sIndex > (MESSAGE_SIZE - 2)))
			{
				//Add a terminator '\0' to the end of the string and dispatch the message.
				scRXMessage[k][sIndex] = '\0';
				DispatchMessage(scRXMessage[k]);
				//Change to idle state.
				sucMessage_Flags[k] = NO_MESSAGE;
			}
			else //Else, copy the data to message buffer.
				scRXMessage[k][sIndex++] = cChar;
		}
	}
}

bool DispatchMessage(char *msg)
{
	int sAddress;

	//Get the address.
	if ((isdigit(msg[0])) && (isdigit(msg[1])))
	{
		sAddress = msg[0] - '0';
		sAddress *= 10;
		sAddress += msg[1] - '0';
	}
	else
	{
		//The message is illegal, discard the message.
		//Return false.
		return false;
	}

	if (msg[2] != ':')
	{
		//The message is illegal, discard the message.
		//Return false.
		return false;
	}
	//Jump to corresponding message handle routine.
	switch (sAddress)
	{
	case 1:
		SampleRoutine1(msg + 3);
		break;
	case 2:
		SampleRoutine2(msg + 3);
		break;
	default:
		DefaultRoutine(msg + 3);
	}
	return true;
}

void DefaultRoutine(char *pcString)
{
	USART_Printf(&huart1, "Default channel: %s\n", pcString);
	USART_Printf(&huart2, "Default channel: %s\n", pcString);
}

void SampleRoutine1(char *pcString)
{
	unsigned int enable[3];
	sscanf(pcString, "%u %u %u", &enable[0], &enable[1], &enable[2]);
	if (enable[0] == 0)
		channelEnable[0] = false;
	else
		channelEnable[0] = true;

	if (enable[1] == 0)
		channelEnable[1] = false;
	else
		channelEnable[1] = true;

	if (enable[2] == 0)
		channelEnable[2] = false;
	else
		channelEnable[2] = true;

	USART_Putc(&huart1, '\n');
	USART_Putc(&huart2, '\n');
	for (int i = 0; i < 3; i++)
	{
		USART_Printf(&huart1, "Channel%i: %s \t", i, (channelEnable[i]) ? "Enabled" : "Disabled");
		USART_Printf(&huart2, "Channel%i: %s \t", i, (channelEnable[i]) ? "Enabled" : "Disabled");
	}
	USART_Putc(&huart1, '\n');
	USART_Putc(&huart2, '\n');
}

void SampleRoutine2(char *pcString)
{
	if ((strcmp(pcString, "oneshoot") == 0) || (strcmp(pcString, "Oneshoot") == 0) || (strcmp(pcString, "ONESHOOT") == 0))
	{
		oneshoot_count = 0;
		stage = WaitSample;
		USART_Puts(&huart1, "\nData recording......\n");
		USART_Puts(&huart2, "\nData recording......\n");
	}
}

uint8_t calculateChecksum(uint8_t *msg, uint32_t len)
{
	uint8_t checksum = msg[0];

	for (uint32_t i = 1; i < len; i++)
		checksum ^= msg[i]; // XOR of all bytes.

	return checksum;
}

void sendPackect(uint8_t head, uint8_t *payload, uint32_t payload_len)
{
#define MAX_POSSIBLE_PACKECT_LEN 32
	uint8_t msg[MAX_POSSIBLE_PACKECT_LEN];

	msg[0] = 0x5A;
	msg[1] = head;
	memcpy(&msg[2], payload, payload_len);
	msg[2 + payload_len] = calculateChecksum(msg, payload_len + 2);

	USART_Putchars(&huart1, msg, payload_len + 3);
}

void sendFreqInfo(uint8_t channel, WavePara wave)
{
	float temp;
	uint8_t payload[16];
	uint8_t head;
	switch (channel)
	{
	case 1:
		head = 0x51;
		temp = 1.0 / signal_400Hz_freq.deltaT;
		memcpy(&payload[12], &temp, 4);
		break;
	case 2:
		head = 0x52;
		temp = 1.0 / signal_100Hz_freq.deltaT;
		memcpy(&payload[12], &temp, 4);
		break;
	case 3:
		head = 0x53;
		temp = 1.0 / signal_35Hz_freq.deltaT;
		memcpy(&payload[12], &temp, 4);
		break;
	default:
		head = 0x50;
		break;
	}
	memcpy(&payload[0], &wave.t, 4);
	memcpy(&payload[4], &wave.freq, 4);
	memcpy(&payload[8], &wave.mag, 4);

	sendPackect(head, payload, 16);
}

void sendBatteryInfo(uint8_t channel, WavePara wave)
{
	float temp;
	uint8_t payload[16];
	uint8_t head;
	switch (channel)
	{
	case 1:
		head = 0x51;
		temp = 1.0 / signal_400Hz_freq.deltaT;
		memcpy(&payload[12], &temp, 4);
		break;
	case 2:
		head = 0x52;
		temp = 1.0 / signal_100Hz_freq.deltaT;
		memcpy(&payload[12], &temp, 4);
		break;
	case 3:
		head = 0x53;
		temp = 1.0 / signal_35Hz_freq.deltaT;
		memcpy(&payload[12], &temp, 4);
		break;
	default:
		head = 0x50;
		break;
	}
	memcpy(&payload[0], &wave.t, 4);
	memcpy(&payload[4], &wave.freq, 4);
	memcpy(&payload[8], &wave.mag, 4);

	sendPackect(head, payload, 16);
}