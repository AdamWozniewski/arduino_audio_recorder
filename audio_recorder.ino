#include <pcmConfig.h>
#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
// #include <Wire.h>
// #include <RTClib.h>
// RTC_DS1307 rtc;
TMRpcm audio;
/*
Hardware Pinout Connection
  Arduino Nano    SD Pin
    v ------------ VCC
    GND ----------- GND
    D10 ----------- CS
    D11 ----------- MOSI
    D12 ----------- MOSO
    D13 ----------- SCK
  ________________________________________
  Arduino Nano     MAX9814
    3.3v ----------- VDD
    GND ------------ GND
    A0 ------------- Out
  ________________________________________
*/
// CREATE GLOBAL VARIABLES
// #######################
// SD_CARD PIN
#define SD_ChipSelectPin 10

// SETTINGS
#define DELAY 500
#define MICROPHONE_PIN A0
#define SAMPLE_RATE 16000

// BUTON_1
#define BUTTON_PIN_1 3
unsigned long lastTimeButtonStateChangedButton1 = 0;
int lastStateButton1 = HIGH;

// BUTTON_2
#define BUTTON_PIN_2 2
unsigned long lastTimeButtonStateChangedButton2 = 0;
int lastStateButton2 = LOW;

// FILE
char *file = NULL;
int file_number = 0;
char filePrefixname[50] = "T";
char exten[10] = ".wav";
bool recording_now = false;
long *randNumberForFile = NULL;
File timeConfig;

// DEBOUNCE
unsigned long debounceDuration = 50;

// LED
byte ledState = LOW;
#define RECORDING_LED_PIN 4

// SOUND
char beep[20] = "beep.wav";
char greetings[20] = "grts.wav";
#define SPEAKER 9
#define MUTE 0
#define VOLUME 3
#define QUALITY 1
String greetingsDelay = "";
// CLOCK
// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
void playAudio() {
  audio.play((char *)greetings);
  delay(greetingsDelay.toInt() + 500);
  audio.play((char *)beep);
  delay(1000);
}

void startRecording() {
  char fileSlNum[20] = "";
  char file_name[50] = "";
  char temp[10];  
  randNumberForFile = random(300);
  ltoa(randNumberForFile, temp, 10);
  itoa(file_number, fileSlNum, 10);
  strcat(file_name, filePrefixname);
  strcat(file_name, fileSlNum);
  strcat(file_name, "_");
  strcat(file_name, temp);
  strcat(file_name, exten); 
  if (!recording_now) {
    if (lastStateButton2 == HIGH && ledState == HIGH) {
      digitalWrite(SPEAKER, HIGH);
      audio.volume(VOLUME);
      playAudio();
      recording_now = true;
      file = file_name;
      digitalWrite(RECORDING_LED_PIN, HIGH);
      digitalWrite(SPEAKER, LOW);
      audio.volume(MUTE);
      audio.pause();
      audio.startRecording(file, SAMPLE_RATE, MICROPHONE_PIN);
    }
  }
}

void stopRecording() {
  if (recording_now) {
    recording_now = false;
    // char fileSlNum[20] = "";
    // char file_name[50] = "";
    // DateTime now = rtc.now();
    // itoa(file_number, fileSlNum, 10);
    // strcat(file_name, filePrefixname);
    // strcat(file_name, fileSlNum);
    // strcat(file_name, "_");
    // strcat(file_name, now.day());
    // strcat(file_name, "_");
    // strcat(file_name, now.month());
    // strcat(file_name, "_");
    // strcat(file_name, now.year());
    // strcat(file_name, "___");
    // strcat(file_name, now.hour());
    // strcat(file_name, ":");
    // strcat(file_name, now.minute());
    // strcat(file_name, "::");
    // strcat(file_name, now.second());
    // strcat(file_name, exten);
    Serial.println(file);
    audio.stopRecording(file);
    file_number++;
    digitalWrite(RECORDING_LED_PIN, LOW);
    // if (!file.rename(sd.vwd(), "name2.txt")) {
    // error("name2.txt");
    // }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Loading...");

  // SET PINS
  pinMode(MICROPHONE_PIN, INPUT);
  pinMode(RECORDING_LED_PIN, OUTPUT);
  pinMode(SPEAKER, OUTPUT);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);

  // SET SD_CARD
  if (!SD.begin(SD_ChipSelectPin)) Serial.println(F("An Error has occurred while mounting SD"));
  while (!SD.begin(SD_ChipSelectPin)) {
    Serial.print(".");
    delay(DELAY);
  }
  SD.begin(SD_ChipSelectPin);
  timeConfig = SD.open("TIME.txt");
  if (timeConfig) {
    while (timeConfig.available()) {
      char timer = timeConfig.read();
      greetingsDelay.concat(timer);
    }
    if (greetingsDelay.length() == 0) greetingsDelay = "7000";
    timeConfig.close();
  } else Serial.println("error opening TIME.txt");
  // CLOCK
  // if (!rtc.begin()) {
  //   Serial.println("Couldn't find RTC");
  //   while (1);
  // }
  // if (! rtc.isrunning()) {
  //   Serial.println("RTC is NOT running!");
  //   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //   // January 21, 2014 at 3am you would call:
  //   // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  // }

  // SET AUDIO
  audio.speakerPin = SPEAKER;
  audio.setVolume(VOLUME);
  audio.quality(QUALITY);

  // SET RECORDER PIN
  audio.CSPin = SD_ChipSelectPin;
  Serial.println("Ready...");
}

void loop() {
  if (millis() - lastTimeButtonStateChangedButton1 > debounceDuration) {
    byte buttonStateBtn1 = !digitalRead(BUTTON_PIN_1);
    if (buttonStateBtn1 != lastStateButton1) {
      if (buttonStateBtn1 == HIGH) {
        ledState = LOW;
        lastStateButton2 = LOW;
        digitalWrite(BUTTON_PIN_2, lastStateButton2);
        digitalWrite(RECORDING_LED_PIN, ledState);
        stopRecording();
      } else ledState = HIGH;
    }
    lastTimeButtonStateChangedButton1 = millis();
    lastStateButton1 = buttonStateBtn1;
  }
  
  // BUTTON_2 - Phone diar
  if (digitalRead(BUTTON_PIN_2) == HIGH) {
    if (millis() - lastTimeButtonStateChangedButton2 > debounceDuration) {
      byte buttonStateBtn2 = digitalRead(BUTTON_PIN_2);
      if (buttonStateBtn2 != lastStateButton2) {
        lastTimeButtonStateChangedButton2 = millis();
        lastStateButton2 = buttonStateBtn2;      
        delay(1000);
        startRecording();
      }
    }
  }
}