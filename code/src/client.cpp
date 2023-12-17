/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */

/**
* @file client.cpp
* @brief Implementation of the Client class methods
* @author Aubry Mangold <aubry.mangold@heig-vd.ch>
* @author Timothée Van Hove <timothee.vanhove@heig-vd.ch>
* @date 2023-12-17
 */

#include "client.h"

#include <unistd.h>

#include <iostream>

int Client::_nextId = 0;

Client::Client(GraphicSalonInterface *interface,
               std::shared_ptr<SalonClientInterface> salon)

    : _interface(interface), _salon(salon), _clientId(_nextId++) {
    _interface->consoleAppendTextClient(_clientId, "Salut, prêt pour une coupe !");
}

void Client::run() {
    while (_salon->isInService()) {
        _interface->consoleAppendTextClient(_clientId, "Je regarde s'il y a de la place dans le salon");
        if (!_salon->accessSalon(_clientId)) {
            _interface->consoleAppendTextClient(_clientId, "Le salon est plein, je reviens plus tard");
            _salon->walkAround(_clientId);
            continue;
        }
        _interface->consoleAppendTextClient(_clientId, "Je vais sur la chaise du barbier");
        _salon->goForHairCut(_clientId);
        _interface->consoleAppendTextClient(_clientId, "J'attends que mes cheveux repoussent");
        _salon->waitingForHairToGrow(_clientId);
    }
    _interface->consoleAppendTextClient(_clientId, "Le salon est fermé... Zut !");
    _salon->goHome(_clientId);
}
