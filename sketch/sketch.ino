#include "driver/ledc.h"

#define BUZZER_PIN     13
#define PIN_BUTTON     4

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_LS_TIMER          LEDC_TIMER_1

#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE

#define LEDC_HS_CH0_GPIO       (BUZZER_PIN)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0

bool isAlerting = false;
int hour = 3600000;
int hours8 = hour * 8;

int dataPin = 15;          // Pin connected to DS of 74HC595（Pin14）
int latchPin = 2;          // Pin connected to ST_CP of 74HC595（Pin12）
int clockPin = 4;          // Pin connected to SH_CP of 74HC595（Pin11）

// Define the encoding of characters 0-1 for the common-anode 7-Segment Display
byte num[] = {
  0xc0, 0xf9
};

void setup() {
    pinMode(PIN_BUTTON, INPUT);
    ledc_channel_config_t ledc_channel = {
        LEDC_HS_CH0_GPIO,
        LEDC_HIGH_SPEED_MODE,
        LEDC_CHANNEL_0,
        LEDC_INTR_DISABLE,
        LEDC_HS_TIMER,
        0,
        0
    };
    ledc_timer_config_t ledc_timer = {
        LEDC_HIGH_SPEED_MODE,
        LEDC_TIMER_10_BIT,
        LEDC_HS_TIMER,
        5000
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel);  
    ledc_fade_func_install(0);
    
  // set pins to output
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}
void loop() {
     delay(60000);
     //delay(hours8);
     writeData(num[0]);// Send display data to 74HC595
     isAlerting = true;
     ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 0, 0);
     alert();
     delay(500);
     writeData(0xff);  // Clear the display content
     isAlerting = false;
     ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 0, 0);
     delay(500);
     writeData(num[0]);// Send display data to 74HC595
     isAlerting = true;
     ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 0, 0);
     alert();
     delay(500);
     writeData(0xff);  // Clear the display content
     isAlerting = false;
     ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 0, 0);
     delay(500);
     writeData(num[0]);// Send display data to 74HC595
     isAlerting = true;
     ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 0, 0);
     alert();
     delay(500);
     writeData(0xff);  // Clear the display content
     isAlerting = false;
     ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 0, 0);
}

void alert() {
    float sinVal;
    int toneVal;
    for (int x = 0; x < 360; x += 1) {
        sinVal = sin(x * (PI / 180));
        toneVal = 2000 + sinVal * 500;
        ledc_set_freq(LEDC_HS_MODE, LEDC_TIMER_0, toneVal);
        ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 512, 0);
        delay(1);
    }
}


void writeData(int value) {
  // Make latchPin output low level
  digitalWrite(latchPin, LOW);
  // Send serial data to 74HC595
  shiftOut(dataPin, clockPin, LSBFIRST, value);
  // Make latchPin output high level, then 74HC595 will update the data to parallel output
  digitalWrite(latchPin, HIGH);
}
