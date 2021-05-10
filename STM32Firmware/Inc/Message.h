/**
  ******************************************************************************
  * File Name          : Message.h
  * Description        : This file contains all the functions prototypes for 
  *                      the UASRT message parsing and dispatch.  
	* Author						 : Qi Yang
	* Date							 : Apr 13, 2021
  ******************************************************************************
  */
#ifndef MESSAGE_H_
#define MESSAGE_H_

//#include <stdint.h>
//#include <ctype.h>
//#include <stdbool.h>
//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
#include "DataStructure.h"
#include "DSP.h"

#define NEW_MESSAGE 0x01
#define NO_MESSAGE 0x02

//This macro defines the maximum message size.
#define MESSAGE_SIZE 64

//This macro defines the maximum protocol payload size.
#define MAX_MSG_PAYLOAD_SIZE 64
#define MAX_PACKET_PAYLOAD_SIZE 20

//*****************************************************************************
//This function fetch the chars remain in RX FIFO, and dispatch the message to corresponding
//message service routines. Usually this routine should be placed in main loop with period less
//than 5ms rather than periodic interrupt service.
//**********************************************************
void UpdateMessage(void);

enum ParsingState
{
  NoMessage = 0,
  MessageIncoming = 1,
  DecodeHead = 2,
  ReceivingColon = 3,
  ReceivingPayload = 4,
  Checksum = 5,
};

class MessageStream
{
private:
  ParsingState sMessageFlags;
  uint8_t pucPayload[MAX_MSG_PAYLOAD_SIZE];
  uint8_t ucPayloadIndex;
  class Queue<unsigned char, MESSAGE_BUFFER_SIZE> cReparsingBuffer;
  uint8_t ucBufferIndex;

  uint8_t ucPacketHead;

  void recv_packet(uint8_t head, uint8_t *payload, uint32_t payload_len);

public:
  MessageStream();
  ~MessageStream();

  uint8_t ParsingMessage(uint8_t *msg, uint8_t len);
};

typedef void(recv_frequency_info_callback)(uint8_t channel, WavePara &wave);
typedef void(recv_battery_info_callback)(BatteryStatus &battery);
typedef void(recv_channel_enable_info_callback)(bool channelEnable[3]);
typedef void(put_chars_callback)(const char *pucArray, int size);

class ProtocolStream
{
private:
  ParsingState sMessageFlags;

  Queue<unsigned char, PACKET_BUFFER_SIZE> cReparsingBuffer;
  uint8_t ucBufferIndex;

  uint8_t ucPacketHead;

  recv_channel_enable_info_callback *channel_enable_callback;
  recv_battery_info_callback *battery_callback;
  recv_frequency_info_callback *frequency_callback;
  put_chars_callback *putc_callback;

  uint8_t calculateChecksum(uint8_t *msg, uint32_t len);
  uint8_t calculateChecksum(Queue<unsigned char, PACKET_BUFFER_SIZE> &buffer, uint32_t len);
  void send_packet(uint8_t head, uint8_t *payload, uint32_t payload_len);
  void recv_packet(uint8_t head, uint8_t *payload, uint32_t payload_len);
  void recv_packet(uint8_t head, Queue<unsigned char, PACKET_BUFFER_SIZE> &buffer, uint32_t payload_len);

public:
  ProtocolStream(put_chars_callback &func_put_chars, recv_channel_enable_info_callback &func_channel_enable, recv_battery_info_callback &func_battery, recv_frequency_info_callback &func_frequency);
  ~ProtocolStream();

  uint8_t ParsingMessage(uint8_t *msg, uint8_t len);

  void send_battery_info(BatteryStatus &battery);
  void send_frequency_info(uint8_t channel, WavePara &wave);
  void send_channel_enable_info(bool channelEnable[3]);
};

class ATModeMessage
{
  enum ATState
  {
    Idle = 0,
    WaitToEnterATMode = 1,
    WaitToSendMessage = 2,
    WaitReplyMessage = 3,
    WaitToPlayback = 4,
  };

#define ATMODE_RX_TIMEOUT_US 2000000
#define ATMODE_RX_BYTE_ARRIVE_DIFF_TIMEOUT_US 200000
#define DELAY_BEFORE_ENTER_ATMODE_US 500000
#define WRITE_DELAY_AFTER_ENTER_ATMODE_US 800000
#define WRITE_DELAY_AFTER_EXIT_ATMODE_US 800000

private:
  bool isATMode;
  Queue<unsigned char, MESSAGE_BUFFER_SIZE> ATModeRXMessage;
  uint64_t lastTick;
  ATState state;

public:
  ATModeMessage(/* args */);
  ~ATModeMessage();
  void EnterATMode();
  void ExitATMode();
  bool isInATMode() { return isATMode; }
  uint8_t ATStateProcess(Queue<unsigned char, USART_TXRX_BUFFER_SIZE> &InputStream);
  // Transmit AT message in a delayed manner, the message should terminate with '\0' character.
  void TransmitATMessage(const char *msg);
};

extern ProtocolStream BLEStream;
#endif /* MESSAGE_H_ */
