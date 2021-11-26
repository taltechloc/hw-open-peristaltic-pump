#ifndef ESPNOW_h
#define ESPNOW_h

#include "Arduino.h"


//int SlaveCnt = 3;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    int ID;
    int flow;
    int settime;
    bool onoff;
} struct_message;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
void InitESPNow();
//bool sendData(int nr);
//void PairWithDevice(uint8_t Address[], int nrPump);

#endif
