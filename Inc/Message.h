/*
 * Message.h
 *
 *  Created on: Mar 22, 2015
 *      Author: QiYang
 */
/**
  ******************************************************************************
  * File Name          : Message.h
  * Description        : This file contains all the functions prototypes for 
  *                      the UASRT message dispatch.  
	* Author						 : Qi Yang
	* Date							 : Oct, 31, 2020
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

#define NEW_MESSAGE 0x01
#define NO_MESSAGE 0x02

//This macro defines the maximum message size.
#define MESSAGE_SIZE 64

//This macro defines the maximum protocol payload size.
#define MAX_PAYLOAD_SIZE 16

//*****************************************************************************
//This function fetch the chars remain in RX FIFO, and dispatch the message to corresponding
//message service routines. Usually this routine should be placed in main loop with period less
//than 5ms rather than periodic interrupt service.
//**********************************************************
void UpdateMessage(void);

//*****************************************************************************
// This function dispatch the message to corresponding message service routines. It is called by
// UpdataMessage(), and it should not be called by yourself.
//**********************************************************
bool DispatchMessage(char *msg);

//*****************************************************************************
// The default message service routine. This function is called when the message address does not match
// any service routine number.
//**********************************************************
void DefaultRoutine(char *pcString);

//*****************************************************************************
// The No.1 message service routine. This function is called when the message address matches @01.
//**********************************************************
void SampleRoutine1(char *pcString);

//*****************************************************************************
// The No.2 message service routine. This function is called when the message address matches @02.
//**********************************************************
void SampleRoutine2(char *pcString);

enum MessageParsingState
{
  NoMessage = 0,
  MessageIncoming = 1,
  ReceivingPayload = 2,
  Checksum = 3,
  NewMessage = 4,
};

typedef void(recv_frequency_info_callback)(uint8_t channel, WavePara &wave);
typedef void(recv_battery_info_callback)(BatteryStatus &battery);
typedef void(recv_channel_enable_info_callback)(bool channelEnable[3]);

class ProtocolStream
{
private:
  MessageParsingState sMessageFlags;
  uint8_t pucPayload[MAX_PAYLOAD_SIZE];
  uint8_t ucPayloadIndex;
  uint8_t pucReparsingBuffer[MAX_PAYLOAD_SIZE];
  uint8_t ucBufferIndex;

  uint8_t ucPacketHead;

  recv_channel_enable_info_callback *channel_enable_callback;
  recv_battery_info_callback *battery_callback;
  recv_frequency_info_callback *frequency_callback;

  uint8_t calculateChecksum(uint8_t *msg, uint32_t len);
  void send_packet(uint8_t head, uint8_t *payload, uint32_t payload_len);
  void recv_packet(uint8_t head, uint8_t *payload, uint32_t payload_len);

public:
  ProtocolStream(recv_channel_enable_info_callback &func_channel_enable, recv_battery_info_callback &func_battery, recv_frequency_info_callback &func_frequency);
  ~ProtocolStream();

  uint8_t ParsingMessage(uint8_t *msg, uint8_t len);

  void send_battery_info(BatteryStatus &battery);
  void send_frequency_info(uint8_t channel, WavePara &wave);
  void send_channel_enable_info(bool channelEnable[3]);
};
#endif /* MESSAGE_H_ */
