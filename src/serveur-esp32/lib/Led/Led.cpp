/**
 * @file Led.cpp
 * @brief Définition de la classe Led
 * @author Thierry VAIRA
 * @version 0.1
 */

#include "Led.h"

/**
 * @brief Constructeur de la classe Led
 * @fn Led::Led
 * @param idLed L'identifiant de la Led
 * @param etat L'état de la Led
 * @param couleur La couleur de la Led
 * @param broche Le numéro de broche de la Led
 * @details Initialise un nouvel objet Led
 */
Led::Led(int idLed, bool etat, String couleur, int broche) :
    idLed(idLed), etat(etat), couleur(couleur), broche(broche)
{
#ifdef DEBUG_LED
    Serial.print(F("Led::Led(idLed, etat, couleur, broche)"));
    Serial.print(F("idLed = "));
    Serial.println(idLed);
    Serial.print(F("etat = "));
    Serial.println(etat);
    Serial.print(F("couleur = "));
    Serial.println(couleur);
    Serial.print(F("broche = "));
    Serial.println(broche);
#endif
    commanderLed();
}

/**
 * @brief Constructeur de la classe Led
 * @fn Led::Led
 * @param objetJSON Un objet JSON sour forme de String contenant les données
 * idLed, etat, couleur et broche
 * @details Initialise un nouvel objet Led
 */
Led::Led(String objetJSON)
{
    StaticJsonDocument<TAILLE_JSON> documentJSON;
    DeserializationError erreur = deserializeJson(documentJSON, objetJSON);
    if(erreur)
    {
        return;
    }
    idLed   = extraireIdLed(documentJSON);
    etat    = extraireEtat(documentJSON);
    broche  = extraireNumeroBroche(documentJSON);
    couleur = extraireCouleur(documentJSON);
    commanderLed();
#ifdef DEBUG_LED
    Serial.print(F("Led::Led(objetJSON)"));
    Serial.print(F("idLed = "));
    Serial.println(idLed);
    Serial.print(F("etat = "));
    Serial.println(etat);
    Serial.print(F("couleur = "));
    Serial.println(couleur);
    Serial.print(F("broche = "));
    Serial.println(broche);
#endif
}

int Led::getIdLed() const
{
    return idLed;
}

bool Led::getEtat() const
{
    return etat;
}

int Led::getNumeroBroche() const
{
    return broche;
}

String Led::getCouleur() const
{
    return couleur;
}

JsonObject Led::getObjetJSON() const
{
    StaticJsonDocument<TAILLE_JSON> documentJSON;
    JsonObject                      objetJSON = documentJSON.as<JsonObject>();
    objetJSON["idLed"]                        = idLed;
    objetJSON["etat"]                         = etat;
    objetJSON["couleur"]                      = couleur;
    objetJSON["broche"]                       = broche;
#ifdef DEBUG_LED
    Serial.print(F("objetJSON : "));
    serializeJson(documentJSON, Serial);
    Serial.println();
#endif
    return objetJSON;
}

void Led::setEtat(bool etat)
{
    // changement d'état ?
    if(etat != this->etat)
    {
        this->etat = etat;
        commanderLed();
    }
}

void Led::setNumeroBroche(int broche)
{
    // changement de broche ?
    if(broche != this->broche)
    {
        this->broche = broche;
    }
}

void Led::setCouleur(String couleur)
{
    this->couleur = couleur;
}

void Led::setCouleur(CouleurLed couleur)
{
    this->couleur = getNomCouleur(couleur);
}

/**
 * @brief Extrait l'idLed dans les données JSON
 * @fn Led::extraireIdLed
 * @return int l'idLed ou -1
 */
int Led::extraireIdLed(StaticJsonDocument<TAILLE_JSON>& documentJSON)
{
    JsonObject objetJSON = documentJSON.as<JsonObject>();
    if(objetJSON.containsKey("idLed"))
    {
#ifdef DEBUG_LED
        Serial.print(F("idLed : "));
        Serial.println(documentJSON["idLed"].as<int>());
#endif
        return documentJSON["idLed"].as<int>();
    }
    return -1;
}

/**
 * @brief Extrait l'état dans les données JSON
 * @fn Led::extraireEtat
 * @return bool
 */
bool Led::extraireEtat(StaticJsonDocument<TAILLE_JSON>& documentJSON)
{
    JsonObject objetJSON = documentJSON.as<JsonObject>();
    if(objetJSON.containsKey("etat"))
    {
#ifdef DEBUG_LED
        Serial.print(F("etat : "));
        Serial.println(documentJSON["etat"].as<bool>());
#endif

        return documentJSON["etat"].as<bool>();
    }
    return false;
}

/**
 * @brief Extrait le numéro de broche dans les données JSON
 * @fn Led::extraireNumeroBroche
 * @return int
 */
int Led::extraireNumeroBroche(StaticJsonDocument<TAILLE_JSON>& documentJSON)
{
    JsonObject objetJSON = documentJSON.as<JsonObject>();
    if(objetJSON.containsKey("broche"))
    {
#ifdef DEBUG_LED
        Serial.print(F("broche : "));
        Serial.println(documentJSON["broche"].as<int>());
#endif

        return documentJSON["broche"].as<int>();
    }
    return -1;
}

/**
 * @brief Extrait la couleur dans les données JSON
 * @fn Led::extraireNumeroBroche
 * @return String
 */
String Led::extraireCouleur(StaticJsonDocument<TAILLE_JSON>& documentJSON)
{
    JsonObject objetJSON = documentJSON.as<JsonObject>();
    if(objetJSON.containsKey("couleur"))
    {
#ifdef DEBUG_LED
        Serial.print(F("couleur : "));
        Serial.println(documentJSON["couleur"].as<String>());
#endif

        return documentJSON["couleur"].as<String>();
    }
    return String();
}

void Led::commanderLed()
{
    digitalWrite(broche, int(etat));
}

void Led::setObjetJSON(JsonObject& objetJSON) const
{
    objetJSON["idLed"]   = getIdLed();
    objetJSON["etat"]    = getEtat();
    objetJSON["couleur"] = getCouleur();
    objetJSON["broche"]  = getNumeroBroche();
}

bool Led::changementBroche(int broche) const
{
    // changement de broche ?
    if(broche != this->broche)
    {
        return true;
    }
    return false;
}

String Led::getNomCouleur(CouleurLed couleur)
{
    switch(couleur)
    {
        case ROUGE:
            return "rouge";
        case VERTE:
            return "verte";
        case ORANGE:
            return "orange";
        default:
            return "";
    }
}
