/**
 * @file src/main.cpp
 * @brief Programme principal du serveur web HTTP Rest
 * @author Thierry VAIRA
 * @version 0.1
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "ServeurWeb.h"

WiFiManager wm;
WiFiClient  espClient;
ServeurWeb  serveurWeb; // par défaut sur le port 80

void setup()
{
    Serial.begin(115200);
    Serial.println(F("Serveur web HTTP Rest"));

    serveurWeb.initialiserPreferences();

    // Configuration du WiFi avec WiFiManager
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // reset settings - wipe credentials for testing
    // wm.resetSettings();
    wm.setTitle("Serveur web HTTP Rest");
    // wm.setDarkMode(true);
    bool resultat = false;
    resultat      = wm.autoConnect(); // auto generated AP name from chipid
    if(!resultat)
    {
        Serial.println(F("Erreur de connexion !"));
        // ESP.restart();
    }
    // fin de la configuration du WiFi avec WiFiManager

    serveurWeb.demarrer();
}

void loop()
{
    serveurWeb.traiterRequetes();
}
