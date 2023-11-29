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

PcoSalon::PcoSalon(GraphicSalonInterface *interface, unsigned int capacity)
    : _interface(interface),
      _isFree(true),
      _isFull(false),
      _capacity(capacity)
{
    // TODO
}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
unsigned int PcoSalon::getNbClient()
{
    // TODO return the size of the queue
    _mutex.lock();
    const std::size_t clientCount = _clients.size();
    _mutex.unlock();
    return clientCount;
}

bool PcoSalon::accessSalon(unsigned clientId)
{
    // TODO corresponds to PUT
    _mutex.lock();

    while(_clients.size() == _capacity) {
        // Go for a walk then come back
        _isFree.wait(&_mutex);
    }

    _clients.push(clientId);

    _isFull.notifyOne();
    _mutex.unlock();

    // animationClientAccessEntrance(clientId);
}


void PcoSalon::goForHairCut(unsigned clientId)
{
    // TODO corresponds to GET
    if (_clients.empty())
        return; // FIXME: check if this is necessary

    _mutex.lock();
    while(_clients.size() < _capacity)
        _isFull.wait(&_mutex);

    _clients.pop();

    _isFree.notifyOne();
    _mutex.unlock();

    // animationClientSitOnChair(clientId);
}

void PcoSalon::waitingForHairToGrow(unsigned clientId)
{
    // TODO Clients waits before trying to go back to the salon
    // animationClientWaitForHairToGrow
}


void PcoSalon::walkAround(unsigned clientId)
{
    // TODO Client waits for room in the salon
}


void PcoSalon::goHome(unsigned clientId){
    // TODO Clients goes back home at the end of the sim
}


/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
void PcoSalon::goToSleep()
{
    // TODO
}


void PcoSalon::pickNextClient()
{
    // TODO

    // animationClientSitOnWorkChair(clientId);
}


void PcoSalon::waitClientAtChair()
{
    // TODO
}


void PcoSalon::beautifyClient()
{
    // TODO
}

/********************************************
 *    Méthodes générales de l'interface     *
 *******************************************/
bool PcoSalon::isInService()
{
    // TODO
}


void PcoSalon::endService()
{
    // TODO
}

/********************************************
 *   Méthodes privées pour les animations   *
 *******************************************/
void PcoSalon::animationClientAccessEntrance(unsigned clientId)
{
    _mutex.unlock();
    _interface->clientAccessEntrance(clientId);
    _mutex.lock();
}

void PcoSalon::animationClientSitOnChair(unsigned clientId, unsigned clientSitNb)
{
    _mutex.unlock();
    _interface->clientSitOnChair(clientId, clientSitNb);
    _mutex.lock();
}

void PcoSalon::animationClientSitOnWorkChair(unsigned clientId)
{
    _mutex.unlock();
    _interface->clientSitOnWorkChair(clientId);
    _mutex.lock();
}

void PcoSalon::animationClientWaitForHairToGrow(unsigned clientId)
{
    _mutex.unlock();
    _interface->clientWaitHairToGrow(clientId, true);
    _mutex.lock();
}

void PcoSalon::animationClientWalkAround(unsigned clientId)
{
    _mutex.unlock();
    _interface->clientWalkAround(clientId);
    _mutex.lock();
}

void PcoSalon::animationBarberGoToSleep()
{
    _mutex.unlock();
    _interface->barberGoToSleep();
    _mutex.lock();
}

void PcoSalon::animationWakeUpBarber()
{
    _mutex.unlock();
    _interface->clientWakeUpBarber();
    _mutex.lock();
}

void PcoSalon::animationBarberCuttingHair()
{
    _mutex.unlock();
    _interface->barberCuttingHair();
    _mutex.lock();
}

void PcoSalon::animationClientGoHome(unsigned clientId){
    _mutex.unlock();
    _interface->clientWaitHairToGrow(clientId, false);
    _mutex.lock();
}
