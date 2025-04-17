// Implémentation des bibliothèques
#include "Adafruit_VL53L0X.h"
#include <SoftwareSerial.h> 
#include <Wire.h>

// Initialisation des addresses des capteurs TOF
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31

// Initialisation du pin shutdown pour différencier les deux capteurs TOF
#define SHT_LOX2 6

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

// Initialisation de la broche D2 pour envoyer les informations sur le 2ème arduino
#define pin2 2
#define pin3 3 

// Initialisation des objets pour les capteurs Time Of Flight
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();

// Initialisation des variables de mesure 
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;

// Initialisation des variables 
bool entree = false;
bool sortie = false;
volatile int etape = 0;
volatile int cpt = 0;

// Fonction setID pour initialiser les capteurs Time Of Flight
void setID() {
  // all reset
  digitalWrite(SHT_LOX2, LOW);
  delay(10);
  // all unreset

  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  // activating LOX1 and resetting LOX2

  digitalWrite(SHT_LOX2, LOW);

  // initing LOX1
  if (!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));
    while (1)
      ;
  }
  delay(10);

  // activating LOX2
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);

  //initing LOX2
  if (!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
    while (1)
      ;
  }
}

void setup() {
  // Initialise la communication série à 115200 bauds
  Serial.begin(115200);

  // Initialisation de la broche D2 en OUTPUT pour envoyer les informations
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);

  // Partie Initialisation des capteurs Time Of Flight
  Wire.begin();
  pinMode(SHT_LOX2, OUTPUT);
  Serial.println(F("Shutdown pins inited..."));
  digitalWrite(SHT_LOX2, LOW);
  Serial.println(F("Both in reset mode...(pins are low)"));
  Serial.println(F("Starting..."));
  setID();

  // Affichage du compteur dans le terminal
  Serial.println(cpt);
}

void loop() {
  // Prise des valeurs de mesure des capteurs
  lox1.rangingTest(&measure1, false);
  lox2.rangingTest(&measure2, false);
  int dist1 = measure1.RangeMilliMeter;
  int dist2 = measure2.RangeMilliMeter;

  // 1ère étape
  if (etape == 0) {
    // Si capteur1 reçoit une valeur dans les mesures correspondantes
    if (dist1 < 310 && dist2 > 310) {
      etape = 1;
      Serial.print("Quelqu'un entre");
      entree = true;
      sortie = false;
    }
    // Si capteur2 reçoit une valeur dans les mesures correspondantes
    if (dist1 > 310 && dist2 < 310) {
      etape = 1;
      Serial.print("Quelqu'un sort");
      entree = false;
      sortie = true;
    }
  }

  // 2ème étape 
  if (etape == 1) {
    // Si capteur2 reçoit une valeur suite au capteur1 => la personne entre 
    if (dist2 < 310 && entree) {
      etape = 2;
      Serial.print("...");
    }
    // Si capteur1 reçoit une valeur suite au capteur2 => la personne sort
    if (dist1 < 310 && sortie) {
      etape = 2;
      Serial.print("...");
    }
  }

  // 3ème étape
  if (etape == 2) {
    // Si les deux capteurs ne reçoivent aucune valeur 
    if (dist1 > 310 && dist2 > 310) {
      // On vérifie l'ordre des capteurs à l'aide des booléens 

      // Si c'est une entrée
      if (entree) {
        // On envoit les informations sur le pin D2 suivit de l'incrémentation du compteur
        digitalWrite(pin2, HIGH);
        delay(1);
        digitalWrite(pin2, LOW);
        cpt++;
        etape = 0;
        Serial.print("OK : ");
        Serial.println(cpt);
      }
      // Si c'est une sortie
      if (sortie) {
        if (cpt > 0) {
          // On envoit les informations sur le pin D3 suivit de la décrémentation du compteur
          digitalWrite(pin3, HIGH);
          delay(1);
          digitalWrite(pin3, LOW);
          cpt--;
        }
        etape = 0;
        Serial.print("OK : ");
        Serial.println(cpt);
      }
    }
  }
}