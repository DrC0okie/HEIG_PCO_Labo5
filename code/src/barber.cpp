/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */
// Modifications à faire dans le fichier

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
        }

        _interface->consoleAppendTextBarber("J'appelle le cient suivant");
        _salon->pickNextClient();
        _interface->consoleAppendTextBarber("J'attends que le client vienne sur la chaise");
        _salon->waitClientAtChair();
        _interface->consoleAppendTextBarber("Je vais coiffer le client");
        _salon->beautifyClient();
    }
    _interface->consoleAppendTextBarber("La journée est terminée, à demain !");
}
