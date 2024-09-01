
// Calibrating the load cell
#include "HX711.h"
#include <esp_now.h>
#include <WiFi.h>

/////////////////////////////////////////////////////Libraries for NTP/////////////////////////////////////////////////
#include "time.h"

int pongCntr = 0;
const char* ssid     = "ICST";
const char* password = "arduino123";

const char* ntpServer = "ntp.technion.ac.il";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

//////////////////////////////////////////////////////RECEIVER MAC Address/////////////////////////////////////////
uint8_t broadcastAddress[] = {0xb0, 0xa7, 0x32, 0xde, 0x95, 0xd8};

#define PONG_MSG "PONG"

#define PINGPONG_NUM 100

///////////////////////////////////////////////////HX711 circuit wiring////////////////////////////////////////////
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;

HX711 scale;

/////////////////////////////////////////LED////////////////////////////////////////////////////////////////////
int ledPin_green = 19; 

//////////////////////////////////////////////////////struct//////////////////////////////////////////////////////

typedef struct struct_message {
  float reading;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

//////////////////////////////////////////////////////button/////////////////////////////////////////////////////////
const int buttonPin = 15;

bool begin = false;
//////////////////////////////////////////////////////init WiFi///////////////////////////////////////////////////////
void initWiFi()
{
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //printLocalTime();

  /////////disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
}

///////////////////////////////////////////////////printing time/////////////////////////////////////////////////////////
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}


////////////////////////////////////////////////callback when data is sent////////////////////////////////////////
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  // if(status == ESP_NOW_SEND_SUCCESS){
  //   Serial.println("Delivery Success");
  //   analogWrite(ledPin_green, 255);
  // }
  // else{
  //   Serial.println("Delivery Fail");
  //   analogWrite(ledPin_green, 0);
  // }
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len){
  String message = String((char*)incomingData);
  //Serial.println("Received PINGgggggggggggg");
  if (message == "PING") {
    esp_now_send(broadcastAddress, (const uint8_t *)PONG_MSG, sizeof(PONG_MSG));
    pongCntr += 1; 
    //Serial.println(pongCntr);
  }
  if (message == "RESTART") {
    Serial.print("RESTARRRRRRRRRRRRRRRRRRRT");
    toSetUp();
  }
  if (message == "BEGIN") {
    begin = true;
  }
}


/////////////////////////////////////////////////SETUP//////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  delay(1000);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);


  // Send 3 pulses to PD_SCK to switch to 80 Hz
  for (int i = 0; i < 3; i++) {
    digitalWrite(LOADCELL_SCK_PIN, HIGH);
    delayMicroseconds(1);  // Keep the pulse short
    digitalWrite(LOADCELL_SCK_PIN, LOW);
    delayMicroseconds(1);
  }

  scale.set_scale(417);
  scale.tare();  // Reset the scale to zero

  //initWiFi();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  while(!begin)
  {
    Serial.print(".");
  }
  Serial.println(" ");
  Serial.println("lets gooo...");

  while(pongCntr < PINGPONG_NUM){
    Serial.print("");
  }
  delay(15);
}

/////////////////////////////////////////////////LOOP//////////////////////////////////////////////////////////////
void loop() {

  // if(digitalRead(buttonPin) == LOW){
  //   delay(100);
  //   toSetUp();
  // }

  if (scale.is_ready()) {

    float reading = scale.get_units(1);  // Average of 3 readings
    analogWrite(ledPin_green, 255);
    myData.reading = reading;
    Serial.print("Result: ");
    Serial.println(reading);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    if (result == ESP_OK) {
    Serial.println("Sent with success");
    analogWrite(ledPin_green, 255);
    }
    else {
      Serial.println("Error sending the data");
      analogWrite(ledPin_green, 0);
    }
  } 
  else {
    Serial.println("HX711 not found.");
    analogWrite(ledPin_green, 0);
  }
  delay(100);
}

void toSetUp(){
  // pongCntr = 0;
  // begin = false;
  // setup();
  analogWrite(ledPin_green, 0);
  ESP.restart();
}
