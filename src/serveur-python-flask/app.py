#!/usr/bin/env python
# encoding: utf-8
# Thierry Vaira

from flask import Flask, request, abort, jsonify, Response
import led
import RPi.GPIO as GPIO

NB_LEDS_MAX = 8
leds = {}
leds[1] = led.Led(1, False, "rouge", 5)
leds[2] = led.Led(2, False, "verte", 17)

GPIO.setmode(GPIO.BCM)
GPIO.setup(leds[1].broche, GPIO.OUT)
GPIO.setup(leds[2].broche, GPIO.OUT)

app = Flask(__name__)

@app.route('/leds', methods=['GET'])
def getLeds():
    ledsValides = (led for led in leds.values() if led != None)
    return jsonify([led.serialize() for led in ledsValides])

@app.route('/led/<int(min=1,max=8):idLed>', methods=['GET'])
def getLed(idLed):
    if idLed in leds:
        if leds[idLed] == None:
            abort(404, description="Led non trouvée")
        return jsonify(leds[idLed].serialize())
    else:
        abort(404, description="Led non trouvée")

@app.route('/led/<int(min=1,max=8):idLed>', methods=['PUT'])
def upadateLed(idLed):
    if idLed in leds:
        if leds[idLed] == None:
            abort(400)
        donnees = request.form.to_dict()
        objetJSON = {
            "idLed": int(donnees["idLed"]),
            "etat": bool(donnees["etat"]),
            "couleur": donnees["couleur"],
            "broche": int(donnees["broche"])
        }
        if idLed == objetJSON["idLed"]:
            leds[idLed].setObjetJSON(objetJSON)
            return jsonify(leds[idLed].serialize())
        else:
            abort(400)
    else:
        abort(400)

@app.route('/led/<int(min=1,max=8):idLed>', methods=['POST'])
def updateLedWithForm(idLed):
    if idLed in leds:
        if leds[idLed] == None:
            abort(400)
        objetJSON = request.json
        if idLed == objetJSON["idLed"]:
            leds[idLed].setObjetJSON(objetJSON)
            return jsonify(leds[idLed].serialize())
        else:
            abort(400)
    else:
        abort(400)

@app.route('/led/<int(min=1,max=8):idLed>', methods=['DELETE'])
def deleteLed(idLed):
    if idLed in leds:
        # del leds[idLed]
        leds[idLed] = None
        return Response(None, 200)
    else:
        abort(404, description="Led non trouvée")

@app.route('/led', methods=['POST'])
def addLed():
    if len(leds) < NB_LEDS_MAX:
        idLed = len(leds) + 1
        objetJSON = request.json
        leds[idLed] = led.Led(idLed, False, objetJSON["couleur"], objetJSON["broche"])
        return jsonify(leds[idLed].serialize())
    else:
        abort(400, description="Trop de leds")

@app.errorhandler(400)
def traiterRequeteNonValide(error):
    return jsonify({'code': 2,'message': 'La demande est invalide'}), 400

@app.errorhandler(404)
def traiterRequeteNonTrouvee(error):
    return 'Led non trouvée', 404

if __name__ == '__main__':
    app.run()
