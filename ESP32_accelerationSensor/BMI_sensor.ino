////////////////////////////////////////////////// /Libraries for SD card//////////////////////////////////////////////
#include "FS.h"
#include "SD.h"
#include <SPI.h>
#include <ArduinoQueue.h>
#define REASSIGN_PINS

int sck = 18;
int miso = 23;
int mosi = 19;
int cs = 5;

////////////////////////////////////////////////////libraries for ESP-NOW//////////////////////////////////////////////

#include <esp_now.h>


uint8_t senderAddress[] = {0x78, 0xE3, 0x6D, 0x1B, 0x0C, 0x9C};

esp_now_peer_info_t peerInfo;

/////////////////////////////////////////////////////Libraries for NTP/////////////////////////////////////////////////
#include <WiFi.h>
#include "time.h"


const char* ssid     = "ICST";
const char* password = "arduino123";


const char* ntpServer = "ntp.technion.ac.il";
// const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

bool set_time = true;


//////////////////////////////////////////////////libraries for BMI160/////////////////////////////////////////////////
#include <DFRobot_BMI160.h>
#include <Adafruit_Sensor.h>

DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;


////////////////////////////////////////////Variables to hold sensor readings//////////////////////////////////////////
String dataMessage;
String forceData;
unsigned long bmiDataTime;
unsigned long forceDataTime;

// Structure to receive data
typedef struct struct_message {
    float force_reading;
} struct_message;

// Create a struct_message called myData
struct_message myData;

//////////////////////////////////////////////////////RTC/////////////////////////////////////////////////////////////

#include <Wire.h>
#include <I2C_RTC.h>

static DS1307 RTC;

unsigned long init_time = 0;
String curr_time = "";

/////////////////////////////////////////pingpong/////////////////////////////////////////////////////////////////////
#define PINGPONG_NUM 100

ArduinoQueue<unsigned long> pingPongQ(PINGPONG_NUM);

#define PING_MSG "PING"

int pingPong = 0;

unsigned long rtt;
float oneWayDelay = 0;
float AVGoneWayDelay = 0;
int pingCntr = 0;
String time_stamp = "";

//////////////////////////////////////////////////////button/////////////////////////////////////////////////////////
const int buttonPin = 13;

#define RESTART "RESTART"

#define BEGIN "BEGIN"
////////////////////////////////////////led////////////////////////////
int ledPin_red = 15;    
int ledPin_green = 25; 



/////////////////////////////////////////////////////init BMI160//////////////////////////////////////////////////////
void init_BMI(){ 
    //init the hardware bmin160  
  if (bmi160.softReset() != BMI160_OK){
    Serial.println("reset false");
    while(1);
  }
  
  //set and init the bmi160 i2c address
  if (bmi160.I2cInit(i2c_addr) != BMI160_OK){
    Serial.println("init false");
    while(1);
  }
}

/////////////////////////////////////////////////Initialize SD card////////////////////////////////////////////////////
void initSDCard(){
  #ifdef REASSIGN_PINS
  SPI.begin(sck, miso, mosi, cs);
  if (!SD.begin(cs)) {
  #else
  if (!SD.begin()) {
  #endif
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

/////////////////////////////////////////////////////Write to the SD card//////////////////////////////////////////////
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

///////////////////////////////////////////////////Append data to the SD card//////////////////////////////////////////
void appendFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    // Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    // Serial.println("Message appended");
  } else {
    // Serial.println("Append failed");
  }
  file.close();
}

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
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  printLocalTime();

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


////////////////////////////////callback function that will be executed when data is received//////////////////////////
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if(pingPong < PINGPONG_NUM)
  {
    String message = String((char*)incomingData);
    if (message == "PONG") {
        // unsigned long currentMillis = millis();
        if(pingPongQ.isEmpty())
        {
          Serial.println("queue is empty!!!!!!!!!!!!");
        }
        unsigned long currentMillis = millis();
        unsigned long startPingTime = pingPongQ.dequeue();
        Serial.print("deq ");
        Serial.println(startPingTime);
        Serial.println(currentMillis - startPingTime);
        rtt = currentMillis - startPingTime;
        oneWayDelay += float(rtt) / 2;
        Serial.println(oneWayDelay);
        pingPong = pingPong + 1;
        Serial.println("PONGGGGG");
    }
  }
  else{
    memcpy(&myData, incomingData, sizeof(myData));
    forceData = String(myData.force_reading) + " ";
    //Append the data to file
    String file_name2 = "/FORCE_data_" + time_stamp + ".txt";
    // appendFile(SD, file_name2.c_str(), forceData.c_str());
    /////////////////////get current time
    curr_time = get_curr_time();
    forceDataTime = millis() - init_time;
    forceDataTime = forceDataTime - AVGoneWayDelay;
    String forceDataTimeS = String(forceDataTime) + " ";
    forceData += forceDataTimeS;
    forceData += curr_time;
    forceData +=  "\r\n";
    appendFile(SD, file_name2.c_str(), forceData.c_str());
    //append
    }
}

/////////////////////////////////////////////////RTC/////////////////////////////////////////////////////////////////

String print_rtc_time()
{
  String res = "";
    switch (RTC.getWeek())
  {
    case 1:
      Serial.print("SUN");
      res+= "SUN";
      break;
    case 2:
      Serial.print("MON");
      res+= "MON";
      break;
    case 3:
      Serial.print("TUE");
      res+= "TUE";
      break;
    case 4:
      Serial.print("WED");
      res+= "WED";
      break;
    case 5:
      Serial.print("THU");
      res+= "THU";
      break;
    case 6:
      Serial.print("FRI");
      res+= "FRI";
      break;
    case 7:
      Serial.print("SAT");
      res+= "SAT";
      break;
  }
  Serial.print(" ");
  res+= "_";
  Serial.print(RTC.getDay());
  res+= RTC.getDay();
  Serial.print("-");
  res+= ".";
  Serial.print(RTC.getMonth());
  res+= RTC.getMonth();
  Serial.print("-");
  res+= ".";
  Serial.print(RTC.getYear());
  res+= RTC.getYear();

  Serial.print(" ");
  res+= "_";

  Serial.print(RTC.getHours());
  res+= RTC.getHours();
  Serial.print(":");
  res+= "_";
  Serial.print(RTC.getMinutes());
  res+= RTC.getMinutes();
  Serial.print(":");
  res+= "_";
  Serial.print(RTC.getSeconds());
  res+= RTC.getSeconds();
  Serial.print(" ");
  //res+= "_";

  if (RTC.getHourMode() == CLOCK_H12)
  {
    switch (RTC.getMeridiem()) {
      case HOUR_AM :
        Serial.print(" AM");
        break;
      case HOUR_PM :
        Serial.print(" PM");
        break;
    }
  }
  Serial.println("");
  return res;
}

String get_curr_time()
{
  String res = "";
  res+= RTC.getHours();
  res+= ":";
  res+= RTC.getMinutes();
  res+= ":";
  res+= RTC.getSeconds();
  return res;
}

void set_rtc()
{
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  RTC.begin();
  RTC.setHourMode(CLOCK_H24);
  if (RTC.getHourMode() == CLOCK_H12)
  {
     RTC.setMeridiem(HOUR_PM);
  }
  RTC.setWeek(timeinfo.tm_wday + 1);
  RTC.setDate(timeinfo.tm_mday,timeinfo.tm_mon + 1,timeinfo.tm_year - 100); // 3la demet omima
  RTC.setTime(timeinfo.tm_hour ,timeinfo.tm_min ,timeinfo.tm_sec); // 3la demet omimaaaaaaaaa
}

//////////////////////////////////////////////////////SETUP///////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Press the button to start setup...");
  pinMode(buttonPin, INPUT_PULLUP);

  while(digitalRead(buttonPin) == HIGH)
  {
    Serial.print(".");
  }
  Serial.println(" ");
  Serial.println("Button pressed!...");

  analogWrite(ledPin_red, 255);
  init_BMI();
  initSDCard();

  if(set_time){
    initWiFi();
    set_rtc();
    set_time = false;
  }

  time_stamp = print_rtc_time();
  String file_time_stamp = time_stamp + "\r\n";
  Serial.println(time_stamp);
  // If the BMI_data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  String file_name = "/BMI_data_" + time_stamp + ".txt";
  Serial.println(file_name);
  Serial.println(file_name.c_str());
  File BMI_file = SD.open(file_name);
  if(!BMI_file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, file_name.c_str(), file_time_stamp.c_str());
    //writeFile(SD, "/BMI_data.txt", time_stamp.c_str());
  }
  else {
    Serial.println("File already exists");
  }
  BMI_file.close();

  // If the Force_data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  String file_name2 = "/FORCE_data_" + time_stamp + ".txt";

  File FORCE_file = SD.open(file_name2);
  if(!FORCE_file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, file_name2.c_str(), file_time_stamp.c_str());
  }
  else {
    Serial.println("File already exists");
  }
  FORCE_file.close();


  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);


  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  // Register peer
  memcpy(peerInfo.peer_addr, senderAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer    
    
  if (!esp_now_is_peer_exist(senderAddress))
  {
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
  }

  esp_now_send(senderAddress, (const uint8_t *)BEGIN, sizeof(BEGIN));
  for(int i = 0; i <PINGPONG_NUM; i++){
    analogWrite(ledPin_red, 255);
    unsigned long startPingTime = millis();
    Serial.println(startPingTime);
    if(!pingPongQ.enqueue(startPingTime))
    {
      Serial.println("enqueue failed!!!!!");
    }
    esp_now_send(senderAddress, (const uint8_t *)PING_MSG, sizeof(PING_MSG));
    Serial.println("Sent PING, waiting for PONG...");
    delay(100);
    analogWrite(ledPin_red, 0);
  }

  while(pingPong < PINGPONG_NUM){
    if(digitalRead(buttonPin) == LOW)
    {
      delay(100);
      toSetUp();
    }
  }
  init_time = millis(); //recording start time 
  AVGoneWayDelay = oneWayDelay / PINGPONG_NUM;


  Serial.print("AVGoneWayDelay: ");
  Serial.println(AVGoneWayDelay);
  analogWrite(ledPin_red, 0);
}

/////////////////////////////////////////////////loop////////////////////////////////////////////////////////////////
void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(buttonPin) == LOW){
    delay(100);
    toSetUp();
  }

  int i = 0;
  int rslt;
  int16_t accelGyro[6]={0};

  
  rslt = bmi160.getAccelGyroData(accelGyro);
  String file_name = "/BMI_data_" + time_stamp + ".txt";
  if(rslt == 0){
    for(i=0;i<6;i++){
      if(i<3){
        Serial.print(accelGyro[i]*3.14/180.0);Serial.print("\t"); //radians
        dataMessage += String(accelGyro[i]*3.14/180.0);
        dataMessage += " ";
        analogWrite(ledPin_green, 255); //led

      }
      else{
        Serial.print(accelGyro[i]/16384.0);Serial.print("\t");
        dataMessage += String(accelGyro[i]/16384.0);
        dataMessage += " ";
        analogWrite(ledPin_green, 255); // led
      }
    }
    curr_time = get_curr_time();
    Serial.print(curr_time);
    bmiDataTime = millis() - init_time; /////////////////////get current time 
    String bmiDataTimeS = String(bmiDataTime);
    dataMessage += bmiDataTimeS;
    dataMessage += " ";
    dataMessage += curr_time;
    dataMessage += "\r\n";
    appendFile(SD, file_name.c_str(), dataMessage.c_str());
    analogWrite(ledPin_green, 255);//led 
    Serial.println();

  }
  else{
    dataMessage = ("err");
    analogWrite(ledPin_green, 0);//led 

  }
  dataMessage = "";
}

void toSetUp(){
  esp_now_send(senderAddress, (const uint8_t *)RESTART, sizeof(RESTART));
  delay(100);
  analogWrite(ledPin_green, 0);//led 
  rtt = 0;
  oneWayDelay = 0;
  AVGoneWayDelay = 0;
  pingPong = 0;
  init_time = 0;
  while(!pingPongQ.isEmpty())
  {
    pingPongQ.dequeue();
  }
  setup();
}
