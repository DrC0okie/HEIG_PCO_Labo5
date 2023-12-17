/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \|___ \
 * | |__) | |   | |  | |    ) | | | | ) | __) |
 * |  ___/| |   | |  | |   / /| | | |/ / |__ <
 * | |    | |___| |__| |  / /_| |_| / /_ ___) |
 * |_|     \_____\____/  |____|\___/____|____/
 */

/**
* @file salon.h
* @author Aubry Mangold <aubry.mangold@heig-vd.ch>
* @author Timothée Van Hove <timothee.vanhove@heig-vd.ch>
* @date 2023-12-17
 */

#ifndef SALON_H
#define SALON_H

#include "salonbarberinterface.h"
#include "salonclientinterface.h"
#include "utils/barberActions.h"
#include "utils/clientActions.h"

typedef ClientAction CA;
typedef BarberAction BA;

class Salon : public SalonBarberInterface, public SalonClientInterface
{

    std::vector<BA> barberDebugData;
    std::vector<std::vector<CA>> clientsDebugData;

public:

/********************************************
     * testing methods *
*******************************************/

void addBarberAction(BA value) {
    barberDebugData.push_back(value);
}

void addClientAction(unsigned clientId, CA value) {
    // Ensure the vector is large enough
    if (clientId >= clientsDebugData.size()) {
        clientsDebugData.resize(clientId + 1);
    }
    clientsDebugData[clientId].push_back(value);
}

[[nodiscard]] const std::vector<BA>& getBarberDebugData() const {
    return barberDebugData;
}

[[nodiscard]] const std::vector<std::vector<CA>>& getClientsDebugData() const {
    return clientsDebugData;
}

    /*!
     * \brief endService Permet au barbier de fermer le salon
     */
    virtual void endService() = 0;
};

#endif // SALON_H
