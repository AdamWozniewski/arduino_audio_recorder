#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
TMRpcm audio;
//creates global variables
// SD_CARD PIN
#define SD_ChipSelectPin 10

// BUTON
#define BUTTON_PIN 2
#define RECORDING_LED_PIN 3
#define MIN_PIN A0
#define SAMPLE_RATE 16000
int reading = 0;

// FILE
int file_number = 0;
char filePrefixname[50] = "GUEST_";
char exten[10] = ".wav";
bool recording_now = false;

// DEBOUNCE
unsigned long debounceDuration = 60;
unsigned long lastTimeButtonStateChanged = 0;
byte ledState = LOW;
byte lastButtonState = LOW;

// SOUND
char beep[20] = "beep.wav";
char greetings[25] = "greetings.wav";
#define SPEAKER 9
#define VOLUME 5

void button_pushed() {
  char fileSlNum[20] = "";
  itoa(file_number, fileSlNum, 10);
  char file_name[50] = "";
  strcat(file_name, filePrefixname);
  strcat(file_name, fileSlNum);
  strcat(file_name, exten);
  audio.play((char *)greetings);
  if (!recording_now && lastButtonState == HIGH) {
    audio.play((char *)beep);
    delay(500);
    audio.play((char *)beep);
    recording_now = true;
    audio.startRecording(file_name, SAMPLE_RATE, MIN_PIN);
    Serial.println(file_name);
  } else {
    recording_now = false;
    audio.stopRecording(file_name);
    file_number++;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("loading...");
  
  // SET PINS
  pinMode(MIN_PIN, INPUT);
  pinMode(RECORDING_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // SET BUTTON
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_pushed, FALLING);

  // SET SD_CARD
  if (!SD.begin(SD_ChipSelectPin)) Serial.println("An Error has occurred while mounting SD");
  while (!SD.begin(SD_ChipSelectPin)) {
    Serial.print(".");
    delay(500);
  }
  SD.begin(SD_ChipSelectPin);

  //SET AUDIO
  audio.speakerPin = SPEAKER;
  audio.setVolume(VOLUME);
  audio.quality(1);

  // SET RECORDER PIN
  audio.CSPin = SD_ChipSelectPin;
}

void loop() {
  if (millis() - lastTimeButtonStateChanged > debounceDuration) {
    byte buttonState = digitalRead(BUTTON_PIN);
    if (buttonState != lastButtonState) {
      lastTimeButtonStateChanged = millis();
      lastButtonState = buttonState;
      if (buttonState == LOW) {
        ledState = (ledState == HIGH) ? LOW : HIGH;
        digitalWrite(RECORDING_LED_PIN, ledState);
      }
    }
  }
}
