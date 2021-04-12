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

ProtocolStream::ProtocolStream(recv_channel_enable_info_callback &func_channel_enable,
							   recv_battery_info_callback &func_battery,
							   recv_frequency_info_callback &func_frequency)
	: channel_enable_callback(func_channel_enable),
	  battery_callback(func_battery),
	  frequency_callback(func_frequency)
{
	sMessageFlags = NoMessage;
}

ProtocolStream::~ProtocolStream()
{
}

uint8_t ProtocolStream::calculateChecksum(uint8_t *msg, uint32_t len)
{
	uint8_t checksum = msg[0];

	for (uint32_t i = 1; i < len; i++)
		checksum ^= msg[i]; // XOR of all bytes.

	return checksum;
}

void ProtocolStream::send_packet(uint8_t head, uint8_t *payload, uint32_t payload_len)
{
#define MAX_POSSIBLE_PACKECT_LEN 32
	uint8_t msg[MAX_POSSIBLE_PACKECT_LEN];

	msg[0] = 0x5A;
	msg[1] = head;
	memcpy(&msg[2], payload, payload_len);
	msg[2 + payload_len] = calculateChecksum(msg, payload_len + 2);

	USART_Putchars(&huart1, (const char *)msg, payload_len + 3);
}

void ProtocolStream::send_frequency_info(uint8_t channel, WavePara &wave)
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

	send_packet(head, payload, 16);
}

void ProtocolStream::send_battery_info(BatteryStatus &battery)
{
	uint8_t temp;
	uint8_t payload[9];
	uint8_t head = 0x59;

	memcpy(&payload[0], &battery.t, 4);
	memcpy(&payload[4], &battery.voltage, 2);
	memcpy(&payload[6], &battery.current, 2);
	temp = (uint8_t)battery.capacity;
	memcpy(&payload[8], &temp, 1);

	send_packet(head, payload, 9);
}

void ProtocolStream::send_channel_enable_info(bool channelEnable[3])
{
	uint8_t payload[3];
	uint8_t head = 0x5F;

	for (int i = 0; i < 3; i++)
		payload[i] = channelEnable[i] ? 1 : 0;
	send_packet(head, payload, 3);
}
void ProtocolStream::recv_packet(uint8_t head, uint8_t *payload, uint32_t len)
{
	switch (ucPacketHead)
	{
	case 0x51:
	case 0x52:
	case 0x53:

		break;
	case 0x59:

		break;
	case 0x5F:

		break;
	default:
		break;
	}
}

uint8_t ProtocolStream::ParsingMessage(uint8_t *msg, uint8_t len)
{
	uint8_t ret = 0x00;
	bool isReparsing = false;
	uint8_t reparsingHead;

	uint8_t i = 0;
	uint8_t byte = msg[0];
	while (i < len)
	{
		switch (sMessageFlags)
		{
		case NoMessage: // While the state machine is in idle state.
			if (byte == 0x5A)
			{
				// Get a possible packet head, change to incoming state.
				sMessageFlags = MessageIncoming;
			}
			// Discard the character if it is not '0x5A'.
			break;
		case MessageIncoming: // While the state machine is in incoming state.
			if (!isReparsing)
			{
				ucBufferIndex = 0;
				reparsingHead = 0;
				pucReparsingBuffer[ucBufferIndex++] = byte;
			}
			switch (byte)
			{
			case 0x51:
			case 0x52:
			case 0x53:
			case 0x59:
			case 0x5F:
				// Get a packet head, change to new state and record the payload.
				sMessageFlags = ReceivingPayload;
				// Reset the packet message index.
				ucPayloadIndex = 0;
				// Store the packet head.
				ucPacketHead = byte;

				break;
			default:
				// Reset to idle state.
				sMessageFlags = NoMessage;
				// If we fail to parse the packet, repase the input string from the buffer.
				isReparsing = true;
				break;
			}
		case ReceivingPayload: // While the state machine is in payload receiving state.
			if (!isReparsing)
				pucReparsingBuffer[ucBufferIndex++] = byte;

			pucPayload[ucPayloadIndex++] = byte;
			if ((ucPacketHead == 0x51) || (ucPacketHead == 0x52) || (ucPacketHead == 0x53))
			{
				if (ucPayloadIndex == 16)
					sMessageFlags = Checksum; // Set to checksum state.
			}
			if (ucPacketHead == 0x59)
			{
				if (ucPayloadIndex == 9)
					sMessageFlags = Checksum; // Set to checksum state.
			}
			if (ucPacketHead == 0x5F)
			{
				if (ucPayloadIndex == 3)
					sMessageFlags = Checksum; // Set to checksum state.
			}
			break;
		case Checksum: // While the state machine is in checksum state.
			if (calculateChecksum(pucPayload, ucPayloadIndex) == byte)
			{
				recv_packet(ucPacketHead, pucPayload, ucPayloadIndex);
				ret = ucPacketHead;
			}
			else
			{
				if (isReparsing)
			}
			// In both case, reset to idle state.
			sMessageFlags = NoMessage;
			ucPayloadIndex = 0;
			break;
		default:
			break;
		}
		i++;
	}

	return ret;
}

uint8_t ProtocolStream::ParsingMessage(uint8_t *msg, uint8_t len)
{
	bool isReparsing = false;
	uint8_t byte;

	int i = 0;
	while (i < len)
	{
		if (!isReparsing)
		{
			byte = msg[i++];
			enqueue(byte);
		}
		else
		{
			byte = enqueue[ucBufferIndex++];
		}
		// Process byte
		{
			if (getPacket)
				dequeue(PacketLen);
			else
			{
				sMessageFlags = NoMessage;
				ucBufferIndex = 0;
				dequeue one byte;
			}
		}
		if ((sMessageFlags == NoMessage) && (queue.len > 0))
		{
			isReparsing = true;
		}
	}
}