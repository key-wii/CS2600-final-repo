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


int dataPin = 15;          // Pin connected to DS of 74HC595（Pin14）
int latchPin = 2;          // Pin connected to ST_CP of 74HC595（Pin12）
int clockPin = 4;          // Pin connected to SH_CP of 74HC595（Pin11）

// Define the encoding of characters 0-9 for the common-anode 7-Segment Display
byte num[] = {
  0xbf, 0x86, 0xdb
};


#include "WiFi.h"
#include "PubSubClient.h"

// WiFi
const char *ssid = "Fast Eagle"; // Enter your WiFi name
const char *password = "bwds&!Dnpa";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.168.1.133";
const char *topic = "esp32/test";
const char *mqtt_username = "emqx";
const char *mqtt_password = "123456789";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  // publish and subscribe
  client.publish(topic, "Hi EMQX I'm ESP32 ^^");
  client.subscribe(topic);


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
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (!isAlerting) {
      isAlerting = true;      
      ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 0, 0);
    }
    alert();
  }
  else {
    if (isAlerting) {
      isAlerting = false;
      ledc_set_duty_and_update(LEDC_HS_MODE, LEDC_CHANNEL_0, 0, 0);
    }
  }
  
  // display 0-F on digital tube
  /*for (int i = 0; i < 3; i++) {
    writeData(num[i]);// Send data to 74HC595
    delay(1000);      // delay 1 second
    writeData(0xff);  // Clear the display content
  }

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();*/
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


void reconnect()
{
  //Try to reestablish connection
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected!");
      //Subscribe
      client.publish("esp32/output", "Hello!"); // <---If we do a sub of this on our computer we will see it say "Hello!"
      client.subscribe("test");          // <---This subscribes to the topic "test"
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived from topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageBuffer;
  
  //Iterate through pointer storing into buffer
  for (int i=0; i < length; i++)        // <--- The data is received as a string of characters, NOT a String object
  {                       
    Serial.print((char)payload[i]);       
    messageBuffer += (char)payload[i];      // <--- So we must assemble it into a buffer to read it in its entirety
  }
  Serial.println();
  Serial.println("-----------------------");
  
  //Check for messages from Led_Signal
  if (String(topic) == "test")  // <--- If we get publish in the topic "test", do this
  {
    Serial.print("Hello through serial via USB");  // <--- This will not be sent to broker
    if (messageBuffer == "angle")            // <--- If the message sent by publisher was "angle"
    {
      Serial.println("angle.");           // <--- The stuff we will do 
      //Do some function here to toggle light
    //or change a value. Whatever works best
    }
  }
}
