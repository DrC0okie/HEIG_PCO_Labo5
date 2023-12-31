/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */

/**
* @file barber.cpp
* @brief Implementation of the Barber class methods
* @author Aubry Mangold <aubry.mangold@heig-vd.ch>
* @author Timothée Van Hove <timothee.vanhove@heig-vd.ch>
* @date 2023-12-17
 */

#include "barber.h"

#include <unistd.h>

#include <iostream>

Barber::Barber(GraphicSalonInterface *interface,
               std::shared_ptr<SalonBarberInterface> salon)
    : _interface(interface), _salon(salon) {
    _interface->consoleAppendTextBarber("Salut, prêt à travailler !");
}

void Barber::run() {
    while (_salon->isInService() || _salon->getNbClient() > 0){
        _interface->consoleAppendTextBarber("Je suis prêt à accueillir un client");
        if (_salon->getNbClient() == 0) {
            _interface->consoleAppendTextBarber("Pas de client, je vais dormir");
            _salon->goToSleep();
            continue;
        }else{
            _interface->consoleAppendTextBarber("J'appelle le client suivant");
            _salon->pickNextClient();
        }

        _interface->consoleAppendTextBarber("J'attends que le client vienne sur la chaise");
        _salon->waitClientAtChair();
        _interface->consoleAppendTextBarber("Je vais coiffer le client");
        _salon->beautifyClient();
    }
    _interface->consoleAppendTextBarber("La journée est terminée, à demain !");
}
