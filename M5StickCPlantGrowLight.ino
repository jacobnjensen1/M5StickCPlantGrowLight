#include <Ticker.h>
#include <WiFi.h>

#include "TimeSpec.h"
#include "timer_config.h"
#include <ezTime.h>

#include <M5StickC.h> 

RTC_TimeTypeDef RTC_TimeStruct;
WiFiClient wifiClient;
//String uniqueId;
Timezone LocalTime;
#define LOCAL_TIMEZONE "America/Denver"
boolean light_state = false;

const int STATUS_LED = 10;  // 10 for esp32 M5StickC, 5 for esp32 Thing
const int POWER_PIN = 26;  // 26 for esp32 M5StickC, 5 for esp32 Thing
const int BUTTON_PIN = 37; 

void syncRTC(){
  char *wifi_network = "XXXXXXXXX";  //Change to the name of WiFi network
  char *wifi_password = "XXXXXXXX";  //Change to password of WiFi network
  
  //requires WiFi
  Serial.println("Connecting to WiFi ...");
  WiFi.begin(wifi_network, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for WiFi..........");
    delay(500);
  }
  Serial.println("Connected to WiFi");
  waitForSync();
  LocalTime.setLocation(LOCAL_TIMEZONE);
  
  RTC_TimeTypeDef TimeStruct;
  TimeStruct.Hours = LocalTime.hour();
  TimeStruct.Minutes = LocalTime.minute();
  TimeStruct.Seconds = LocalTime.second();
  M5.Rtc.SetTime(&TimeStruct);
}

void initLCD() {
    // put your setup code here, to run once:
  
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(32, 0, 2);
  M5.Lcd.println("Grow Lights Waiting...");
}

void printStatus() {

  Serial.println("Printing Status");
  
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(45, 0, 2);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes);


  M5.Lcd.setCursor(55, 30);
  M5.Lcd.setTextSize(2);
  if (light_state)
    M5.Lcd.println("ON");
  else
    M5.Lcd.println("OFF");
}

void print_times() {
  Serial.println("");
  Serial.println("ON TIMES:");
  for (byte i = 0; i < (sizeof(on_times) / sizeof(on_times[0])); i++){
    TimeSpec time = on_times[i];
    char buff[100];
    sprintf(buff, "%02d:%02d", time.hour, time.minute);
    Serial.println(buff);
  }
  Serial.println("");
  Serial.println("OFF TIMES:");
  for (byte i = 0; i < (sizeof(on_times) / sizeof(on_times[0])); i++){
    TimeSpec time = off_times[i];
    char buff[100];
    sprintf(buff, "%02d:%02d", time.hour, time.minute);
    Serial.println(buff);
  }
}

void setup() {
  digitalWrite(POWER_PIN, 0);
  digitalWrite(STATUS_LED, 1);    // Inverted for the built-in light
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(POWER_PIN, OUTPUT);  
  pinMode(STATUS_LED, OUTPUT);

  M5.begin();
  initLCD();
  
  //Serial.setDebugOutput(true);
  Serial.begin(115200);
  Serial.println("..... STARTING .....");
  print_times();    
  
  syncRTC(); //Uncomment to resync the internal clock
  printStatus();
}

void loop() {
  M5.update();
  check_timer();
  //checkButtonPress();
  if (M5.BtnA.wasPressed()) {
    handleButtonPress();
    Serial.println("BUTTONS!!!!!");
  }
} 

void check_timer() {
  const int CHECK_FREQUENCY = 1000;
  static int lastPrintTime = 0;
  int currentTime = millis();
  static int previousMinute = 0;
  if(currentTime - lastPrintTime > CHECK_FREQUENCY || lastPrintTime == 0) {
    M5.Rtc.GetTime(&RTC_TimeStruct);
    int current_second_of_day = RTC_TimeStruct.Hours * SECONDS_PER_HOUR + RTC_TimeStruct.Minutes * SECONDS_PER_MINUTE + RTC_TimeStruct.Seconds;
    
    char buff[100];
    
    sprintf(buff, "Checking at %02d:%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
    Serial.println(buff);

    if (RTC_TimeStruct.Minutes != previousMinute){
      printStatus();
      previousMinute = RTC_TimeStruct.Minutes;
    }
    

    for (byte i = 0; i < (sizeof(on_times) / sizeof(on_times[0])); i++){
      if (current_second_of_day - on_times[i].secondval() < 2 && current_second_of_day - on_times[i].secondval() > -2 ) {
        setLightState(1);
      }
    }

    for (byte i = 0; i < (sizeof(off_times) / sizeof(off_times[0])); i++){
      if (current_second_of_day - off_times[i].secondval() < 2 && current_second_of_day - off_times[i].secondval() > -2 ) {
        setLightState(0);
      }
    }
    
    lastPrintTime = currentTime;
  }
}

void handleButtonPress() {
  setLightState(!light_state);
}

void setLightState(bool onOff) {
  if(onOff)
    Serial.println("Light state: On");
  else
    Serial.println("Light state: Off");
  
  digitalWrite(POWER_PIN, onOff);
  digitalWrite(STATUS_LED, !onOff);    // Inverted for the built-in light
  light_state = onOff;
  printStatus();
}
