#include <pcmConfig.h>
#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
TMRpcm audio;
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
int lastStateButton1 = LOW;

// BUTTON_2
#define BUTTON_PIN_2 2
unsigned long lastTimeButtonStateChangedButton2 = 0;
int lastStateButton2 = LOW;

// FILE
char *file = NULL;
int file_number = 0;
char filePrefixname[50] = "GUEST_";
char exten[10] = ".wav";
bool recording_now = false;

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
#define VOLUME 4
#define QUALITY 1

void startRecording() {
  char fileSlNum[20] = "";
  char file_name[50] = "";
  itoa(file_number, fileSlNum, 10);
  strcat(file_name, filePrefixname);
  strcat(file_name, fileSlNum);
  strcat(file_name, exten);
   if (!recording_now) {
     if (lastStateButton1 == LOW && ledState) {
       audio.volume(VOLUME);
       digitalWrite(SPEAKER, 1);
       audio.play((char *)greetings);
       delay(7500);
       audio.play((char *)beep);
       delay(1000);
       audio.pause();
       audio.volume(MUTE);
       recording_now = true;
       file = file_name; 
       digitalWrite(RECORDING_LED_PIN, HIGH);
       audio.startRecording(file, SAMPLE_RATE, MICROPHONE_PIN);
     } 
   } 
}

void stopRecording () {
  if (recording_now) {
    digitalWrite(RECORDING_LED_PIN, LOW);
    recording_now = false;
    audio.stopRecording(file);
    file_number++;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Loading...");

  // SET PINS
  pinMode(MICROPHONE_PIN, INPUT);
  pinMode(RECORDING_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);

  // SET SD_CARD
  if (!SD.begin(SD_ChipSelectPin)) Serial.println(F("An Error has occurred while mounting SD"));
  while (!SD.begin(SD_ChipSelectPin)) {
    Serial.print(".");
    delay(DELAY);
  }
  SD.begin(SD_ChipSelectPin);

  // SET AUDIO
  audio.speakerPin = SPEAKER;
  audio.setVolume(VOLUME);
  audio.quality(QUALITY);

  // SET RECORDER PIN
  audio.CSPin = SD_ChipSelectPin;
  Serial.println("Ready...");
}

void loop() {
  // BUTTON 1 - Handset button
  if (millis() - lastTimeButtonStateChangedButton1 > debounceDuration) {
    byte buttonStateBtn1 = digitalRead(BUTTON_PIN_1);
    if (buttonStateBtn1 != lastStateButton1) {
      if (buttonStateBtn1 == LOW) ledState = HIGH;
      else {
        ledState = LOW;
        lastStateButton2 = LOW;
        digitalWrite(BUTTON_PIN_2, lastStateButton2);
        digitalWrite(RECORDING_LED_PIN, ledState);
        stopRecording();
      }
    }
    lastStateButton1 = buttonStateBtn1;
  }
  // BUTTON_2 - Phone diar
  if (millis() - lastTimeButtonStateChangedButton2 > debounceDuration) {
    byte buttonStateBtn2 = digitalRead(BUTTON_PIN_2);
    if (buttonStateBtn2 != lastStateButton2) {     
      lastTimeButtonStateChangedButton2 = millis();
      lastStateButton2 = buttonStateBtn2;
      startRecording();
    }
  }
}
