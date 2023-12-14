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
    : _interface(interface), capacity(capacity + 1)
{
    // TODO
}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
unsigned int PcoSalon::getNbClient() {
    _mutex.lock();
    unsigned int count = nbClientsInSalon;
    _mutex.unlock();
    return count;
}

bool PcoSalon::accessSalon(unsigned clientId) {
    _mutex.lock();

    // The salon is fullanimationClientSitOnChair
    if(nbClientsInSalon >= capacity) {
        _mutex.unlock();
        return false;
    }

    nbClientsInSalon++;
    animationClientAccessEntrance(clientId);

    if(barberSleeping)
        barberAvailable.notifyOne(); // Wake up the barber if sleeping

    // Go directly for hair cut if the barber is available and no one is waiting
    if (workingChairAvailable && nbClientsInSalon <= 1) {
        workingChairAvailable = false;
        _mutex.unlock();
        return true;
    }

    // Else take a ticket and wait for the barber to be ready
    size_t clientTicket = currentTicket++;
    animationClientSitOnChair(clientId, clientTicket);

    // Wait for the barber to be ready
    while (clientTicket != nextServeTicket || nbClientsInSalon >= capacity) {
        clientAvailable.wait(&_mutex);
    }

    _mutex.unlock();
    return true;
}

void PcoSalon::goForHairCut(unsigned clientId) {
    _mutex.lock();
    animationClientSitOnWorkChair(clientId);

    // Notify the barber that the client is on the working chair
    clientOnWorkingChair.notifyOne();

    // Wait for the barber to notify the job done
    beautifyDone.wait(&_mutex);

    // Free the working chair
    workingChairAvailable = true;
    _mutex.unlock();
}

void PcoSalon::waitingForHairToGrow(unsigned clientId) {
    // Simulate waiting for hair to grow
    _mutex.lock();
    animationClientWaitForHairToGrow(clientId);
    _mutex.unlock();
}


void PcoSalon::walkAround(unsigned clientId) {
    // Simulate client walking around
    _mutex.lock();
    animationClientWalkAround(clientId);
    _mutex.unlock();
}


void PcoSalon::goHome(unsigned clientId) {
    // Simulate client going home
    _mutex.lock();
    animationClientGoHome(clientId);
    _mutex.unlock();
}


/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
void PcoSalon::goToSleep() {
    _mutex.lock();
    animationBarberGoToSleep();
    while (nbClientsInSalon == 0) {  // Sleep if no clients are waiting
        barberSleeping = true;
        barberAvailable.wait(&_mutex);
    }
    barberSleeping = false;
    animationWakeUpBarber();
    _mutex.unlock();
}

void PcoSalon::pickNextClient() {
    _mutex.lock();

    // A client is already in the working chair
    if(!workingChairAvailable){
        _mutex.unlock();
        return;
    }

    if (nbClientsInSalon > 0) {
        // call the next client
        nextServeTicket++;
        clientAvailable.notifyAll();
    }
    _mutex.unlock();
}


void PcoSalon::waitClientAtChair() {
    _mutex.lock();
    clientOnWorkingChair.wait(&_mutex);
    _mutex.unlock();
}


void PcoSalon::beautifyClient() {
    _mutex.lock();
    animationBarberCuttingHair();

    // The cut is done
    beautifyDone.notifyOne();
    nbClientsInSalon--;

    _mutex.unlock();
}

/********************************************
 *    Méthodes générales de l'interface     *
 *******************************************/
bool PcoSalon::isInService() {
    // TODO
}


void PcoSalon::endService() {
    _mutex.lock();
    currentTicket = 0;
    nextServeTicket = 0;
    nbClientsInSalon = 0;
    _mutex.unlock();
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
