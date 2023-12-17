/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */

/**
* @file application.cpp
* @brief Implementation of the Application class.
* @author Aubry Mangold <aubry.mangold@heig-vd.ch>
* @author Timothée Van Hove <timothee.vanhove@heig-vd.ch>
* @date 2023-12-17
 */

#include "application.h"
#include <pcosynchro/pcologger.h>
#include <iostream>
#include "barber.h"
#include "client.h"
#include "pcosalon.h"
#include "utils/tests.h"

Application::Application(GraphicSalonInterface *interface, unsigned nbPlaces, unsigned nbClients)
    : _interface(interface), _nbPlaces(nbPlaces), _nbClients(nbClients)
{}

void Application::run()
{
    std::vector<std::unique_ptr<Launchable>> threads(0);

    // Création du salon
    std::shared_ptr<Salon> salon = std::make_shared<PcoSalon>(_interface, _nbPlaces);

    // Création du barbier
    std::shared_ptr<Barber> barber = std::make_shared<Barber>(_interface, salon);
    barber->startThread();

    // Création des tâches clients
    for (unsigned int i = 0; i < _nbClients; ++i) {
        threads.push_back(std::make_unique<Client>(_interface, salon));
    }

    // Mélange des tâches afin d'avoir un démarrage dans un ordre aléatoire
    std::random_shuffle(std::begin(threads), std::end(threads));

    // Démarrage des threads
    for (auto& thread : threads) {
        thread->startThread();
    }

    // Attente d'une entrée utilisateur
    QTextStream qin(stdin);
    QString Line = qin.readLine();

    // Fin de service
    salon->endService();

    // Attendre la fin des threads clients
    for (auto& thread : threads) {
        thread->join();
    }

    // Attendre la fin du thread barbier
    barber->join();

    // Run test functions
    std::vector<BA> barberDebugData = salon->getBarberDebugData();
    std::vector<std::vector<CA>> clientsDebugData = salon->getClientsDebugData();
    Tests::testClientsSequence(clientsDebugData);
    Tests::testBarberSequence(barberDebugData);

    logger() << "Le programme est terminé..." << std::endl;
}
