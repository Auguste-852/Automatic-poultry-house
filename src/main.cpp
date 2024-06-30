#include <Arduino.h>

#include <SPI.h>
#include <RF24.h>

#define extverrin 22
#define inverrin 21
#define motoreat 25
#define photoresistor 34
#define btndoor 35
#define btneat 39

// Définir les broches pour le module NRF24L01
#define pinCE   4
#define pinCSN  5
RF24 radio(pinCE, pinCSN);

// Définir les adresses des tunnels
#define tunnel1 "PIPE1"
#define tunnel2 "PIPE2"
const byte adresses[][6] = {tunnel1, tunnel2};

bool doorOpen = false;

// Structure pour stocker les paramètres de configuration
struct Settings {
  uint8_t doorInterval;
  uint8_t feedInterval;
  uint8_t foodQuantity;
  uint16_t photoresistorThreshold;
};

Settings settings;




void openDoor();
void closeDoor();
void applySettings(const Settings &settings);
void distributeFood();


void setup() {
  Serial.begin(115200);

  pinMode(extverrin, OUTPUT);
  pinMode(inverrin, OUTPUT);
  pinMode(motoreat, OUTPUT);
  pinMode(photoresistor, INPUT);
  
  settings.photoresistorThreshold = 500;µ

  // Initialiser le module RF24
  radio.begin();
  radio.openReadingPipe(1, adresses[0]);  // Ouverture du "tunnel1" en LECTURE
  radio.openWritingPipe(adresses[1]);     // Ouverture du "tunnel2" en ÉCRITURE
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  Serial.print("Radio connected: ");
  Serial.println(radio.isChipConnected() ? "YES" : "NO !!!");
}

void loop() {
  if (radio.available()) {
    char command;
    radio.read(&command, sizeof(command));

    // Vérifier les commandes de 'O' (ouvrir) ou 'C' (fermer)
    if (command == 'O') {
      Serial.print("O");
      openDoor();
    } else if (command == 'C') {
      closeDoor();
    } else {
      // Sinon, lire les paramètres de configuration
      Settings settings;
      radio.read(&settings, sizeof(Settings));
      applySettings(settings);
    }
  }

  int photoresistorValue = analogRead(photoresistor);

  Serial.print("Valeur de la photoresistance : ");
  Serial.println(photoresistorValue);

  // Vérifier l'état actuel de la porte avant de décider d'ouvrir ou fermer
  if (photoresistorValue < settings.photoresistorThreshold && !doorOpen) {
    openDoor();
    doorOpen = true;  // Mettre à jour l'état de la porte
  } else if (photoresistorValue >= settings.photoresistorThreshold && doorOpen) {
    closeDoor();
    doorOpen = false;  // Mettre à jour l'état de la porte
  }
  delay(1000);
}


void applySettings(const Settings &settings) {
  Serial.print("Mise à jour des paramètres : ");
  Serial.print("doorInterval = ");
  Serial.print(settings.doorInterval);
  Serial.print(", feedInterval = ");
  Serial.print(settings.feedInterval);
  Serial.print(", foodQuantity = ");
  Serial.print(settings.foodQuantity);
  Serial.print(", photoresistorThreshold = ");
  Serial.println(settings.photoresistorThreshold);
}

void openDoor() {
  // Code pour ouvrir la porte
  Serial.println("Ouverture de la porte...");
  digitalWrite(inverrin, HIGH);
  delay(3000); //replace for limitswitch
  digitalWrite(inverrin, LOW);
}

void closeDoor() {
  // Code pour fermer la porte
  Serial.println("Fermeture de la porte...");
  digitalWrite(extverrin, HIGH);
  delay(3000); //replace for limitswitch
  digitalWrite(extverrin, LOW);
}