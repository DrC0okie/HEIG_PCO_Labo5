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
    while (true) {
        if (_salon->getNbClient() <= 0) {
            _interface->consoleAppendTextBarber("Je m'endors...");
            _salon->goToSleep();
            _interface->consoleAppendTextBarber("Je me réveille !");
        } else {
            _salon->pickNextClient();
            _interface->consoleAppendTextBarber("Je vais chercher un client !");
        }
        _salon->waitClientAtChair();
        _interface->consoleAppendTextBarber("Je coiffe un client !");
        _salon->beautifyClient();
        _interface->consoleAppendTextBarber("J'ai fini de coiffer un client !");
    }
    _interface->consoleAppendTextBarber("La journée est terminée, à demain !");
}
