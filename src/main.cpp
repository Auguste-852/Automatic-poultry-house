#include <Arduino.h>

#include <SPI.h>
#include <RF24.h>

#define extverrin 22
#define inverrin 21
#define motoreat 25
#define photoresistor 34
#define btndoor 35

//#define openLimitSwitchPin 6
//#define closeLimitSwitchPin 7

// Définir les broches pour le module NRF24L01
#define pinCE   4
#define pinCSN  5
RF24 radio(pinCE, pinCSN);

// Définir les adresses des tunnels
#define tunnel1 "PIPE1"
#define tunnel2 "PIPE2"
const byte adresses[][6] = {tunnel1, tunnel2};

unsigned long previousFeedTime = 0;

bool doorOpen = false;

// Structure pour stocker les paramètres de configuration
struct Settings {
  uint8_t doorInterval;//temps entre nourriture et fermeture porte
  uint8_t feedInterval; //temps entre deux nourriture
  uint8_t foodQuantity; //quantité nourriture distribué
  uint16_t photoresistorThreshold; //Valeur declanchement photoresistance
  uint8_t timeToEat;  // Temps alloué pour la distribution de nourriture (en secondes)
};

Settings settings;


void openDoor();
void closeDoor();
void applySettings(const Settings &settings);
void distributeFood();
void sendDoorStatus(const char* status);
void distributeFood(uint8_t quantity);



void setup() {
  Serial.begin(115200);

  pinMode(extverrin, OUTPUT);
  pinMode(inverrin, OUTPUT);
  pinMode(motoreat, OUTPUT);
  pinMode(photoresistor, INPUT);
  //pinMode(openLimitSwitchPin, INPUT);
  //pinMode(closeLimitSwitchPin, INPUT);
  
  settings.photoresistorThreshold = 500;

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

  unsigned long currentMillis = millis();

  // Vérifier si l'intervalle feedInterval est écoulé
  if (currentMillis - previousFeedTime >= settings.feedInterval * 3600000UL) {
    // Distribuer la nourriture avec la quantité foodQuantity
    distributeFood(settings.foodQuantity);
    
    // Mettre à jour le temps précédent de la distribution de nourriture
    previousFeedTime = currentMillis;
  }
  
  int photoresistorValue = analogRead(photoresistor);

  Serial.print("Valeur de la photoresistance : ");
  Serial.println(photoresistorValue);


    // Vérifier l'état actuel de la porte avant de décider d'ouvrir ou fermer
  /* code pour limit switch
  if (photoresistorValue < settings.photoresistorThreshold && !doorOpen) {
    openDoor();
    if (digitalRead(openLimitSwitchPin) == HIGH) {
      doorOpen = true;  // Mettre à jour l'état de la porte
    }
  } else if (photoresistorValue >= settings.photoresistorThreshold && doorOpen) {
    closeDoor();
    if (digitalRead(closeLimitSwitchPin) == HIGH) {
      doorOpen = false;  // Mettre à jour l'état de la porte
    }
  }
  delay(1000); */

  // Vérifier l'état actuel de la porte avant de décider d'ouvrir ou fermer


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

void distributeFood(uint8_t quantity) {
  // Simuler la distribution de la nourriture
  Serial.print("Distributing ");
  Serial.print(quantity);
  Serial.println(" units of food...");
}

void openDoor() {
  // Code pour ouvrir la porte
  Serial.println("Ouverture de la porte...");
  digitalWrite(inverrin, HIGH);
  delay(3000); // Remplacer par l'utilisation de l'interrupteur de fin de course
  digitalWrite(inverrin, LOW);

  /* Vérifier l'interrupteur de fin de course pour l'ouverture
  if (digitalRead(openLimitSwitchPin) == HIGH) {
    Serial.println("Porte ouverte avec succès");
    doorOpen = true;
    sendDoorStatus("OPEN");
  } else {
    Serial.println("Erreur : fin de course d'ouverture non activé");
    sendDoorStatus("ERROR_OPEN");
  } */
}

void closeDoor() {
  // Code pour fermer la porte
  Serial.println("Fermeture de la porte...");
  digitalWrite(extverrin, HIGH);
  delay(3000); // Remplacer par l'utilisation de l'interrupteur de fin de course
  digitalWrite(extverrin, LOW);
}
  /* Vérifier l'interrupteur de fin de course pour la fermeture
  if (digitalRead(closeLimitSwitchPin) == HIGH) {
    Serial.println("Porte fermée avec succès");
    doorOpen = false;
    sendDoorStatus("CLOSED");
  } else {
    Serial.println("Erreur : fin de course de fermeture non activé");
    sendDoorStatus("ERROR_CLOSE");
  }
*/

void sendDoorStatus(const char* status) {
  radio.stopListening();
  radio.write(&status, sizeof(status));
  radio.startListening();
  }
}