#ifndef SERVEUR_ESP32_H

#include <Arduino.h>

/**
 * @def NOM_SERVEUR_WEB
 * @brief Le nom du serveur web (http://iot-esp32.local/)
 */
#define NOM_SERVEUR_WEB "iot-esp32"

/**
 * @def PORT_SERVEUR_WEB
 * @brief Le numéro de port du serveur web
 */
#define PORT_SERVEUR_WEB 80

/**
 * @def TAILLE_JSON
 * @brief Définit la taille en octets max pour un document JSON
 */
#define TAILLE_JSON 256

/**
 * @def NB_LEDS_MAX
 * @brief Définit le nombre max de Leds
 */
#define NB_LEDS_MAX 8

#endif // SERVEUR_ESP32_H
