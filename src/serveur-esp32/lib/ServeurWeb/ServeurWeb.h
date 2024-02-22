/**
 * @file ServeurWeb.h
 * @brief Déclaration de la classe ServeurWeb
 * @author Thierry VAIRA
 * @version 0.1
 */

#ifndef SERVEURWEB_H
#define SERVEURWEB_H

#include "../../include/serveur-esp32.h"
#include "Led.h"
#include <WebServer.h>
#include <Preferences.h>
#include <vector>
#include <ArduinoJson.h>

#define DEBUG_SERVEUR_WEB

// Brochages
#define GPIO_LED_ROUGE  5  //!< La Led rouge
#define GPIO_LED_VERTE  16 //!< La Led verte
#define GPIO_LED_ORANGE 17 //!< La Led orange

/**
 * @class ServeurWeb
 * @brief Déclaration de la classe ServeurWeb
 */
class ServeurWeb : public WebServer
{
  private:
    Preferences preferences; //!< pour le stockage interne
    StaticJsonDocument<TAILLE_JSON>
                      documentJSON; //!< pour traiter les données JSON
    std::vector<Led*> leds;
    std::vector<bool> brochesDisponibles;

    void   setNom(String nomServeurWeb = NOM_SERVEUR_WEB);
    void   installerGestionnairesRequetes();
    void   afficherAccueil();
    void   traiterRequeteGetLeds();           // GET
    void   traiterRequeteGetLed();            // GET
    void   traiterRequeteUpdateLedWithForm(); // POST
    void   traiterRequeteUpdateLed();         // PUT
    void   traiterRequeteDeleteLed();         // DELETE
    void   traiterRequeteAddLed();            // POST
    void   traiterRequeteNonTrouvee();
    int    extraireIdLed();
    bool   extraireEtat();
    int    extraireNumeroBroche();
    String extraireCouleur();
    bool   estCouleurValide(String couleur) const;
    bool   estCouleurValide(CouleurLed couleur) const;
    bool   estBrocheValide(int broche) const;
    bool   estBrocheDisponible(int broche) const;
    bool   estRequeteGetLedPossible();
    bool   estRequeteUpdateLedWithFormPossible();
    bool   estRequeteUpdateLedPossible();
    bool   estRequeteDeleteLedPossible();
    bool   estAjoutPossible();

  public:
    ServeurWeb(uint16_t port = PORT_SERVEUR_WEB);
    void initialiserPreferences();
    void initialiserLeds();
    void demarrer();
    void traiterRequetes();
};

#endif // SERVEURWEB_H
