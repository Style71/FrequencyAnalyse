/**
  ******************************************************************************
  * File Name          : Message.h
  * Description        : This file contains all the functions prototypes for 
  *                      the UASRT message parsing and dispatch.  
	* Author						 : Qi Yang
	* Date							 : Apr 13, 2021
  ******************************************************************************
  */
#include "Message.h"
#include "usart.h"
#include "SysTime.h"
#include "process.h"
#include <cctype>
#include <cstring>

// Extern objects declaration.
extern Queue<unsigned char, USART_TXRX_BUFFER_SIZE> USART2_RX_Stream;

extern bool channelEnable[3];
extern bool virtualVal;

// Function declaration.
void putchars(const char *pucArray, int size);
void recv_frequency_info(uint8_t channel, WavePara &wave);
void recv_battery_info(BatteryStatus &battery);
void recv_channel_enable_info(bool channelEnable[3]);

// Class objects declaration.
MessageStream USBStream;
ProtocolStream BLEStream(putchars, recv_channel_enable_info, recv_battery_info, recv_frequency_info);
ATModeMessage USART_AT_Proc;

// Global variable declaration.
extern FreqWave signal_400Hz_freq;
extern FreqWave signal_100Hz_freq;
extern FreqWave signal_35Hz_freq;

void UpdateMessage()
{
	uint8_t byte;
	if (USART_AT_Proc.isInATMode())
	{
		USART_AT_Proc.ATStateProcess(USART2_RX_Stream);
	}
	else
	{
		while (USART2_RX_Stream.size() > 0)
		{
			byte = USART2_RX_Stream.pop_front();
			USBStream.ParsingMessage(&byte, 1);
			BLEStream.ParsingMessage(&byte, 1);
		}
	}
}

//*****************************************************************************
// The default message service routine. This function is called when the message address does not match
// any service routine number.
//**********************************************************
void DefaultRoutine(const char *pcString)
{
	USART_Printf(&huart2, "Default channel: %s\n", pcString);
}

//*****************************************************************************
// The No.1 message service routine. This function is called when the message address matches @01.
//**********************************************************
void SampleRoutine1(const char *pcString)
{
	unsigned int enable[3];
	sscanf(pcString, "%u %u %u", &enable[0], &enable[1], &enable[2]);
	for (int i = 0; i < 3; i++)
	{
		if (enable[i] == 0)
			channelEnable[i] = false;
		else
			channelEnable[i] = true;
	}

	USART_Putc(&huart2, '\n');
	for (int i = 0; i < 3; i++)
	{
		USART_Printf(&huart2, "Channel%i: %s \t", i, (channelEnable[i]) ? "Enabled" : "Disabled");
	}
	USART_Putc(&huart2, '\n');
}

//*****************************************************************************
// The No.2 message service routine. This function is called when the message address matches @02.
//**********************************************************
void SampleRoutine2(const char *pcString)
{
	char str[64];
	int i = 0;
	int argc;
	char *argv[MAX_ARGX];

	do
	{
		str[i] = pcString[i];
	} while (pcString[i++] != '\0');

	getarg(str, argc, argv);
	if (argc == 0)
		return;

	if (strcmp(argv[0], "dump") == 0)
	{
		process_dump(argc, argv);
	}

	if (strcmp(argv[0], "mode") == 0)
	{
		process_mode(argc, argv);
	}

	if (strcmp(argv[0], "ATComm") == 0)
	{
		process_ATComm(argc, argv);
	}

	if (strcmp(argv[0], "ls") == 0)
	{
		process_ls();
	}
}

//*****************************************************************************
// The No.3 message service routine. This function is called when the message address matches @03.
//**********************************************************
void SampleRoutine3(const char *pcString)
{
	USART_Puts(&huart2, "\nData input changed to ");
	if (virtualVal)
	{
		virtualVal = false;
		USART_Puts(&huart2, "REAL mode.\n");
	}
	else
	{
		virtualVal = true;
		USART_Puts(&huart2, "VIRTUAL mode.\n");
	}
}

//*****************************************************************************
// The No.4 message service routine. This function is called when the message address matches @04.
//**********************************************************
void SampleRoutine4(const char *pcString)
{
	USART_AT_Proc.EnterATMode();
	USART_AT_Proc.TransmitATMessage(pcString);
}

ATModeMessage::ATModeMessage(/* args */)
{
	lastTick = 0;
	isATMode = false;
	state = ATState::Idle;
}

ATModeMessage::~ATModeMessage()
{
}

void ATModeMessage::EnterATMode()
{
	isATMode = true;

	// Clear all incoming message.
	USART2_RX_Stream.pop_front(USART2_RX_Stream.size());
}

void ATModeMessage::ExitATMode()
{
	isATMode = false;
	// Clear all incoming message.
	USART2_RX_Stream.pop_front(USART2_RX_Stream.size());
}

void ATModeMessage::ATStateProcess(Queue<unsigned char, USART_TXRX_BUFFER_SIZE> &InputStream)
{
	uint64_t now = GetUs();
	uint64_t dt = now - lastTick;
	int msgSize;

	switch (state)
	{
	case ATState::WaitToEnterATMode:
		if (dt > DELAY_BEFORE_ENTER_ATMODE_US)
		{
			// Enter AT mode.
			HAL_GPIO_WritePin(BT_AT_GPIO_Port, BT_AT_Pin, GPIO_PIN_RESET);

			state = WaitToSendMessage;
			lastTick = now;
		}
		break;
	case ATState::WaitToSendMessage:
		if (dt > WRITE_DELAY_AFTER_ENTER_ATMODE_US)
		{
			USART_Putchars(&huart2, (const char *)ATModeRXMessage.queue, ATModeRXMessage.size());
			ATModeRXMessage.clear();

			state = WaitReplyMessage;
			lastTick = now;
		}
		break;
	case ATState::WaitReplyMessage:
		msgSize = USART2_RX_Stream.size();
		if (msgSize > 0)
		{
			lastTick = now;
			while (msgSize > 0)
			{
				ATModeRXMessage.push_back(USART2_RX_Stream.pop_front());
				msgSize--;
			}
		}
		else if (((ATModeRXMessage.size() > 0) && (dt > ATMODE_RX_BYTE_ARRIVE_DIFF_TIMEOUT_US)) || ((ATModeRXMessage.size() == 0) && (dt > ATMODE_RX_TIMEOUT_US)))
		{
			lastTick = now;
			state = WaitToPlayback;
			// Exit AT mode.
			HAL_GPIO_WritePin(BT_AT_GPIO_Port, BT_AT_Pin, GPIO_PIN_SET);
		}
		break;
	case ATState::WaitToPlayback:
		if (dt > WRITE_DELAY_AFTER_EXIT_ATMODE_US)
		{
			state = Idle;
			ExitATMode();
			USART_Putchars(&huart2, (const char *)ATModeRXMessage.queue, ATModeRXMessage.size());
		}
		break;
	default:
		break;
	}
}

// Transmit AT message in a delayed manner, the message should terminate with '\0' character.
void ATModeMessage::TransmitATMessage(const char *msg)
{
	// Write AT command to CH9143 after entering AT mode for 800ms.
	ATModeRXMessage.clear();
	// For now we just record the time and command, but didn't send the command.
	lastTick = GetUs();
	int i = 0;
	while (msg[i] != '\0')
		ATModeRXMessage.push_back(msg[i++]);

	ATModeRXMessage.push_back('\r');
	ATModeRXMessage.push_back('\n');

	state = ATState::WaitToEnterATMode;
}

MessageStream::MessageStream()
{
	ucBufferIndex = 0;
}
MessageStream::~MessageStream()
{
}

uint8_t MessageStream::ParsingMessage(uint8_t *msg, uint8_t len)
{
	uint8_t ret = 0x00;
	uint8_t byte;

	int i = 0;
	while (i < len)
	{
		if (ucBufferIndex == cReparsingBuffer.size())
			cReparsingBuffer.push_back(msg[i++]);

		byte = cReparsingBuffer[ucBufferIndex++];
		switch (sMessageFlags)
		{
		case NoMessage: // While the state machine is in idle state.
			if (byte == '@')
			{
				// Get a possible packet head, change to incoming state.
				sMessageFlags = MessageIncoming;
			}
			else // Discard the character if it is not '@'.
			{
				ucBufferIndex = 0;
				cReparsingBuffer.pop_front(1);
			}
			break;
		case MessageIncoming:  // While the state machine is in incoming state.
			if (isdigit(byte)) //Get the address.
			{
				ucPacketHead = byte - '0';
				ucPacketHead *= 10;
				sMessageFlags = DecodeHead;
			}
			else
			{
				//The message is illegal, discard the message and reset to idle state.
				sMessageFlags = NoMessage;
				ucBufferIndex = 0;
				cReparsingBuffer.pop_front(2);
			}
			break;
		case DecodeHead:
			if (isdigit(byte)) //Get the address.
			{
				ucPacketHead += byte - '0';
				// Get a packet head, change to new state.
				sMessageFlags = ReceivingColon;
			}
			else
			{
				//The message is illegal, discard the message and reset to idle state.
				sMessageFlags = NoMessage;
				ucBufferIndex = 0;
				cReparsingBuffer.pop_front(3);
			}
			break;
		case ReceivingColon:
			if (byte == ':') //Get the address.
			{
				// Get colon, change to new state and receiving payload.
				sMessageFlags = ReceivingPayload;
				// Reset the packet message index.
				ucPayloadIndex = 0;
			}
			else
			{
				//The message is illegal, discard the message and reset to idle state.
				sMessageFlags = NoMessage;
				ucBufferIndex = 0;
				cReparsingBuffer.pop_front(4);
			}
			break;
		case ReceivingPayload: // While the state machine is in payload receiving state.
			pucPayload[ucPayloadIndex++] = byte;
			if ((byte == '!') || (ucPayloadIndex >= MAX_MSG_PAYLOAD_SIZE))
			{
				//Add a terminator '\0' to the end of the string and dispatch the message.
				pucPayload[--ucPayloadIndex] = '\0';
				recv_packet(ucPacketHead, pucPayload, ucPayloadIndex);
				cReparsingBuffer.pop_front(ucBufferIndex);
				ucBufferIndex = 0;
				sMessageFlags = NoMessage;
				ret = ucPacketHead;
			}
			break;
		default:
			break;
		}
	}

	return ret;
}

//*****************************************************************************
// This function dispatch the message to corresponding message service routines.
//**********************************************************
void MessageStream::recv_packet(uint8_t head, uint8_t *payload, uint32_t payload_len)
{
	//Jump to corresponding message handle routine.
	switch (head)
	{
	case 1:
		SampleRoutine1((const char *)payload);
		break;
	case 2:
		SampleRoutine2((const char *)payload);
		break;
	case 3:
		SampleRoutine3((const char *)payload);
		break;
	case 4:
		SampleRoutine4((const char *)payload);
		break;
	default:
		DefaultRoutine((const char *)payload);
		break;
	}
}

ProtocolStream::ProtocolStream(put_chars_callback &func_put_chars,
							   recv_channel_enable_info_callback &func_channel_enable,
							   recv_battery_info_callback &func_battery,
							   recv_frequency_info_callback &func_frequency)
	: putc_callback(func_put_chars),
	  channel_enable_callback(func_channel_enable),
	  battery_callback(func_battery),
	  frequency_callback(func_frequency)
{
	sMessageFlags = NoMessage;
	ucBufferIndex = 0;
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

uint8_t ProtocolStream::calculateChecksum(Queue<unsigned char, PACKET_BUFFER_SIZE> &buffer, uint32_t len)
{
	uint8_t checksum = buffer[0];

	for (uint32_t i = 1; i < len; i++)
		checksum ^= buffer[i]; // XOR of all bytes.

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

	putc_callback((const char *)msg, payload_len + 3);
	//USART_Putchars(&huart1, (const char *)msg, payload_len + 3);
}

void ProtocolStream::send_frequency_info(uint8_t channel, WavePara &wave)
{
	float temp;
	uint8_t payload[20];
	uint8_t head;
	switch (channel)
	{
	case 1:
		head = 0x51;
		temp = 1.0 / signal_400Hz_freq.deltaT;
		memcpy(&payload[16], &temp, 4);
		break;
	case 2:
		head = 0x52;
		temp = 1.0 / signal_100Hz_freq.deltaT;
		memcpy(&payload[16], &temp, 4);
		break;
	case 3:
		head = 0x53;
		temp = 1.0 / signal_35Hz_freq.deltaT;
		memcpy(&payload[16], &temp, 4);
		break;
	default:
		head = 0x50;
		break;
	}
	memcpy(&payload[0], &wave.t, 8);
	memcpy(&payload[8], &wave.freq, 4);
	memcpy(&payload[12], &wave.mag, 4);

	send_packet(head, payload, 20);
}

void ProtocolStream::send_battery_info(BatteryStatus &battery)
{
	uint8_t temp;
	uint8_t payload[13];
	uint8_t head = 0x59;

	memcpy(&payload[0], &battery.t, 8);
	memcpy(&payload[8], &battery.voltage, 2);
	memcpy(&payload[10], &battery.current, 2);
	temp = (uint8_t)battery.capacity;
	memcpy(&payload[12], &temp, 1);

	send_packet(head, payload, 13);
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
	WavePara freq;
	BatteryStatus Batt;
	bool channelEnableBytes[3];

	switch (head)
	{
	case 0x51:
	case 0x52:
	case 0x53:
		memcpy(&freq.t, &payload[0], 8);
		memcpy(&freq.freq, &payload[8], 4);
		memcpy(&freq.mag, &payload[12], 4);

		frequency_callback(head & 0x0F, freq);
		break;
	case 0x59:
		memcpy(&Batt.t, &payload[0], 8);
		memcpy(&Batt.voltage, &payload[8], 2);
		memcpy(&Batt.current, &payload[10], 2);
		Batt.capacity = payload[12];

		battery_callback(Batt);
		break;
	case 0x5F:
		for (int i = 0; i < 3; i++)
			channelEnableBytes[i] = (payload[i]) ? true : false;

		channel_enable_callback(channelEnableBytes);
		break;
	default:
		break;
	}
}

void ProtocolStream::recv_packet(uint8_t head, Queue<unsigned char, PACKET_BUFFER_SIZE> &buffer, uint32_t payload_len)
{
	uint8_t payload[PACKET_BUFFER_SIZE];

	for (uint32_t i = 0; i < payload_len; i++)
		payload[i] = buffer[i + 2];

	recv_packet(head, payload, payload_len);
}

uint8_t ProtocolStream::ParsingMessage(uint8_t *msg, uint8_t len)
{
	uint8_t ret = 0x00;
	uint8_t byte;
	uint8_t ucPayloadLen;

	int i = 0;
	while (i < len)
	{
		if (ucBufferIndex == cReparsingBuffer.size())
			cReparsingBuffer.push_back(msg[i++]);

		byte = cReparsingBuffer[ucBufferIndex++];
		switch (sMessageFlags)
		{
		case NoMessage: // While the state machine is in idle state.
			if (byte == 0x5A)
			{
				// Get a possible packet head, change to incoming state.
				sMessageFlags = MessageIncoming;
			}
			else // Discard the character if it is not '0x5A'.
			{
				ucBufferIndex = 0;
				cReparsingBuffer.pop_front(1);
			}
			break;
		case MessageIncoming: // While the state machine is in incoming state.
			switch (byte)
			{
			case 0x51:
			case 0x52:
			case 0x53:
			case 0x59:
			case 0x5F:
				// Get a packet head, change to new state and record the payload.
				sMessageFlags = ReceivingPayload;
				break;
			default:
				// Reset to idle state.
				sMessageFlags = NoMessage;
				// If we fail to parse the packet, discard the first byte and repase the input string from the buffer.
				ucBufferIndex = 0;
				cReparsingBuffer.pop_front(1);
				break;
			}
			break;
		case ReceivingPayload: // While the state machine is in payload receiving state.
			// Get the packet message index.
			ucPayloadLen = ucBufferIndex - 2;
			// Get the packet head.
			ucPacketHead = cReparsingBuffer[1];
			if ((ucPacketHead == 0x51) || (ucPacketHead == 0x52) || (ucPacketHead == 0x53))
			{
				if (ucPayloadLen == 20)
					sMessageFlags = Checksum; // Set to checksum state.
			}
			if (ucPacketHead == 0x59)
			{
				if (ucPayloadLen == 13)
					sMessageFlags = Checksum; // Set to checksum state.
			}
			if (ucPacketHead == 0x5F)
			{
				if (ucPayloadLen == 3)
					sMessageFlags = Checksum; // Set to checksum state.
			}
			break;
		case Checksum: // While the state machine is in checksum state.
			// Get the packet message index.
			ucPayloadLen = ucBufferIndex - 1; // This ucPayloadLen contains 2 bytes head and actual payload bytes, exclude the checksum byte.
			if (calculateChecksum(cReparsingBuffer, ucPayloadLen) == byte)
			{
				recv_packet(ucPacketHead, cReparsingBuffer, ucPayloadLen - 2); // The length here is the payload bytes length so we minus 2.
				cReparsingBuffer.pop_front(ucBufferIndex);
				ret = ucPacketHead;
			}
			else
			{
				cReparsingBuffer.pop_front(1);
			}
			// In both case, reset to idle state.
			sMessageFlags = NoMessage;
			ucBufferIndex = 0;
			break;
		default:
			break;
		}
	}
	return ret;
}

void putchars(const char *pucArray, int size)
{
	// Sending packet via bluetooth.
	USART_Putchars(&huart2, pucArray, size);
}

void recv_frequency_info(uint8_t channel, WavePara &wave)
{
	/*
	FreqWave *pWave;
	switch (channel)
	{
	case 1:
		pWave = &signal_400Hz_freq;
		break;
	case 2:
		pWave = &signal_100Hz_freq;
		break;
	case 3:
		pWave = &signal_35Hz_freq;
		break;
	default:
		break;
	}
	USART_Printf(&huart2, "f%hhu = (%u.%03us, %.2f+-%.2fHz, %.2fmV)\r\n", channel, (uint32_t)(wave.t / 1000000), (uint32_t)((wave.t / 1000) % 1000), wave.freq, 1.0 / pWave->deltaT, wave.mag * 1000);
*/
}

void recv_battery_info(BatteryStatus &battery)
{
	//USART_Printf(&huart2, "Time: %u.%03us, voltage: %humV, current: %humA, capacity: %.2lf%%\r\n", (uint32_t)(battery.t / 1000000), (uint32_t)((battery.t / 1000) % 1000), battery.voltage, battery.current, battery.capacity);
}

void recv_channel_enable_info(bool enable[3])
{
	for (int i = 0; i < 3; i++)
	{
		if (enable[i] == 0)
			channelEnable[i] = false;
		else
			channelEnable[i] = true;
	}

	USART_Putc(&huart2, '\n');
	for (int i = 0; i < 3; i++)
	{
		USART_Printf(&huart2, "Channel%i: %s \t", i, (channelEnable[i]) ? "Enabled" : "Disabled");
	}
	USART_Putc(&huart2, '\n');
}