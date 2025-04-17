// Initialisation des bibliothèques
#include "grove_two_rgb_led_matrix.h"
#include <SoftwareSerial.h>
#include <Wire.h>

// Couleur cyan pour l'afficheur
#define DISPLAY_COLOR 0xfe

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

// Déclaration des objets qui permettent d'interagir avec les matrices
GroveTwoRGBLedMatrixClass screen1(GROVE_TWO_RGB_LED_MATRIX_DEF_I2C_ADDR, 1), screen2(GROVE_TWO_RGB_LED_MATRIX_DEF_I2C_ADDR + 1, 1);

// Pin 5 et 6 pour le module HC-06, correspondant au port D5 du shield
#define RxD 5
#define TxD 6
SoftwareSerial hc06(RxD, TxD);

// Initialisation des variables
int pin2 = 2;
int pin3 = 3;
int nbp = 0;

void setup() {
  // Initialise la communication série à 115200 bauds
  Serial.begin(115200);

  // Partie Matrice LED
  Wire.begin();
  uint16_t VID = 0, VID2 = 0;
  VID = screen1.getDeviceVID();  // VID = VendorID
  VID2 = screen2.getDeviceVID();
  if (VID != 0x2886) {  // 0x2886 -> ModèleID de la matrice LED, pour vérifier que l'appareil contacté est bien une matrice LED
    Serial.println("Can not detect led matrix 1!!!");
  }
  if (VID2 != 0x2886) {
    Serial.println("Can not detect led matrix 2!!!");
  }
  Serial.println("Matrix init success!!!");

  // Initialisation des afficheurs à 0
  screen1.displayNumber(nbp, 2000, true, DISPLAY_COLOR);
  screen2.displayNumber(nbp, 2000, true, DISPLAY_COLOR);

  // Configuration du bluetooth
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  hc06.begin(115200);
  delay(500);
  hc06.print("AT+VERSION");
  delay(1000);

  // Initialisation de la broche D2 en INPUT pour recevoir les informations
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);
}

void loop() {
  // Prise des informations de la broche D2
  int entree = digitalRead(pin2);
  int sortie = digitalRead(pin3);

  // Si c'est une entrée, on incrémente puis on affiches
  if (entree == HIGH) {
    if (nbp < 40) {
      nbp++;
      Serial.println(nbp);
      screen2.displayNumber(nbp / 10, 2000, true, DISPLAY_COLOR);  // On affiche l'unité du compteur (propriété de la division des int)
      screen1.displayNumber(nbp % 10, 2000, true, DISPLAY_COLOR);  // On affiche la nouvelle valeur
      hc06.print(nbp);

      Serial.print("{\"nbp\":");
      Serial.print(nbp);
      Serial.println("}");
    }
  }

  // Si c'est une sortie, on décrémente puis on affiche
  if (sortie == HIGH) {
    if (nbp > 0) {
      nbp--;
      Serial.println(nbp);
      screen2.displayNumber(nbp / 10, 2000, true, DISPLAY_COLOR);  // On affiche l'unité du compteur (propriété de la division des int)
      screen1.displayNumber(nbp % 10, 2000, true, DISPLAY_COLOR);  // On affiche la nouvelle valeur
      hc06.print(nbp);
      Serial.print("{\"nbp\":");
      Serial.print(nbp);
      Serial.println("}");
    }
  }
}
