/**
 * @file ServeurWeb.cpp
 * @brief Définition de la classe ServeurWeb
 * @author Thierry VAIRA
 * @version 0.1
 */

#include "ServeurWeb.h"
#include <ESPmDNS.h>

/**
 * @brief Constructeur de la classe ServeurWeb
 * @fn ServeurWeb::ServeurWeb
 * @param port Le port du serveur web
 * @details Initialise un nouvel objet ServeurWeb en utilisant le constructeur
 * de la classe de base WebServer.
 */
ServeurWeb::ServeurWeb(uint16_t port /*= PORT_SERVEUR_WEB*/) : WebServer(port)
{
}
/**
 * @brief Initialise les préférences de l'application
 * @fn ServeurWeb::initialiserPreferences()
 * @details Démarre la gestion des préférences en utilisant la mémoire EEPROM.
 */
void ServeurWeb::initialiserPreferences()
{
    preferences.begin("eeprom", false); // false pour r/w
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
    setNom();
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::demarrer() : adresse IP = "));
    Serial.println(WiFi.localIP());
#endif
    // Installe les gestionnaires de requêtes
    on("/", HTTP_GET, std::bind(&ServeurWeb::afficherAccueil, this));
    on("/notifications", std::bind(&ServeurWeb::traiterRequeteGetLeds, this));

    on("/activations",
       HTTP_GET,
       std::bind(&ServeurWeb::traiterRequeteGETActivations, this));
    on("/activation",
       HTTP_POST,
       std::bind(&ServeurWeb::traiterRequetePOSTActivation, this));

    on("/boite", HTTP_GET, std::bind(&ServeurWeb::traiterRequeteGETLed, this));
    on("/boite",
       HTTP_POST,
       std::bind(&ServeurWeb::traiterRequetePOSTLed, this));
    on("/machine",
       HTTP_GET,
       std::bind(&ServeurWeb::traiterRequeteGETMachine, this));
    on("/machine",
       HTTP_POST,
       std::bind(&ServeurWeb::traiterRequetePUTLed,
                 this)); // Ajout de la route /machine en POST
    on("/poubelle",
       HTTP_GET,
       std::bind(&ServeurWeb::traiterRequeteGETPoubelle, this));
    on("/poubelle",
       HTTP_POST,
       std::bind(&ServeurWeb::traiterRequeteDELETELed,
                 this)); // Ajout de la route /poubelle en POST
    on("/intervallePoubelle",
       HTTP_POST,
       std::bind(&ServeurWeb::traiterRequetePOSTIntervallePoubelle, this));

    onNotFound(std::bind(&ServeurWeb::traiterRequeteNonTrouvee, this));

    // Démarre le serveur
    begin();

#ifdef DEBUG_SERVEUR_WEB
    Serial.println(F("ServeurWeb::demarrer() ok"));
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
    else
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.println("setNom() http://" + nomServeurWeb + ".local/");
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
    on("/", HTTP_GET, std::bind(&ServeurWeb::afficherAccueil, this));
    on("/leds", std::bind(&ServeurWeb::traiterRequeteGetLeds, this));
    on("/led", HTTP_GET, std::bind(&ServeurWeb::traiterRequeteGETLed, this));
    on("/led", HTTP_POST, std::bind(&ServeurWeb::traiterRequetePOSTLed, this));
    on("/led", HTTP_PUT, std::bind(&ServeurWeb::traiterRequetePUTLed, this));
    on("/led",
       HTTP_DELETE,
       std::bind(&ServeurWeb::traiterRequeteDELETELed, this));
    onNotFound(std::bind(&ServeurWeb::traiterRequeteNonTrouvee, this));
}

/**
 * @brief Affiche la page d'accueil du serveur web
 * @fn ServeurWeb::afficherAccueil
 * @details Affiche la page d'accueil du serveur web. La méthode utilise
 la fonction send() pour envoyer la réponse au client HTTP avec un code de
 statut 200 et un type de contenu "text/html"
 */
void ServeurWeb::afficherAccueil()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::afficherAccueil() : requête = "));
    Serial.println((method() == HTTP_GET) ? "GET" : "POST");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif
    String message = "<h1>Bienvenue sur le serveur HTTP REST</h1>\n";
    message += "<p>Adresse IP : " + WiFi.localIP().toString() + "</p>\n";
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
    Serial.print(
      F("ServeurWeb::traiterRequeteGETNotifications() : requête = "));
    Serial.println((method() == HTTP_GET) ? "GET" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif

    documentJSON.clear();
    /*
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
    documentJSON["boite"] = stationLumineuse->getEtatBoiteAuxLettres();
    JsonArray machines    = documentJSON.createNestedArray("machines");
    for(int i = 0; i < NB_LEDS_NOTIFICATION_MACHINES; ++i)
    {
        machines.add(stationLumineuse->getEtatMachine(i));
    }
    JsonArray poubelle = documentJSON.createNestedArray("poubelles");
    for(int i = 0; i < NB_LEDS_NOTIFICATION_POUBELLES; ++i)
    {
        poubelle.add(stationLumineuse->getEtatPoubelle(i));
    }

#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("JSON : "));
    serializeJson(documentJSON, Serial);
    Serial.println();
#endif
    char buffer[TAILLE_JSON];
    serializeJson(documentJSON, buffer);
    send(200, "application/json", buffer);
}

/**
 * @brief Traite une requête HTTP GET relative à une Led
 * @fn ServeurWeb::traiterRequeteGETLed
 */
void ServeurWeb::traiterRequeteGETLed()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteGETLed() : requête = "));
    Serial.println((method() == HTTP_GET) ? "GET" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif

    // Répondre à la requête en donnant l'état de la boîte aux lettres
    String jsonResponse = "{\"etat\": ";
    jsonResponse += etat ? "true" : "false";
    jsonResponse += "}";
    send(200, "application/json", jsonResponse);
}

/**
 * @brief Traite la requête POST d'une Led
 * @fn ServeurWeb::traiterRequetePOSTLed
 */
void ServeurWeb::traiterRequetePOSTLed()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequetePOSTLed() : requête = "));
    Serial.println((method() == HTTP_POST) ? "POST" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif

    if(hasArg("plain") == false)
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.println(F("Erreur !"));
#endif
        send(400,
             "application/json",
             "{\"code\": 1,\"message\": \"La demande est incorrecte\"}");
        return;
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
        return;
    }
    else
    {
        JsonObject objetJSON = documentJSON.as<JsonObject>();
        if(objetJSON.containsKey("etat"))
        {
#ifdef DEBUG_SERVEUR_WEB
            Serial.print(F("etat : "));
            Serial.println(documentJSON["etat"].as<bool>());
#endif

            send(200,
                 "application/json",
                 "{\"idLed\": 1,\"etat\": false,\"couleur\": "
                 "\"rouge\",\"broche\": 4}");
        }
        else
        {
#ifdef DEBUG_SERVEUR_WEB
            Serial.print(F("Erreur : champ etat manquant"));
#endif
            send(400,
                 "application/json",
                 "{\"code\": 3,\"message\": \"La demande est incomplète\"}");
            return;
        }
    }
}

/**
 * @brief Traite la requête PUT pour modifier l'état d'une Led
 * @fn ServeurWeb::traiterRequetePUTLed
 */
void ServeurWeb::traiterRequetePUTLed()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequetePOSTMachine() : requête = "));
    Serial.println((method() == HTTP_PUT) ? "PUT" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif

    if(hasArg("plain") == false)
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.println(F("Erreur !"));
#endif
        send(400,
             "application/json",
             "{\"code\": 1,\"message\": \"La demande est incorrecte\"}");
        return;
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
        return;
    }
    else
    {
        send(200,
             "application/json",
             "{\"idLed\": 1,\"etat\": false,\"couleur\": "
             "\"rouge\",\"broche\": 4}");
    }
}

/**
 * @brief Traite la requête DELETE pour supprimer une Led
 * @fn ServeurWeb::traiterRequeteDELETELed
 * @details Récupère la demande POST de la poubelle envoyée par le client et
 vérifie si elle est correcte. Ensuite, détermine l'état de la poubelle et
 l'enregistre dans l'objet StationLumineuse. Si la demande est correcte, envoie
 une réponse JSON de confirmation au client. Si la demande est incorrecte,
 envoie une réponse JSON d'erreur correspondante au client.
 */
void ServeurWeb::traiterRequeteDELETELed()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteDELETELed() : requête = "));
    Serial.println((method() == HTTP_DELETE) ? "DELETE" : "Autre");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif

    if(hasArg("plain") == false)
    {
#ifdef DEBUG_SERVEUR_WEB
        Serial.println(F("Erreur !"));
#endif
        send(400,
             "application/json",
             "{\"code\": 1,\"message\": \"La demande est incorrecte\"}");
        return;
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
        return;
    }
    else
    {
        JsonObject objetJSON = documentJSON.as<JsonObject>();
        if(objetJSON.containsKey("idLed"))
        {
#ifdef DEBUG_SERVEUR_WEB
            Serial.print(F("id : "));
            Serial.println(documentJSON["idLed"].as<int>());
#endif
            send(200,
                 "application/json",
                 "{\"idLed\": 1,\"etat\": false,\"couleur\": "
                 "\"rouge\",\"broche\": 4}");
        }
        else
        {
#ifdef DEBUG_SERVEUR_WEB
            Serial.print(F("Erreur : champ etat ou numeroPoubelle manquant"));
#endif
            send(400,
                 "application/json",
                 "{\"code\": 3,\"message\": \"La demande est incomplète\"}");
            return;
        }
    }
}

/**
 * @brief Traite une requête qui n'a pas été trouvée
 * @fn ServeurWeb::traiterRequeteNonTrouvee
 * @details Récupère les informations de la requête (URI, méthode, arguments) et
 les concatène dans un message d'erreur de type 404. Envoie ensuite ce message
 au client qui a effectué la requête.
 */
void ServeurWeb::traiterRequeteNonTrouvee()
{
#ifdef DEBUG_SERVEUR_WEB
    Serial.print(F("ServeurWeb::traiterRequeteNonTrouvee() : requête = "));
    Serial.println((method() == HTTP_GET) ? "GET" : "POST");
    Serial.print(F("URI : "));
    Serial.println(uri());
#endif

    String message = "404 Led non trouvée\n\n";
    message += "URI: ";
    message += uri();
    message += "\nMethod: ";
    message += (method() == HTTP_GET) ? "GET" : "Autre";
    message += "\nArguments: ";
    message += args();
    message += "\n";
    for(uint8_t i = 0; i < args(); i++)
    {
        message += " " + argName(i) + ": " + arg(i) + "\n";
    }
    send(404, "text/plain", message);
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
