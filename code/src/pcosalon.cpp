/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */
// Modifications à faire dans le fichier

#include "pcosalon.h"

#include <pcosynchro/pcothread.h>

#include <iostream>

#include "utils/barberActions.h"
#include "utils/clientActions.h"

typedef ClientAction CA;
typedef BarberAction BA;

PcoSalon::PcoSalon(GraphicSalonInterface *interface, unsigned int nb_chairs)
    : _interface(interface),
      _capacity(nb_chairs + 1),
      _nbChairs(nb_chairs),
      _inService(true) {
    _currentTicket               = 1;
    _nextTicket                  = 0;
    _nbClientsInside             = 0;
    _freeChairIndex              = 0;
    _barberChairFree             = true;
    _clientToBarberChair         = false;
    _barberSleeping              = false;
}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
unsigned int PcoSalon::getNbClient() {
    _mutex.lock();
    unsigned int count = _nbClientsInside;
    _mutex.unlock();
    return count;
}

bool PcoSalon::accessSalon(unsigned clientId) {
    _mutex.lock();

    addClientAction(clientId, CA::CHECK_PLACE);

    // The salon is full
    if (_nbClientsInside >= _capacity) {
        addClientAction(clientId, CA::SALON_FULL);
        _mutex.unlock();
        return false;
    }

    _nbClientsInside++;
    animationClientAccessEntrance(clientId);

    addClientAction(clientId, CA::ENTER_SALON);

    // Wake up the barber if sleeping
    if (_barberSleeping){
        addClientAction(clientId, CA::BARBER_SLEEPING);
        _barber.notifyOne();
        _clientToBarberChair = true;
        _mutex.unlock();
        addClientAction(clientId, CA::WAKE_UP_BARBER);
        return true;
    }

    // Take a ticket and wait for the barber to be ready
    addClientAction(clientId, CA::BARBER_AWAKE);
    size_t clientTicket = _currentTicket++;
    animationClientSitOnChair(clientId, _freeChairIndex);
    _freeChairIndex = (_freeChairIndex + 1) % _nbChairs;

    // Wait for the barber to be ready
    while (clientTicket != _nextTicket || _nbClientsInside >= _capacity){
        addClientAction(clientId, CA::WAIT_FOR_TURN);
        _client.wait(&_mutex);
    }

    _mutex.unlock();
    return true;
}

void PcoSalon::goForHairCut(unsigned clientId) {
    _mutex.lock();
    addClientAction(clientId, CA::WALKING_TO_BARBER_CHAIR);
    animationClientSitOnWorkChair(clientId);

    // Notify the barber that the client is on the working chair
    _barberChairFree = false;
    _clientToBarberChair = false;
    _clientOnWorkingChair.notifyOne();

    // Wait for the barber to finish the hair cut
    _beautifyDone.wait(&_mutex);
    _barberChairFree = true;
    _clientOnWorkingChair.notifyOne();
    _mutex.unlock();
    addClientAction(clientId, CA::SIT_ON_BARBER_CHAIR);
}

void PcoSalon::waitingForHairToGrow(unsigned clientId) {
    _mutex.lock();
    addClientAction(clientId, CA::LEAVE_BARBER_CHAIR);
    animationClientWaitForHairToGrow(clientId);
    addClientAction(clientId, CA::WAIT_FOR_HAIR_TO_GROW);
    _mutex.unlock();
}


void PcoSalon::walkAround(unsigned clientId) {
    _mutex.lock();
    animationClientWalkAround(clientId);
    addClientAction(clientId, CA::WALK_AROUND);
    _mutex.unlock();
}


void PcoSalon::goHome(unsigned clientId) {
    _mutex.lock();
    animationClientGoHome(clientId);
    addClientAction(clientId, ClientAction::GO_HOME);
    _mutex.unlock();
}

/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
void PcoSalon::goToSleep() {
    _mutex.lock();
    addBarberAction(BA::GO_TO_SLEEP);
    _barberSleeping = true;
    animationBarberGoToSleep();

    // Wait for a client to wake the barber up
    while (_nbClientsInside == 0 && _inService)
        _barber.wait(&_mutex);

    addBarberAction(BA::WAKE_UP);
    _barberSleeping = false;
    animationWakeUpBarber();
    _mutex.unlock();
}

void PcoSalon::pickNextClient() {
    _mutex.lock();

    // A client is already in the working chair
    if (!_barberChairFree || _clientToBarberChair) {
        _mutex.unlock();
        return;
    }

    addBarberAction(BA::PICK_NEXT_CLIENT);
    // call the next client
    if (_nbClientsInside > 0) {
        _nextTicket++;
        _client.notifyAll();
        _mutex.unlock();
        return;
    }
    _mutex.unlock();
}


void PcoSalon::waitClientAtChair() {
    _mutex.lock();
    addBarberAction(BA::WAIT_CLIENT_AT_CHAIR);
    while (_barberChairFree || _clientToBarberChair)
        _clientOnWorkingChair.wait(&_mutex);

    _mutex.unlock();
}


void PcoSalon::beautifyClient() {
    _mutex.lock();
    addBarberAction(BA::CUT_CLIENT_HAIR);
    animationBarberCuttingHair();

    // The cut is done
    _beautifyDone.notifyOne();
    _clientOnWorkingChair.wait(&_mutex);

    // The client is leaving
    _nbClientsInside--;
    _mutex.unlock();
}

/********************************************
 *    Méthodes générales de l'interface     *
 *******************************************/
bool PcoSalon::isInService() {
    _mutex.lock();
    const bool inService = _inService;
    _mutex.unlock();
    return inService;
}

void PcoSalon::endService() {
    _mutex.lock();
    _inService = false;
    if (_barberSleeping)
        _barber.notifyOne();
    _mutex.unlock();
}

/********************************************
 *   Méthodes privées pour les animations   *
 *******************************************/
void PcoSalon::animationClientAccessEntrance(unsigned clientId) {
    _mutex.unlock();
    _interface->clientAccessEntrance(clientId);
    _mutex.lock();
}

void PcoSalon::animationClientSitOnChair(unsigned clientId,
                                         unsigned clientSitNb) {
    _mutex.unlock();
    _interface->clientSitOnChair(clientId, clientSitNb);
    _mutex.lock();
}

void PcoSalon::animationClientSitOnWorkChair(unsigned clientId) {
    _mutex.unlock();
    _interface->clientSitOnWorkChair(clientId);
    _mutex.lock();
}

void PcoSalon::animationClientWaitForHairToGrow(unsigned clientId) {
    _mutex.unlock();
    _interface->clientWaitHairToGrow(clientId, true);
    _mutex.lock();
}

void PcoSalon::animationClientWalkAround(unsigned clientId) {
    _mutex.unlock();
    _interface->clientWalkAround(clientId);
    _mutex.lock();
}

void PcoSalon::animationBarberGoToSleep() {
    _mutex.unlock();
    _interface->barberGoToSleep();
    _mutex.lock();
}

void PcoSalon::animationWakeUpBarber() {
    _mutex.unlock();
    _interface->clientWakeUpBarber();
    _mutex.lock();
}

void PcoSalon::animationBarberCuttingHair() {
    _mutex.unlock();
    _interface->barberCuttingHair();
    _mutex.lock();
}

void PcoSalon::animationClientGoHome(unsigned clientId) {
    _mutex.unlock();
    _interface->clientWaitHairToGrow(clientId, false);
    _mutex.lock();
}
