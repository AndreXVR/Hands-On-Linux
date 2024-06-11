// Defina os pinos de LED e LDR
// Defina uma variável com valor máximo do LDR (4000)
// Defina uma variável para guardar o valor atual do LED (10)

#include "DHT.h"
int ledPin = 22;
int ledValue = 10;

int ldrPin = 36;
int ldrMax = 4000;

#define DHT_PIN 21
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

void setup() {
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    
    dht.begin();
    
    Serial.printf("SmartLamp Initialized.\n");
}

// Função loop será executada infinitamente pelo ESP32
void loop() {
  //delay(2000);
    //Obtenha os comandos enviados pela serial
    if (Serial.available() > 0) {
      String cmd = Serial.readString();
    
      //e processe-os com a função processCommand
      processCommand(cmd);
    } 
    
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
    } else if (command.equals("GET_LED")) {
        Serial.println("RES SET_LED 1");
        ledGetValue();
    }
    else if (command.equals("GET_LDR")) {
        ldrGetValue();

    }else if (command.equals("GET_TEMP")) {
        tempGetValue();
    }
    else if (command.equals("GET_HUM")) {
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

// Função para ler o valor do LED
int ledGetValue() {
    Serial.printf("RES GET_LED ");
    Serial.println(ledValue);
}

float humGetValue(){
  float h = dht.readHumidity();
  Serial.print(F("Humidity: "));
  Serial.print(h);
}

float tempGetValue(){
  float t = dht.readTemperature();
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  
}
