#ifndef _MAIN_CONF_H_
#define _MAIN_CONF_H_

#include <IRrecv.h>

#define DATA_JSON_FNAME "/irhid_data.json"

extern bool irStartReceive;
extern bool irReceived;
extern bool irReceiving;
extern bool irStartSend;
extern decode_results results;
extern uint16_t *p_ir_send_data;
extern int ir_send_data_length;

#endif
