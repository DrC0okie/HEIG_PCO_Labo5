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
    if(nbClientsInSalon == 0)
        firstClientId = clientId;

    nbClientsInSalon++;

    std::cerr << "Client " << clientId << " is trying to access the salon" << std::endl;
    // The salon is fullanimationClientSitOnChair
    if(nbClientsInSalon >= capacity) {
        std::cerr << "Client " << clientId << ": There is : " << nbClientsInSalon << " clients in the salon, I go for a walk" << std::endl;
        _mutex.unlock();
        return false;
    }

    std::cerr << "Client " << clientId << ": I enter the salon, now there is " << nbClientsInSalon << " clients in the salon" << std::endl;
    animationClientAccessEntrance(clientId);

    if(barberSleeping){
        std::cerr << "Client " << clientId << ": The barber is sleeping, I wake him up" << std::endl;
        barberAvailable.notifyOne(); // Wake up the barber if sleeping
    }

    // Go directly for hair cut if the barber is available and no one is waiting
    if (workingChairFree && clientId == firstClientId) {
        std::cerr << "Client " << clientId << ": The working chair is available, and I am the first client in the shop" << std::endl;
        _mutex.unlock();
        return true;
    }
    std::cerr << "Client " << clientId << ": The working chair state: " << workingChairFree << " and there is " << nbClientsInSalon << " client(s) in the salon. I must take a ticket" << std::endl;

    // Else take a ticket and wait for the barber to be ready
    size_t clientTicket = currentTicket++;
    std::cerr << "Client " << clientId << ": My ticket number is " << clientTicket << ". Now I wait" << std::endl;
    animationClientSitOnChair(clientId, clientTicket);

    // Wait for the barber to be ready
    while (clientTicket != nextServeTicket || nbClientsInSalon >= capacity) {
        clientAvailable.wait(&_mutex);
    }

    std::cerr << "Client " << clientId << ": I have been notified that the ticket #" << nextServeTicket << " must go to the chair. Now I stop waiting" << std::endl;

    _mutex.unlock();
    return true;
}

void PcoSalon::goForHairCut(unsigned clientId) {
    _mutex.lock();
    std::cerr << "Client " << clientId << ": I'm sitting on the barber chair" << std::endl;
    animationClientSitOnWorkChair(clientId);

    // Notify the barber that the client is on the working chair
    workingChairFree = false;
    std::cerr << "Client " << clientId << ": Now the chair is not free anymore. I notify the barber that i'm ready" << std::endl;
    clientOnWorkingChair.notifyOne();

    // Wait for the barber to notify the job done
    beautifyDone.wait(&_mutex);
    std::cerr << "Client " << clientId << ": The barber has finished it's cut" << std::endl;

    std::cerr << "Client " << clientId << ": I notify the barber that I leaved the chair" << std::endl;
    workingChairFree = true;

    clientOnWorkingChair.notifyOne();
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
    barberSleeping = true;
    animationBarberGoToSleep();
    while (nbClientsInSalon == 0) {  // Sleep if no clients are
        std::cerr << "Barber : Zzzz..." << std::endl;
        barberAvailable.wait(&_mutex);
    }
    std::cerr << "The barber is awakening" << std::endl;
    barberSleeping = false;
    animationWakeUpBarber();
    std::cerr << "The barber is fully awake" << std::endl;
    _mutex.unlock();
}

void PcoSalon::pickNextClient() {
    _mutex.lock();
    std::cerr << "Barber: I must pick a client" << std::endl;
    // A client is already in the working chair
//    if(!workingChairFree){
//        std::cerr << "Barber: The working chair is not free" << std::endl;
//        _mutex.unlock();
//        return;
//    }

//    std::cerr << "Barber: The working chair is free" << std::endl;

    if (nbClientsInSalon > 0) {
        std::cerr << "Barber: There is "<< nbClientsInSalon << "client(s) in the salon, notify the ticket #" << nextServeTicket << std::endl;
        // call the next client
        nextServeTicket++;
        clientAvailable.notifyAll();
        _mutex.unlock();
        return;
    }

    std::cerr << "Barber: There is "<< nbClientsInSalon << "client(s) in the salon, do nothing" << std::endl;
    _mutex.unlock();
}


void PcoSalon::waitClientAtChair() {
    _mutex.lock();
    while(workingChairFree){
        std::cerr << "Barber: I'm waiting the client at the chair" << std::endl;
        clientOnWorkingChair.wait(&_mutex);
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
    beautifyDone.notifyOne();

    std::cerr << "Barber: I'm waiting that the client leaves the chair" << std::endl;
    clientOnWorkingChair.wait(&_mutex);

    nbClientsInSalon--;
    std::cerr << "Barber: Now I decrement the number of client in the shop. There are " << nbClientsInSalon << " in the shop" << std::endl;

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
    firstClientId = 0;
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
