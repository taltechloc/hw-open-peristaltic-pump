/*
 Create a WiFi access point and provide a web server on it. 
For tutorial video of this code, watch out this video
https://youtu.be/fcmb_3aBfH4
  visit 
  http://www.techiesms.com
  for IoT project tutorials.
  
        #techiesms
  explore | learn | share
*/

// Import required libraries
#ifdef ESP32
  #include <ArduinoJson.h>
  #include <WiFi.h>
  #include <SPIFFS.h>
  #include <ESPAsyncWebServer.h>
  /*
  //#include <WebSocketsServer.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <FS.h>*/
#endif

#include <esp_now.h>
#include "ESPNOW.h"
#include "MACaddress.h"

#define DEBUG true
#define DEBUG2 false

/*Communication specific variable*/
/* Set these to your desired credentials. */
const char *ssid = "SmartFlowController2"; // You can change it according to your ease
const char *password = "Taltech2020"; // You can change it according to your ease
/* Put IP Address details */
IPAddress local_ip(192,168,5,1);
IPAddress gateway(192,168,5,1);
IPAddress subnet(255,255,255,0);
//const char* PARAM_MESSAGE = "message";
const int dns_port = 53;
const int http_port = 80;
//const int ws_port = 1337;//websocket port

char msg_buf[15];
int pumpState = 0;

/*Webserver*/
AsyncWebServer server(http_port); // establishing server at port 80 (HTTP protocol's default port)
AsyncWebSocket webSocket("/ws");
//ESP-NOW

struct_message myData[3];

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress1[] = {0x4C, 0x11, 0xAE, 0xEA, 0xEE, 0xE8};
uint8_t broadcastAddress2[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t broadcastAddress3[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define NUMSLAVES 3
esp_now_peer_info_t slaves[NUMSLAVES] = {};

int flow=50;
int settime=50;
const int resolution = 12;
const int ledChannel = 1;
const int freq = 5000;

bool mrunning = true;

const int PWMPin=26;
int setpoint=2000;

unsigned long previousMillis=millis();
unsigned long currentMillis=millis();

bool sendData(int nr) {
  const uint8_t *peer_addr = slaves[nr].peer_addr;
    esp_err_t result = esp_now_send(peer_addr,(uint8_t *)&myData[nr], sizeof(myData[nr]));
    Serial.print("Send Status: ");
    if (result == ESP_OK) {
      Serial.println("Success");
      return true;
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW not Init.");
      return false;
    } else if (result == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
      return false;
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
      Serial.println("Internal Error");
      return false;
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
      return false;
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
      return false;
    } else {
      Serial.println("Not sure what happened");
      return false;
    }
    delay(100);
}
// Initilizeing ESPNOW and checking if the device is paired, if not, then it's going to be paired.
bool PairWithDevice(uint8_t Address[], int nrPump)
{
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  Serial.println("Entering esp_now_register_send_cb");
  esp_now_register_send_cb(OnDataSent);

  Serial.println("Registering peer");
  // Register peer

  memcpy(slaves[nrPump].peer_addr, Address, 6);
  slaves[nrPump].channel = 0;  
  slaves[nrPump].encrypt = false;

  // check if the peer exists
  bool exists = esp_now_is_peer_exist(slaves[nrPump].peer_addr);
  if (exists) {
    // Slave already paired.
    Serial.println("Already Paired");
  }
  else{
  Serial.println("Adding peer");
  // Add peer
  // Slave not paired, attempt pair
  esp_err_t addStatus = esp_now_add_peer(&slaves[nrPump]);        
  if (addStatus = ESP_OK){
    Serial.print("Pairing with device was succesful.");
    return true;
  } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
    return false;
  } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Add Peer - Invalid Argument");
    return false;
  } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
    Serial.println("Peer list full");
    return false;
  } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("Out of memory");
    return false;
  } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
    Serial.println("Peer Exists");
    return true;
  } else {
    Serial.println("Not sure what happened");// peerinfo must be global variable
    return false;
  }

  delay(100);
}
}

void SendMessageUser(bool input, AsyncWebSocketClient * client)
{
    if (input == true) {
    Serial.println("Sent with success to Pump2");
    sprintf(msg_buf, "%d", "Sent with success");
    Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
    client->text(msg_buf);
  }
  else {
    Serial.println("Error sending data to Pump2");
    sprintf(msg_buf, "%d", "Error connecting or sending data to pump2");
    Serial.printf("Sending to [%u]: %s\n", client, "Error connecting or sending data to pump2");
    client->text("Error connecting or sending data to pump2");
  } 
}  
//char MACaddress(String input,char result[]);

// Callback: receiving any WebSocket message
void onWebSocketEvent(AsyncWebSocket * server, 
                      AsyncWebSocketClient * client, 
                      AwsEventType type, void * arg, 
                      uint8_t *data, size_t length) {
  // Figure out the type of WebSocket event
  switch(type) 
  {
    // Client has disconnected
    case WS_EVT_DISCONNECT:
      Serial.printf("[%u] Disconnected!\n", client);
      break;
    // New client has connected
    case WS_EVT_CONNECT:
      {
        //IPAddress ip = webSocket.remoteIP(client);
        Serial.printf("[%u] Connection from ", client);
        //Serial.println(ip.toString());
      }
      break;
    // Handle text messages from client
    
    //For everything else: do nothing
    case WS_EVT_PONG:
    {
      Serial.println("[%u] Message type: WS_EVT_PONG");
      break;
    }   
    case WS_EVT_DATA:
    {
      AwsFrameInfo * info = (AwsFrameInfo*)arg;
      if (DEBUG)
      {
        Serial.printf("[%u] Received text: %s\n", client, data);
      }
      //StaticJsonDocument<200> doc;
      DynamicJsonDocument doc(1024);
      
      uint8_t* input = data;
      DeserializationError error=deserializeJson(doc, input);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }
      //JsonObject obj = doc.as<JsonObject>();
      const char* ID = doc["ObjectID"];
      if (DEBUG)
      {      
        Serial.println(doc["ObjectID"].as<char*>());
        Serial.printf("[%u] Received ObjectID: %s\n", ID);
        /*
        const char* message = doc["message"];
        Serial.println(doc["message"].as<String>());
        Serial.printf("[%u] Received message: %s\n", message);*/
        String mes =doc["message"];
        Serial.println(mes);
        
      }
      // Toggle LED
      if (strcmp(ID,"togglePump1")==0) 
      {  
        //pumpState = pumpState ? 0 : 1;
        int Recieved_message = doc["message"];
        pumpState=Recieved_message;
        if (DEBUG)
        {           
         Serial.printf("Toggling Pump to %u\n", pumpState);
        }
      }
      // Report the state of the Pump
      else if (strcmp(ID,"getPumpState1")==0) 
      {
        if (DEBUG)
        {    
        sprintf(msg_buf, "%d", pumpState);
        Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
        }
        //webSocket.sendTXT(client, msg_buf);
        client->text("Recieved get pumpstate for pump 1");
        mrunning = true;
      }
      else if (strcmp(ID,"sendPumpTime1")==0) 
      {
        int Recieved_message = doc["message"];
        settime=Recieved_message;
        if (DEBUG)
        {         
        sprintf(msg_buf, "%d", settime);
        Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
        }
        //webSocket.sendTXT(client, msg_buf);
        client->text("Recieved Pump 1 time: ");
        client->text(msg_buf);
      }
      else if (strcmp(ID,"sendPumpFlow1")==0) 
      {
        int Recieved_message = doc["message"];
        setpoint=Recieved_message;
        if (DEBUG)
        {   
        sprintf(msg_buf, "%d", setpoint);
        Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
        }
        client->text("Recieved Pump 1 flow: ");
        client->text(msg_buf);
        //webSocket.sendTXT(client, msg_buf);
           
      }
      else if (strcmp(ID,"sendMACAdress2")==0) 
      { 
        Serial.println("It went to sendMACAdress2 if statement"); 
        String RecAddress1 =doc["message"];
        Serial.print("Recieved message: ");
        Serial.println(RecAddress1);
        char result[6]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        MACaddress(RecAddress1,result);

        Serial.print("MAC id is: ");               
        for (int i=0; i <6; ++i)
        {
          broadcastAddress1[i]=result[i];
          Serial.print(result[i]);
          if (i!=5)  
          {       
            Serial.print(", ");
          }
        }
        Serial.println(" ");     
        //client->text(msg_buf);
        //webSocket.sendTXT(client, msg_buf);
        for (int i=0;i <6; ++i)
        {
           Serial.print(broadcastAddress1[i]);
           if(i!=5)
           {
            Serial.print(", ");
           }
        }
        Serial.println(" ");
        bool Paired=PairWithDevice(broadcastAddress1,0);
        if (Paired)
        {
          esp_now_register_send_cb(OnDataSent);
          
          client->text("Paired");
        }
        else
        {
          client->text("NotPaired");
        }
      }
      // Toggle LED
      if (strcmp(ID,"togglePump2")==0) 
      {  
        //pumpState = pumpState ? 0 : 1;
        int Recieved_message= doc["message"];
        
        if (DEBUG)
        {           
         Serial.printf("Toggling Pump 2 to %u\n",Recieved_message);
        }
        // Send message via ESP-NOW
          // Set values to send
        myData[0].ID = 3;
        myData[0].onoff = Recieved_message;
        
        // Send message via ESP-NOW
        bool res = sendData(0);
        SendMessageUser(res,client);
       
        
      }
      // Report the state of the Pump
      else if (strcmp(ID,"getPumpState2")==0) 
      {
        int Recieved_message = doc["message"];
        if (DEBUG)
        {    
          sprintf(msg_buf, "%d", Recieved_message);
          Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
          client->text("Sent state ifno to Pump 2");
        }
        myData[0].ID = 4;

        // Send message via ESP-NOW
        bool res = sendData(0);
        SendMessageUser(res,client);
      }
      else if (strcmp(ID,"sendPumpTime2")==0) 
      {
        int Recieved_message = doc["message"];
        myData[0].ID = 2;
        myData[0].settime=Recieved_message;
        if (DEBUG)
        {         
          sprintf(msg_buf, "%d", Recieved_message);
          Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
          client->text("Sent Pump 2 time: ");
          client->text(msg_buf);
        }
        //webSocket.sendTXT(client, msg_buf);

        bool res = sendData(0);
        SendMessageUser(res,client);
      }
      else if (strcmp(ID,"sendPumpFlow2")==0) 
      {
        int Recieved_message = doc["message"];
        myData[0].ID = 1;
        myData[0].flow=Recieved_message;
        if (DEBUG)
        {   
          sprintf(msg_buf, "%d", Recieved_message);
          Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
          client->text("send pump flow to Pump 2: ");
          client->text(msg_buf);
        }

        //webSocket.sendTXT(client, msg_buf);
        bool res = sendData(0);
        SendMessageUser(res, client);
           
      }
      else if (strcmp(ID,"sendMACAdress3")==0) 
      { 
        Serial.println("It went to sendMACAdress3 if statement"); 
        String RecAddress1 =doc["message"];
        Serial.print("Recieved message: ");
        Serial.println(RecAddress1);
        char result[6]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        MACaddress(RecAddress1,result);

        Serial.print("MAC id is: ");               
        for (int i=0; i <6; ++i)
        {
          broadcastAddress2[i]=result[i];
          Serial.print(result[i]);
          if (i!=5)  
          {       
            Serial.print(", ");
          }
        }
        Serial.println(" ");     
        
        //webSocket.sendTXT(client, msg_buf);
        for (int i=0;i <6; ++i)
        {
           Serial.print(broadcastAddress2[i]);
           if(i!=5)
           {
            Serial.print(", ");
           }
        }
        Serial.println(" ");
        bool Paired=PairWithDevice(broadcastAddress2,0);
        if (Paired)
        {
          client->text("Paired2");
        }
        else
        {
          client->text("NotPaired2");
        }
      }
      // Toggle LED
      if (strcmp(ID,"togglePump3")==0) 
      { 
        int Recieved_message = doc["message"]; 
        //pumpState = pumpState ? 0 : 1;
        if (DEBUG)
        {           
         Serial.printf("Toggling Pump to %u\n", Recieved_message);
        }
        // Send message via ESP-NOW
          // Set values to send
        myData[1].ID = 3;
        myData[1].onoff = Recieved_message;
        
        // Send message via ESP-NOW
        bool res = sendData(1);
        SendMessageUser(res,client);
      }
      // Report the state of the Pump
      else if (strcmp(ID,"getPumpState3")==0) 
      {
        if (DEBUG)
        {    
          sprintf(msg_buf, "%d", "Getting Pump state for pump 3");
          Serial.printf("Sending to [%u]: %s\n", client, "Getting Pump state for pump 3");
          client->text("Getting Pump state for pump 3");
        }

        myData[1].ID = 4;
        
        // Send message via ESP-NOW
        bool res = sendData(1);
        SendMessageUser(res,client);
      }
      else if (strcmp(ID,"sendPumpTime3")==0) 
      {
        int Recieved_message = doc["message"];
        myData[1].ID = 2;
        myData[1].settime=Recieved_message;
        if (DEBUG)
        {         
          sprintf(msg_buf, "%d", Recieved_message);
          Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
          client->text("Recieved pump time for pump 3: ");
          client->text(msg_buf);
        }

        bool res = sendData(1);
        SendMessageUser(res,client);
      }
      else if (strcmp(ID,"sendPumpFlow3")==0) 
      {
        int Recieved_message = doc["message"];
        myData[1].ID = 1;
        myData[1].flow=Recieved_message;
        if (DEBUG)
        {   
          sprintf(msg_buf, "%d", Recieved_message);
          Serial.printf("Sending to [%u]: %s\n", client, msg_buf);
          client->text("Recieved pump flow for pump 3: ");
          client->text(msg_buf);
        }
        
        //webSocket.sendTXT(client, msg_buf);
        bool res = sendData(1);
        SendMessageUser(res, client);
           
      }      
      // Message not recognized
      else 
      {
        Serial.println("[%u] Message not recognized");
        
      }
      break;
    }
  
    default:
    break;
  }
} 
     
/*The files which are sent from webservers memory*/
// Callback: send homepage
void onIndexRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();              
  request->send(SPIFFS, "/Index.html", "text/html");
  if (DEBUG)
  {
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  }
}
 // Callback: send style sheet
void onCSSRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/main.css", "text/css");
  if (DEBUG)
  {  
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  } 
}
// Callback: send jquery
void onjqueyRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/jquery.js", "application/js");
  if (DEBUG)
  {
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  }
}
// Callback: send myJsFnctions
void onmyFunctionsRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();             
  request->send(SPIFFS, "/myJsFunctions.js", "application/js");
  if (DEBUG)
  {  
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  } 
}
// Callback: send 404 if requested file does not exist
void onPageNotFound(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(404, "text/plain", "Not found");
  if (DEBUG)
  {   
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  } 
}

void setup() {

  // PWM for Motor
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(PWMPin, ledChannel);
  
  Serial.begin(115200);

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    while(1);
  }

  Serial.println();
  Serial.println("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
  //WiFi.mode(WIFI_AP);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);// --> This line will create a WiFi hotspot.
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);
  if(!WiFi.softAPConfig(local_ip, gateway, subnet)){
      Serial.println("AP Config Failed");
  }
  Serial.print("AP IP address: ");
  //Serial.println(myIP);
  Serial.println(WiFi.softAPIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
   // On HTTP request for root, provide index.html file
  server.on("/", HTTP_GET, onIndexRequest);
 
  // On HTTP request for style sheet, provide style.css
  server.on("/main.css", HTTP_GET, onCSSRequest);

  server.on("/jquery.js", HTTP_GET, onjqueyRequest);

  server.on("/myJsFunctions.js", HTTP_GET, onmyFunctionsRequest);

  // Handle requests for pages that do not exist
  server.onNotFound(onPageNotFound);
  // Start web server
  
  // Start WebSocket server and assign callback
  //webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  server.addHandler(&webSocket);
  server.begin();
  Serial.println("HTTP server started");

  //setpoint = analogSacale(flow,maxAValue);
  
}

void loop() {
  int delayTime=settime*1000;
  while(pumpState == 1)
  {
    
    if (mrunning)
    {
      currentMillis = millis();
      ledcWrite(ledChannel, setpoint);
      mrunning = false;
      //Serial.println("Pump runs on the max speed");
      Serial.println("Pump running time is: ");
      Serial.println(delayTime);
      previousMillis= millis();
     }
     ledcWrite(ledChannel, setpoint);
     if (previousMillis-currentMillis >=delayTime)
     {
     mrunning = true;
     Serial.println("Job completed");
     pumpState=0;     
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
      delay(10);
}
