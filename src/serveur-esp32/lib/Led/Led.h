/**
 * @file Led.h
 * @brief Déclaration de la classe Led
 * @author Thierry VAIRA
 * @version 0.1
 */

#ifndef Led_H
#define Led_H

#include "../../include/serveur-esp32.h"
#include <ArduinoJson.h>

#define DEBUG_LED

enum CouleurLed
{
    ROUGE,
    VERTE,
    ORANGE,
    NB_COULEURS
};

/**
 * @class Led
 * @brief Déclaration de la classe Led
 */
class Led
{
  private:
    StaticJsonDocument<TAILLE_JSON>
           documentJSON; //!< pour traiter les données JSON
    int    idLed;        //!< l'idLed
    bool   etat;         //!< l'état de la led
    String couleur;      //!< la couleur de la led
    int    broche;       //!< le numero de broche de la led

    int    extraireIdLed(StaticJsonDocument<TAILLE_JSON>& documentJSON);
    bool   extraireEtat(StaticJsonDocument<TAILLE_JSON>& documentJSON);
    int    extraireNumeroBroche(StaticJsonDocument<TAILLE_JSON>& documentJSON);
    String extraireCouleur(StaticJsonDocument<TAILLE_JSON>& documentJSON);

  public:
    Led(int idLed, bool etat, String couleur, int broche);
    Led(String objetJSON);

    int        getIdLed() const;
    bool       getEtat() const;
    int        getNumeroBroche() const;
    String     getCouleur() const;
    JsonObject getObjetJSON() const;
    void       setEtat(bool etat);
    void       setNumeroBroche(int broche);
    void       setCouleur(String couleur);
    void       setCouleur(CouleurLed couleur);
    void       commanderLed();
    void       setObjetJSON(JsonObject& objetJSON) const;

    static String getNomCouleur(CouleurLed couleur);
};

#endif // Led_H
