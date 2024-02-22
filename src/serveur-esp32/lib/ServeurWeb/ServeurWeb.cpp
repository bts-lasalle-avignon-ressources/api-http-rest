/**
 * @file ServeurWeb.cpp
 * @brief Définition de la classe ServeurWeb
 * @author Thierry VAIRA
 * @version 0.1
 */

#include "ServeurWeb.h"
#include <ESPmDNS.h>
#include <uri/UriRegex.h>

const int    brochesValides[]  = { 4,  5,  13, 14, 16, 17, 18, 19,
                                   21, 22, 23, 25, 26, 27, 32, 33 };
const String couleursValides[] = { "rouge", "verte", "orange" };

/**
 * @brief Constructeur de la classe ServeurWeb
 * @fn ServeurWeb::ServeurWeb
 * @param port Le port du serveur web
 * @details Initialise un nouvel objet ServeurWeb en utilisant le constructeur
 * de la classe de base WebServer.
 */
ServeurWeb::ServeurWeb(uint16_t port /*= PORT_SERVEUR_WEB*/) : WebServer(port)
{
    // on commence avec deux Leds !
    leds.push_back(new Led(1, false, "rouge", GPIO_LED_ROUGE));
    leds.push_back(
      new Led("{idLed: 2, etat: false, couleur: 'verte', broche: " +
              String(GPIO_LED_VERTE) + "}"));

    // initialise les broches libres
    for(int i = 0;
        i <=
        brochesValides[(sizeof(brochesValides) / sizeof(brochesValides[0])) -
                       1];
        ++i)
    {
        if(i == 0 || i == GPIO_LED_ROUGE || i == GPIO_LED_VERTE)
        {
            brochesDisponibles.push_back(false);
        }
        else
        {
            brochesDisponibles.push_back(true);
        }
    }
}

/**
 * @brief Initialise les préférences de l'application
 * @fn ServeurWeb::initialiserPreferences()
 * @details Démarre la gestion des préférences en utilisant la mémoire EEPROM.
 */
void ServeurWeb::initialiserPreferences()
{
    preferences.begin("eeprom", false); // false pour r/w
    // @todo restaurer les valeurs sauvegardées des Leds
}

void ServeurWeb::initialiserLeds()
{
    pinMode(GPIO_LED_ROUGE, OUTPUT);
    pinMode(GPIO_LED_ORANGE, OUTPUT);
    pinMode(GPIO_LED_VERTE, OUTPUT);
    digitalWrite(GPIO_LED_ROUGE, LOW);
    digitalWrite(GPIO_LED_ORANGE, LOW);
    digitalWrite(GPIO_LED_VERTE, LOW);
}

/**
 * @brief Démarre le serveur web et installe les gestionnaires de requêtes
 * @fn ServeurWeb::demarrer
 * @details Cette méthode configure et démarre le serveur Web. Elle définit les
 * gestionnaires de requêtes pour les différentes routes, puis démarre le
 * serveur en utilisant la méthode begin(). Le serveur sera accessible à partir
 * de l'adresse IP locale obtenue via la connexion WiFi.
 */
void ServeurWeb::demarrer()
{
    initialiserLeds();
    setNom();
    installerGestionnairesRequetes();
    begin(); // Démarre le serveur

#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::demarrer()"));
    Serial.print(F("Adresse IP = "));
    Serial.println(WiFi.localIP());
    Serial.println("URL        = http://" + String(NOM_SERVEUR_WEB) +
                   ".local/");
#endif
}

/**
 * @brief traite les requêtes reçues par le serveur web
 * @fn ServeurWeb::traiterRequetes
 * @details Appelle la méthode handleClient() du serveur web pour traiter les
 * requêtes reçues par le serveur.
 */
void ServeurWeb::traiterRequetes()
{
    handleClient();
}

/**
 * @brief Définit le nom du serveur Web
 * @fn ServeurWeb::setNom
 * @param nomStationLumineuse Le nom du serveur Web à utiliser
 * @details Cette méthode permet de définir le nom du serveur Web. Si aucun nom
 * n'est spécifié, le nom par défaut NOM_SERVEUR_WEB sera utilisé. Le serveur
 * sera accessible à partir de l'adresse http://nomStationLumineuse.local/. La
 * méthode utilise le service mDNS pour permettre l'accès au serveur avec un nom
 * d'hôte convivial.
 */
void ServeurWeb::setNom(String nomServeurWeb)
{
    if(nomServeurWeb.isEmpty())
        nomServeurWeb = NOM_SERVEUR_WEB;
    // Pour un accès : http://nomServeurWeb.local/
    if(!MDNS.begin(nomServeurWeb.c_str()))
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.println(F("setNom() Erreur mDNS !"));
#endif
    }
}

/**
 * @brief Installe les gestionnaires de requêtes
 * @fn ServeurWeb::installerGestionnairesRequetes
 * @details Cette méthode installe les gestionnaires de requêtes pour les
 * différentes routes du serveur Web. Les gestionnaires sont définis à l'aide de
 * la méthode on(). Les routes sont associées aux méthodes de traitement des
 * requêtes correspondantes.
 */
void ServeurWeb::installerGestionnairesRequetes()
{
    // L'API HTTP REST
    on("/", HTTP_GET, std::bind(&ServeurWeb::afficherAccueil, this));
    on("/leds", HTTP_GET, std::bind(&ServeurWeb::traiterRequeteGetLeds, this));
    on(UriRegex("/led/([1-" + String(NB_LEDS_MAX) + "]+)$"),
       HTTP_GET,
       std::bind(&ServeurWeb::traiterRequeteGetLed, this));
    on(UriRegex("/led/([1-" + String(NB_LEDS_MAX) + "]+)$"),
       HTTP_POST,
       std::bind(&ServeurWeb::traiterRequeteUpdateLedWithForm, this));
    on(UriRegex("/led/([1-" + String(NB_LEDS_MAX) + "]+)$"),
       HTTP_PUT,
       std::bind(&ServeurWeb::traiterRequeteUpdateLed, this));
    on(UriRegex("/led/([1-" + String(NB_LEDS_MAX) + "]+)$"),
       HTTP_DELETE,
       std::bind(&ServeurWeb::traiterRequeteDeleteLed, this));
    on("/led", HTTP_POST, std::bind(&ServeurWeb::traiterRequeteAddLed, this));
    onNotFound(std::bind(&ServeurWeb::traiterRequeteNonTrouvee, this));
}

/**
 * @brief Affiche une page d'accueil pour le serveur web
 * @fn ServeurWeb::afficherAccueil
 * @details Affiche la page d'accueil du serveur web. La méthode utilise
 la fonction send() pour envoyer la réponse au client HTTP avec un code de
 statut 200 et un type de contenu "text/html"
 */
void ServeurWeb::afficherAccueil()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::afficherAccueil() : requête = "));
    Serial.println((method() == HTTP_GET) ? "GET" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif
    String message = "<h1>Bienvenue sur le serveur HTTP REST</h1>\n";
    message += "<p>Adresse IP : " + WiFi.localIP().toString() + "</p>\n";
    message +=
      "<p>URL        : http://" + String(NOM_SERVEUR_WEB) + ".local/</p>\n";
    message +=
      "<p>Leds       : http://" + String(NOM_SERVEUR_WEB) + ".local/leds</p>\n";
    message += "<p>...</p>\n";
    message += "<p>LaSalle Avignon v1.0</p>\n";
    send(200, F("text/html"), message);
}

/**
 * @brief Traite la requête GET pour obtenir la liste des Leds
 * @fn ServeurWeb::traiterRequeteGetLeds
 */
void ServeurWeb::traiterRequeteGetLeds()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteGetLeds() : requête = "));
    Serial.println((method() == HTTP_GET) ? "GET" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif
    /*  Exemple de réponse :
        [
            {
                "idLed": 1,
                "etat": false,
                "couleur": "rouge",
                "broche": 4
            },
            {
                "idLed": 2,
                "etat": false,
                "couleur": "verte",
                "broche": 5
            }
        ]
    */
    documentJSON.clear();
    for(int i = 0; i < leds.size(); ++i)
    {
        if(leds[i] == nullptr)
        {
            continue;
        }
        JsonObject objetLed = documentJSON.createNestedObject();
        leds[i]->setObjetJSON(objetLed);
    }

    char buffer[TAILLE_JSON];
    serializeJson(documentJSON, buffer);
    send(200, "application/json", buffer);

#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("JSON : "));
    serializeJson(documentJSON, Serial);
    Serial.println();
#endif
}

/**
 * @brief Traite une requête HTTP GET relative à une Led
 * @fn ServeurWeb::traiterRequeteGetLed
 */
void ServeurWeb::traiterRequeteGetLed()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteGetLed() : requête = "));
    Serial.println((method() == HTTP_GET) ? "GET" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
    Serial.print(F("{idLed} : "));
    Serial.println(uri().substring(uri().lastIndexOf('/') + 1).toInt());
#endif

    if(estRequeteGetLedPossible())
    {
        documentJSON.clear();
        JsonObject objetLed = documentJSON.createNestedObject();
        int        idLed = uri().substring(uri().lastIndexOf('/') + 1).toInt();
        leds[idLed - 1]->setObjetJSON(objetLed);

        char buffer[TAILLE_JSON];
        serializeJson(documentJSON, buffer);
        send(200, "application/json", buffer);

#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("JSON : "));
        serializeJson(documentJSON, Serial);
        Serial.println();
#endif
    }
}

/**
 * @brief Traite la requête POST pour mettre à jour une Led
 * @fn ServeurWeb::traiterRequeteUpdateLedWithForm
 */
void ServeurWeb::traiterRequeteUpdateLedWithForm()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(
      F("ServeurWeb::traiterRequeteUpdateLedWithForm() : requête = "));
    Serial.println((method() == HTTP_POST) ? "POST" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
    Serial.print(F("{idLed} : "));
    Serial.println(uri().substring(uri().lastIndexOf('/') + 1).toInt());
    Serial.print(F("Body : "));
    Serial.println(arg("plain"));
#endif

    if(estRequeteUpdateLedWithFormPossible())
    {
        int idLed = extraireIdLed();
        leds[idLed - 1]->setCouleur(extraireCouleur());
        brochesDisponibles[leds[idLed - 1]->getNumeroBroche()] = true;
        leds[idLed - 1]->setNumeroBroche(extraireNumeroBroche());
        brochesDisponibles[extraireNumeroBroche()] = false;
        leds[idLed - 1]->setEtat(extraireEtat());

        documentJSON.clear();
        JsonObject objetLed = documentJSON.createNestedObject();
        leds[idLed - 1]->setObjetJSON(objetLed);

        char buffer[TAILLE_JSON];
        serializeJson(documentJSON, buffer);
        send(200, "application/json", buffer);

#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("JSON : "));
        serializeJson(documentJSON, Serial);
        Serial.println();
#endif
    }
}

/**
 * @brief Traite la requête PUT pour modifier une Led
 * @fn ServeurWeb::traiterRequeteUpdateLed
 */
void ServeurWeb::traiterRequeteUpdateLed()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteUpdateLed() : requête = "));
    Serial.println((method() == HTTP_PUT) ? "PUT" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
    Serial.print(F("{idLed} : "));
    Serial.println(uri().substring(uri().lastIndexOf('/') + 1).toInt());
#endif

    if(estRequeteUpdateLedPossible())
    {
        int  idLed = arg("idLed").toInt();
        bool etat  = (arg("etat") == "1") || (arg("etat") == "true");
        leds[idLed - 1]->setCouleur(arg("couleur"));
        brochesDisponibles[leds[idLed - 1]->getNumeroBroche()] = true;
        leds[idLed - 1]->setNumeroBroche(arg("broche").toInt());
        brochesDisponibles[arg("broche").toInt()] = false;
        leds[idLed - 1]->setEtat(etat);

        documentJSON.clear();
        JsonObject objetLed = documentJSON.createNestedObject();
        leds[idLed - 1]->setObjetJSON(objetLed);

        char buffer[TAILLE_JSON];
        serializeJson(documentJSON, buffer);
        send(200, "application/json", buffer);

#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("JSON : "));
        serializeJson(documentJSON, Serial);
        Serial.println();
#endif
    }
}

/**
 * @brief Traite la requête DELETE pour supprimer une Led
 * @fn ServeurWeb::traiterRequeteDeleteLed
 */
void ServeurWeb::traiterRequeteDeleteLed()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteDeleteLed() : requête = "));
    Serial.println((method() == HTTP_DELETE) ? "DELETE" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
    Serial.print(F("{idLed} : "));
    Serial.println(uri().substring(uri().lastIndexOf('/') + 1).toInt());
#endif

    if(estRequeteDeleteLedPossible())
    {
        int idLed = uri().substring(uri().lastIndexOf('/') + 1).toInt();
        leds[idLed - 1]->setEtat(false);
        brochesDisponibles[leds[idLed - 1]->getNumeroBroche()] = true;
        delete leds[idLed - 1];
        leds[idLed - 1] = nullptr;
    }

    // Pas de réponse pour cette requête
}

/**
 * @brief Traite la requête POST pour ajouter une Led
 * @fn ServeurWeb::traiterRequeteAddLed
 */
void ServeurWeb::traiterRequeteAddLed()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteAddLed() : requête = "));
    Serial.println((method() == HTTP_POST) ? "POST" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
    Serial.print(F("Body : "));
    Serial.println(arg("plain"));
#endif

    /*  Exemple de données reçues :
        {
            "couleur": "rouge",
            "broche": 27
        }
    */

    if(estAjoutPossible())
    {
        int idLed = leds.size() + 1;

        // instancie la nouvelle Led
        leds.push_back(
          new Led(idLed, false, extraireCouleur(), extraireNumeroBroche()));
        brochesDisponibles[extraireNumeroBroche()] = false;

        documentJSON.clear();
        JsonObject objetLed = documentJSON.createNestedObject();
        leds[idLed - 1]->setObjetJSON(objetLed);

        char buffer[TAILLE_JSON];
        serializeJson(documentJSON, buffer);
        send(200, "application/json", buffer);

#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("JSON : "));
        serializeJson(documentJSON, Serial);
        Serial.println();
#endif
    }
}

/**
 * @brief Traite une requête qui n'a pas été trouvée
 * @fn ServeurWeb::traiterRequeteNonTrouvee
 */
void ServeurWeb::traiterRequeteNonTrouvee()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteNonTrouvee()"));
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif

    String message = "404 Led non trouvée\r\n";
    send(404, "text/plain", message);

#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("text/plain : "));
    Serial.println(message);
#endif
}

/**
 * @brief Extrait l'idLed dans les données JSON
 * @fn ServeurWeb::extraireIdLed
 * @return int l'idLed ou -1
 */
int ServeurWeb::extraireIdLed()
{
    JsonObject objetJSON = documentJSON.as<JsonObject>();
    if(objetJSON.containsKey("idLed"))
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("idLed : "));
        Serial.println(documentJSON["idLed"].as<int>());
#endif
        return documentJSON["idLed"].as<int>();
    }
    return -1;
}

/**
 * @brief Extrait l'état dans les données JSON
 * @fn ServeurWeb::extraireEtat
 * @return bool
 */
bool ServeurWeb::extraireEtat()
{
    JsonObject objetJSON = documentJSON.as<JsonObject>();
    if(objetJSON.containsKey("etat"))
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("etat : "));
        Serial.println(documentJSON["etat"].as<bool>());
#endif

        return documentJSON["etat"].as<bool>();
    }
    return false;
}

/**
 * @brief Extrait le numéro de broche dans les données JSON
 * @fn ServeurWeb::extraireNumeroBroche
 * @return int
 */
int ServeurWeb::extraireNumeroBroche()
{
    JsonObject objetJSON = documentJSON.as<JsonObject>();
    if(objetJSON.containsKey("broche"))
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("broche : "));
        Serial.println(documentJSON["broche"].as<int>());
#endif

        return documentJSON["broche"].as<int>();
    }
    return -1;
}

/**
 * @brief Extrait la couleur dans les données JSON
 * @fn ServeurWeb::extraireNumeroBroche
 * @return String
 */
String ServeurWeb::extraireCouleur()
{
    JsonObject objetJSON = documentJSON.as<JsonObject>();
    if(objetJSON.containsKey("couleur"))
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("couleur : "));
        Serial.println(documentJSON["couleur"].as<String>());
#endif

        return documentJSON["couleur"].as<String>();
    }
    return String();
}

bool ServeurWeb::estCouleurValide(String couleur) const
{
    for(int i = 0; i < (sizeof(couleursValides) / sizeof(couleursValides[0]));
        i++)
    {
        if(couleur == couleursValides[i])
        {
            return true;
        }
    }
    return false;
}

bool ServeurWeb::estCouleurValide(CouleurLed couleur) const
{
    for(int i = 0; i < (sizeof(couleursValides) / sizeof(couleursValides[0]));
        i++)
    {
        if(Led::getNomCouleur(couleur) == couleursValides[i])
        {
            return true;
        }
    }
    return false;
}

bool ServeurWeb::estBrocheValide(int broche) const
{
    for(int i = 0; i < (sizeof(brochesValides) / sizeof(brochesValides[0]));
        i++)
    {
        if(broche == brochesValides[i])
        {
            return true;
        }
    }
    return false;
}

bool ServeurWeb::estBrocheDisponible(int broche) const
{
    return brochesDisponibles[broche];
}

bool ServeurWeb::estRequeteGetLedPossible()
{
    int idLed = uri().substring(uri().lastIndexOf('/') + 1).toInt();
    if(idLed < 1 || idLed > leds.size())
    {
        traiterRequeteNonTrouvee();
        return false;
    }
    if(leds[idLed - 1] == nullptr)
    {
        traiterRequeteNonTrouvee();
        return false;
    }
    return true;
}

bool ServeurWeb::estRequeteUpdateLedWithFormPossible()
{
    if(hasArg("plain") == false)
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.println(F("Erreur !"));
#endif
        send(400,
             "application/json",
             "{\"code\": 1,\"message\": \"La demande est incomplète\"}");
        return false;
    }

    String body = arg("plain");
#ifdef DEBUG_SERVEUR_WEB
    Serial.println(body);
#endif
    DeserializationError erreur = deserializeJson(documentJSON, body);
    if(erreur)
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("Erreur deserializeJson() : "));
        Serial.println(erreur.f_str());
#endif
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    int idLed = extraireIdLed();
    if(idLed == -1)
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    else if(idLed > leds.size() ||
            idLed != uri().substring(uri().lastIndexOf('/') + 1).toInt())
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(leds[idLed - 1] == nullptr)
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estCouleurValide(extraireCouleur()))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estBrocheValide(extraireNumeroBroche()))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estBrocheDisponible(extraireNumeroBroche()))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    return true;
}

bool ServeurWeb::estRequeteUpdateLedPossible()
{
    if(hasArg("idLed") == false)
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.println(F("Erreur !"));
#endif
        send(400,
             "application/json",
             "{\"code\": 1,\"message\": \"La demande est incomplète\"}");
        return false;
    }
    int idLed = arg("idLed").toInt();
    if(idLed == -1)
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    else if(idLed > leds.size() ||
            idLed != uri().substring(uri().lastIndexOf('/') + 1).toInt())
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(leds[idLed - 1] == nullptr)
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estCouleurValide(arg("couleur")))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estBrocheValide(arg("broche").toInt()))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estBrocheDisponible(arg("broche").toInt()))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    return true;
}

bool ServeurWeb::estRequeteDeleteLedPossible()
{
    int idLed = uri().substring(uri().lastIndexOf('/') + 1).toInt();
    if(idLed < 1 || idLed > leds.size())
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }

    if(leds[idLed - 1] == nullptr)
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    return true;
}

bool ServeurWeb::estAjoutPossible()
{
    if(hasArg("plain") == false)
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.println(F("Erreur !"));
#endif
        send(400,
             "application/json",
             "{\"code\": 1,\"message\": \"La demande est incomplète\"}");
        return false;
    }
    String body = arg("plain");
#ifdef DEBUG_SERVEUR_WEB
    Serial.println(body);
#endif
    DeserializationError erreur = deserializeJson(documentJSON, body);
    if(erreur)
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.print(F("Erreur deserializeJson() : "));
        Serial.println(erreur.f_str());
#endif
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(extraireCouleur().isEmpty() || extraireNumeroBroche() == -1)
    {
        send(400,
             "application/json",
             "{\"code\": 3,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estCouleurValide(extraireCouleur()))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estBrocheValide(extraireNumeroBroche()))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(!estBrocheDisponible(extraireNumeroBroche()))
    {
        send(400,
             "application/json",
             "{\"code\": 2,\"message\": \"La demande est invalide\"}");
        return false;
    }
    if(leds.size() >= NB_LEDS_MAX)
        return false;
    return true;
}
