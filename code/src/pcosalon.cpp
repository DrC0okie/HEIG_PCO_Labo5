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

    // The salon is full
    if (_nbClientsInside >= _capacity) {
        _mutex.unlock();
        return false;
    }

    // Check if the client is the first one in the salon
    if (_nbClientsInside == 0) {
        _firstClientId = clientId;
        _clientToBarberChair = true;
    }

    _nbClientsInside++;
    animationClientAccessEntrance(clientId);

    // Wake up the barber if sleeping
    if (_barberSleeping)
        _barber.notifyOne();

    // Go directly for hair cut if first client in the salon
    if (_clientToBarberChair && clientId == _firstClientId) {
        _mutex.unlock();
        return true;
    }

    // Take a ticket and wait for the barber to be ready
    size_t clientTicket = _currentTicket++;
    animationClientSitOnChair(clientId, _freeChairIndex);
    _freeChairIndex = (_freeChairIndex + 1) % _nbChairs;

    // Wait for the barber to be ready
    while (clientTicket != _nextTicket || _nbClientsInside >= _capacity)
        _client.wait(&_mutex);

    _mutex.unlock();
    return true;
}

void PcoSalon::goForHairCut(unsigned clientId) {
    _mutex.lock();
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
}

void PcoSalon::waitingForHairToGrow(unsigned clientId) {
    _mutex.lock();
    animationClientWaitForHairToGrow(clientId);
    _mutex.unlock();
}


void PcoSalon::walkAround(unsigned clientId) {
    _mutex.lock();
    animationClientWalkAround(clientId);
    _mutex.unlock();
}


void PcoSalon::goHome(unsigned clientId) {
    _mutex.lock();
    animationClientGoHome(clientId);
    _mutex.unlock();
}

/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
void PcoSalon::goToSleep() {
    _mutex.lock();
    _barberSleeping = true;
    animationBarberGoToSleep();

    // Wait for a client to wake the barber up
    while (_nbClientsInside == 0 && _inService)
        _barber.wait(&_mutex);

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
    while (_barberChairFree || _clientToBarberChair)
        _clientOnWorkingChair.wait(&_mutex);

    _mutex.unlock();
}


void PcoSalon::beautifyClient() {
    _mutex.lock();
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
