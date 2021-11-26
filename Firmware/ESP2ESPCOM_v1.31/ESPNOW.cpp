#include <esp_now.h>
#include <WiFi.h>
#include "Arduino.h"
#include "ESPNOW.h"

struct_message incomingReadings;
int incomingID=0;
int incomingflow=0;
int incomingsettime=0; 
bool incomingonoff=0; 


// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingID = incomingReadings.ID; //1= flow, 2=settime 3=onoff //4 what's the pump state
  if (incomingID==1)
  {
    incomingflow = incomingReadings.flow;
    incomingsettime = incomingReadings.settime;
    incomingonoff = incomingReadings.onoff;
    Serial.print("ID: ");
    Serial.println(incomingID);
    Serial.print("flow: ");
    Serial.println(incomingflow);
    Serial.print("Settime: ");
    Serial.println(incomingsettime);
    Serial.print("Onoff: ");
    Serial.println(incomingonoff);
    Serial.println();
    
  }

  else
  {
      Serial.println("Recived message has different ID than 1");
  }
  
  
}
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
    // Once ESPNow is successfully Init, we will register for recv CB to
    esp_now_register_recv_cb(OnDataRecv); 
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}
