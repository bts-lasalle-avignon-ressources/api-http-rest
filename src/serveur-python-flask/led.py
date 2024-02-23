# coding: utf-8
# Thierry Vaira

class Led(object):
    def __init__(self, idLed: int=None, etat: bool=None, couleur: str=None, broche: int=None):
        self._idLed = idLed
        self._etat = etat
        self._couleur = couleur
        self._broche = broche

    def serialize(self) -> dict:
        return {
            "idLed": self._idLed,
            "etat": self._etat,
            "couleur": self._couleur,
            "broche": self._broche
        }

    def setObjetJSON(self, objetJSON: dict):
        self._idLed = objetJSON["idLed"]
        self.etat = objetJSON["etat"]
        self.couleur = objetJSON["couleur"]
        self.broche = objetJSON["broche"]

    def detecterChangementBroche(self, broche: int) -> bool:
        if broche != self._broche:
            return True
        return False

    def commander(self):
        #if self._etat:
        #    GPIO.output(LED_PIN, GPIO.HIGH)
        #else:
        #    GPIO.output(LED_PIN, GPIO.LOW)
        pass

    @property
    def idLed(self) -> int:
        return self._idLed

    @idLed.setter
    def idLed(self, idLed: int):
        if idLed is None:
            raise ValueError("Valeur invalide pour id_led")
        self._idLed = idLed

    @property
    def etat(self) -> bool:
        return self._etat

    @etat.setter
    def etat(self, etat: bool):
        if etat is None:
            raise ValueError("Valeur invalide pour etat")
        if etat != self._etat:
            self._etat = etat
            self.commander()

    @property
    def couleur(self) -> str:
        return self._couleur

    @couleur.setter
    def couleur(self, couleur: str):
        couleursValides = ["rouge", "verte", "orange"]
        if couleur not in couleursValides:
            raise ValueError(
                "Valeur invalide pour couleur ({0}), valeurs autorisées : {1}"
                .format(couleur, couleursValides)
            )
        self._couleur = couleur

    @property
    def broche(self) -> int:
        return self._broche

    @broche.setter
    def broche(self, broche: int):
        if broche is None:
            raise ValueError("Valeur invalide pour etat")
        #Raspberry Pi 3 modèle B+ : GPIO 2-27, GPIO 40-45
        #Raspberry Pi 4 modèle B  : GPIO 2-27, GPIO 40-45
        brochesValides = [4, 5, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27]
        if broche not in brochesValides:
            raise ValueError(
                "Valeur invalide pour broche ({0}), valeurs autorisées : {1}"
                .format(broche, brochesValides)
            )
        if broche != self._broche:
            self._broche = broche
