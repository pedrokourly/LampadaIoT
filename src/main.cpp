#include <Arduino.h>
#include <Ethernet.h>
#include <PubSubClient.h>

const int pinoEnergiaLDR = 3;
const int pinoLeituraLDR = A2;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

// EthernetClient ethClient;
// PubSubClient client(ethClient);

void setup() {
  Serial.begin(9600);
  pinMode(pinoEnergiaLDR, OUTPUT);
  digitalWrite(pinoEnergiaLDR, HIGH);
  Serial.println("Lendo valor do LDR...");
  // Ethernet.begin(mac);
  // client.setServer(mqttServer, mqttPort);
}

// void reconect(){
//   while (!client.connected()) {
//     if (client.connect("ArduinoClient")) {
//     } else {
//       delay(2000);
//     }
//   }
// }

void loop() {
  
  int valorLDR = analogRead(pinoLeituraLDR);
  Serial.print("Valor do LDR: ");
  Serial.println(valorLDR);
  delay(2000);
}