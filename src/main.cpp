#include <Arduino.h>

// Use A2 para garantir que estamos falando da porta Analógica
const int pinoEnergiaLDR = 3; // Ligue o VCC do sensor na Digital 3
const int pinoLeituraLDR = A2; // Ligue o A0 do sensor na Analógica A2

void setup() {
  // Garanta que o monitor_speed no platformio.ini também é 9600
  Serial.begin(9600);
  
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  Serial.println("--- INICIANDO O ARDUINO ---"); // Mensagem de teste

  // Configura a alimentação "gambiarra" via pino digital
  pinMode(pinoEnergiaLDR, OUTPUT);
  digitalWrite(pinoEnergiaLDR, HIGH); // Liga 5V no sensor
  
  Serial.println("Sensor alimentado! Entrando no loop...");
}

void loop() {
  // Lê o valor analógico (0 a 1023)
  int valorLDR = analogRead(pinoLeituraLDR);
  
  Serial.print("Luz: ");
  Serial.print(valorLDR);
  
  // Interpretação básica para te ajudar
  if(valorLDR < 200) {
    Serial.println(" (Escuro / Noite)");
  } else if(valorLDR > 800) {
    Serial.println(" (Muito Claro / Sol)");
  } else {
    Serial.println(" (Meia luz)");
  }

  delay(1000);
}