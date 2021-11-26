/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x4C, 0x11, 0xAE, 0xEA, 0xEE, 0xB4};

// Define variables to store User Input to be sent
//int flow=60;
//int settime=60;
//bool onoff=0;

const int PWMPin=26;
//int setpoint =2000; // analogvalue for flow
const int resolution = 12;
const int ledChannel = 1;
const int freq = 5000;
bool mrunning = true;
unsigned long previousMillis = 0;
unsigned long currentMillis=0;

// Define variables to store incoming readings
int incomingID=0;
int incomingflow=2000;
int incomingsettime=60; 
bool incomingonoff=0; 

// Variable to store if sending data was successful
String success;
String MacAddress;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int ID;
    int flow;
    int settime;
    bool onoff;
} struct_message;



// Create a struct_message called ESPSlave to hold the sending info
struct_message ESPSlave;

// Create a struct_message called incomingReadings
struct_message incomingReadings;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingID = incomingReadings.ID; //1= flow, 2=settime 3=onoff //4 what's the pump state
  Serial.print("Incoming ID: ");
  Serial.println(incomingID); 
  if (incomingID==1)
  {
    incomingflow = incomingReadings.flow;
    Serial.print("Incoming flow: ");
    Serial.println(incomingflow); 
    
  }
  else if (incomingID==2)
  {
    incomingsettime = incomingReadings.settime;
    Serial.print("Incoming readings: ");
    Serial.println(incomingsettime); 
    
  }
  else if(incomingID==3)
  {
    incomingonoff = incomingReadings.onoff;
    Serial.print("Incoming onoff: ");
    Serial.println(incomingonoff); 
  }
  else if (incomingID==4)
  {
    //incomingonoff = incomingReadings.onoff;
    //Serial.print("Incoming onoff: ");
    //Serial.println(incomingonoff); 
     Serial.print("Requested Pump state was asked");
     
     //Serial.println(incomingonoff); 
     ESPSlave.ID=1; //update
     ESPSlave.flow = incomingflow;
     ESPSlave.settime = incomingsettime;
     ESPSlave.onoff = incomingonoff;
     // Send message via ESP-NOW
     esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &ESPSlave, sizeof(ESPSlave));  
  }
  
  
}
 
void setup() {
  analogSetWidth(12); //12 bit resolution for ADC writing
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(PWMPin, ledChannel);
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else {
    Serial.println("ESP-NOW initialized");    
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
    // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  esp_now_register_recv_cb(OnDataRecv);
    
  MacAddress = WiFi.macAddress();
  Serial.println("macAddress is: ");
  Serial.println(MacAddress);
  
}
 
void loop() {
  int delayTime=incomingsettime*1000;
  while(incomingonoff == 1)
  {
    if (mrunning)
    {
      currentMillis = millis();
      ledcWrite(ledChannel, incomingflow);
      mrunning = false;
      Serial.println("Pump runs on the max speed");
      Serial.println("Pump running time is: ");
      Serial.println(delayTime);
      previousMillis= millis();
    }
     ledcWrite(ledChannel, incomingflow);
     if (previousMillis-currentMillis >=delayTime)
     {
       mrunning = true;
       Serial.println("Job completed");
       incomingonoff=0;
       // Set values to send
       ESPSlave.ID=1; //update
       ESPSlave.flow = incomingflow;
       ESPSlave.settime = incomingsettime;
       ESPSlave.onoff = incomingonoff;
       // Send message via ESP-NOW
       esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &ESPSlave, sizeof(ESPSlave));     
       break;
     }
     delay(1);
     //Waiting until time is over
     //Serial.print("Countine timer: ");
     //unsigned int long timer= (previousMillis-currentMillis)/1000;
     //Serial.println(timer);
     previousMillis = millis();
   }   
    //Serial.println("Pump runs on the zero speed");
    delay(1);
   
    ledcWrite(ledChannel, 0);
    mrunning = true;
  /*
  // Set values to send
  ESPSlave.flow = flow;
  ESPSlave.settime = settime;
  ESPSlave.onoff = onoff;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &ESPSlave, sizeof(ESPSlave));
  
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }*/
  /*
  delay(1000);
  Serial.print("ID: ");
  Serial.println(incomingID);
  Serial.print("flow: ");
  Serial.println(incomingflow);
  Serial.print("Settime: ");
  Serial.println(incomingsettime);
  Serial.print("Onoff: ");
  Serial.println(incomingonoff);
  Serial.println();*/
}
