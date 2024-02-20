/**
 * @file ServeurWeb.h
 * @brief Déclaration de la classe ServeurWeb
 * @author Thierry VAIRA
 * @version 0.1
 */

#ifndef SERVEURWEB_H
#define SERVEURWEB_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#define DEBUG_SERVEUR_WEB

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

#define NB_LEDS 2

enum CouleurLed
{
    ROUGE,
    VERTE,
    NB_COULEURS
};

/**
 * @class ServeurWeb
 * @brief Déclaration de la classe ServeurWeb
 */
class ServeurWeb : public WebServer
{
  private:
    Preferences preferences; //!< pour le stockage interne
    StaticJsonDocument<TAILLE_JSON>
           documentJSON;                   //!< pour traiter les données JSON
    int    idLeds[NB_LEDS]     = { 1, 2 }; //!< l'idLed des leds
    bool   etatLedS[NB_LEDS]   = { false, false }; //!< l'état des leds
    int    brocheLeds[NB_LEDS] = { 4, 5 }; //!< le numero de broche des leds
    String couleurLeds[NB_COULEURS] = { "rouge",
                                        "verte" }; //!< la couleur des leds

    void   setNom(String nomServeurWeb = NOM_SERVEUR_WEB);
    void   installerGestionnairesRequetes();
    void   afficherAccueil();
    void   traiterRequeteGetLeds();
    void   traiterRequeteGETLed();
    void   traiterRequetePOSTLed();
    void   traiterRequetePUTLed();
    void   traiterRequeteDELETELed();
    void   traiterRequeteNonTrouvee();
    int    extraireIdLed();
    bool   extraireEtat();
    int    extraireNumeroBroche();
    String extraireCouleur();

  public:
    ServeurWeb(uint16_t port = PORT_SERVEUR_WEB);
    void initialiserPreferences();
    void demarrer();
    void traiterRequetes();
};

#endif // SERVEURWEB_H
