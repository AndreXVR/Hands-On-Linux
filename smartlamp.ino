// Defina os pinos de LED e LDR
// Defina uma variável com valor máximo do LDR (4000)
// Defina uma variável para guardar o valor atual do LED (10)

#include "DHT.h"

int ledPin = 23;
int ledValue = 10;
int DHTPin = 22;
int ldrPin = 36;
int ldrMax = 4000;

#define DHTTYPE DHT11
DHT dht(DHTPin, DHTTYPE);

void setup() {
    Serial.begin(9600);
    
    pinMode(ledPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    pinMode(DHTPin, INPUT);
    analogWrite(ledPin, ledValue);

    dht.begin();
    
    Serial.printf("SmartLamp Initialized.\n");
}

// Função loop será executada infinitamente pelo ESP32
void loop() {
  String cmd;
   while (Serial.available() > 0) {
        char serialChar = Serial.read();
        cmd += serialChar; 

        if (serialChar == '\n') {
            processCommand(cmd);
            cmd = "";
        }
    }
   delay(100);
}


void processCommand(String command) {
    // compare o comando com os comandos possíveis e execute a ação correspondente      
    command.trim();
    if (command.startsWith("SET_LED")) {
        int pos = command.indexOf(' ');
        if (pos != -1) {
          String intensityString = command.substring(pos + 1);
          int intensity = intensityString.toInt();
          ledUpdate(intensity);
      }
    } else if (command.startsWith("GET_LED")) {
        Serial.printf("RES GET_LED %d\n", ledValue);
    } else if (command.startsWith("GET_LDR")) {
        Serial.printf("RES GET_LDR %d\n", ldrGetValue());
    } else if (command.startsWith("GET_TEMP")) {
        tempGetValue();
    } else if (command.startsWith("GET_HUM")) {
        humGetValue();
    }
}

// Função para atualizar o valor do LED
void ledUpdate(int led_intensity) {
    if(led_intensity >= 0 && led_intensity <= 100) {
        ledValue = led_intensity;
        analogWrite(ledPin, ledValue);
        Serial.println("RES SET_LED 1");
    } else {
        Serial.println("RES SET_LED -1");
    }
}

// Função para ler o valor do LDR
int ldrGetValue() {
    // Leia o sensor LDR e retorne o valor normalizado.
    int value=analogRead(ldrPin);
    if(value >= ldrMax){
      return 255;
    }
    return value*255/ldrMax;
}

void tempGetValue() {
  Serial.printf("RES GET_TEMP %.2f\n", dht.readTemperature());
}

void humGetValue() {
  Serial.printf("RES GET_HUM %.2f\n", dht.readHumidity());
}
