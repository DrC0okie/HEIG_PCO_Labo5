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
      _nbWaitingChairs(nb_chairs),
      _inService(true)
{
    // TODO
}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
unsigned int PcoSalon::getNbClient() {
    _mutex.lock();
    unsigned int count = _nbClientsInSalon;
    _mutex.unlock();
    return count;
}

bool PcoSalon::accessSalon(unsigned clientId) {
    _mutex.lock();
    std::cerr << "Client " << clientId << " is trying to access the salon" << std::endl;

    // The salon is full
    if(_nbClientsInSalon >= _capacity) {
        _mutex.unlock();
        return false;
    }

    // Check if the client is the first one in the salon
    if(_nbClientsInSalon == 0)
        _firstClientId = clientId;

    _nbClientsInSalon++;
    animationClientAccessEntrance(clientId);
    std::cerr << "Client " << clientId << ": I enter the salon, now there are " << _nbClientsInSalon << " clients in the salon" << std::endl;

    if(_barberSleeping){
        std::cerr << "Client " << clientId << ": The barber is sleeping, I wake him up" << std::endl;
        _barberAvailable.notifyOne(); // Wake up the barber if sleeping
    }

    // Go directly for hair cut if the barber is available and no one is waiting
    if (_workingChairFree && clientId == _firstClientId) {
        std::cerr << "Client " << clientId << ": The working chair is available, and I am the first client in the shop" << std::endl;
        _clientWalkingToWorkingChair = true;
        _mutex.unlock();
        return true;
    }
    std::cerr << "Client " << clientId << ": there are " << _nbClientsInSalon << " clients in the salon. I must take a ticket" << std::endl;

    // Take a ticket and wait for the barber to be ready
    size_t clientTicket = _currentTicket++;
    std::cerr << "Client " << clientId << ": My ticket number = " << clientTicket << ". Now I wait on the chair #" << freeChairIndex << std::endl;

    animationClientSitOnChair(clientId, freeChairIndex);
    freeChairIndex = (freeChairIndex + 1) % _nbWaitingChairs;

    // Wait for the barber to be ready
    while (clientTicket != _nextServeTicket || _nbClientsInSalon >= _capacity) {
        _clientAvailable.wait(&_mutex);
    }

    std::cerr << "Client " << clientId << ": I have been notified that the ticket #" << _nextServeTicket
              << " must go to the chair. Now I free my wait chair #" << freeChairIndex << std::endl;

    _mutex.unlock();
    return true;
}

void PcoSalon::goForHairCut(unsigned clientId) {
    _mutex.lock();
    std::cerr << "Client " << clientId << ": I'm walking to the working chair" << std::endl;
    animationClientSitOnWorkChair(clientId);

    // Notify the barber that the client is on the working chair
    std::cerr << "Client " << clientId << ": I'm seated in the barber chair. I notify the barber that i'm ready to have my hair cut" << std::endl;
    _workingChairFree = false;
    _clientWalkingToWorkingChair = false;
    _clientOnWorkingChair.notifyOne();

    // Wait for the barber to notify the job done
    _beautifyDone.wait(&_mutex);
    std::cerr << "Client " << clientId << ": The barber has finished it's cut" << std::endl;
    std::cerr << "Client " << clientId << ": I notify the barber that I leaved the chair" << std::endl;
    _workingChairFree = true;
    _clientOnWorkingChair.notifyOne();
    _mutex.unlock();
}

void PcoSalon::waitingForHairToGrow(unsigned clientId) {
    // Simulate waiting for hair to grow
    _mutex.lock();
    std::cerr << "Client " << clientId << ": I'm waiting for my hair to grow" << std::endl;
    animationClientWaitForHairToGrow(clientId);
    _mutex.unlock();
}


void PcoSalon::walkAround(unsigned clientId) {
    // Simulate client walking around
    _mutex.lock();
    std::cerr << "Client " << clientId << ": I'm going to walk around, coming back in a while" << std::endl;
    animationClientWalkAround(clientId);
    _mutex.unlock();
}


void PcoSalon::goHome(unsigned clientId) {
    // Simulate client going home
    _mutex.lock();
    std::cerr << "Client " << clientId << ": I'm going home" << std::endl;
    animationClientGoHome(clientId);
    _mutex.unlock();
}


/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
void PcoSalon::goToSleep() {
    _mutex.lock();
    std::cerr << "Barber : I'm going to sleep" << std::endl;
    _barberSleeping = true;
    animationBarberGoToSleep();
    while (_nbClientsInSalon == 0) {  // Sleep if no clients are
        std::cerr << "Barber : Zzzz..." << std::endl;
        _barberAvailable.wait(&_mutex);
    }
    std::cerr << "The barber is awakening" << std::endl;
    _barberSleeping = false;
    animationWakeUpBarber();
    std::cerr << "The barber is fully awake" << std::endl;
    _mutex.unlock();
}

void PcoSalon::pickNextClient() {
    _mutex.lock();
    // A client is already in the working chair
    if(!_workingChairFree || _clientWalkingToWorkingChair){
        std::cerr << "Barber: A client is waiting on the working chair" << std::endl;
        _mutex.unlock();
        return;
    }

    std::cerr << "Barber: The working chair is free, I must pick a client" << std::endl;

    if (_nbClientsInSalon > 0) {
        _nextServeTicket++;
        std::cerr << "Barber: There are "<< _nbClientsInSalon
                  << " clients in the salon, notify the ticket #" << _nextServeTicket << std::endl;
        // call the next client
        _clientAvailable.notifyAll();
        _mutex.unlock();
        return;
    }

    std::cerr << "Barber: There are "<< _nbClientsInSalon << "client in the salon, do nothing" << std::endl;
    _mutex.unlock();
}


void PcoSalon::waitClientAtChair() {
    _mutex.lock();
    while(_workingChairFree || _clientWalkingToWorkingChair){
        std::cerr << "Barber: I'm waiting the client at the chair" << std::endl;
        _clientOnWorkingChair.wait(&_mutex);
    }
    std::cerr << "Barber: The client is on the chair, I can start" << std::endl;
    _mutex.unlock();
}


void PcoSalon::beautifyClient() {
    _mutex.lock();
    std::cerr << "Barber: I'm starting cutting the hair" << std::endl;
    animationBarberCuttingHair();

    // The cut is done
    std::cerr << "Barber: The cut is done, I notifiy the client on the barber chair" << std::endl;
    _beautifyDone.notifyOne();

    std::cerr << "Barber: I'm waiting that the client leaves the chair" << std::endl;
    _clientOnWorkingChair.wait(&_mutex);

    _nbClientsInSalon--;
    std::cerr << "Barber: Now I decrement the number of client in the shop. There are " << _nbClientsInSalon << " in the shop" << std::endl;

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
    if(_barberSleeping)
        _barberAvailable.notifyOne();
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
