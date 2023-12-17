//
// Created by tim on 17.12.23.
//

#ifndef PCO_TESTS_H
#define PCO_TESTS_H

#include <vector>

#include "barberActions.h"
#include "clientActions.h"

typedef ClientAction CA;
typedef BarberAction BA;

static const std::vector<std::vector<CA>> CLIENT_VALID_SEQ = {
    {
     CA::CHECK_PLACE,
     CA::ENTER_SALON,
     CA::BARBER_AWAKE,
     CA::WAIT_FOR_TURN,
     CA::WALKING_TO_BARBER_CHAIR,
     CA::SIT_ON_BARBER_CHAIR,
     CA::LEAVE_BARBER_CHAIR,
     CA::WAIT_FOR_HAIR_TO_GROW
    },
    {
     CA::CHECK_PLACE,
     CA::ENTER_SALON,
     CA::BARBER_SLEEPING,
     CA::WAKE_UP_BARBER,
     CA::WALKING_TO_BARBER_CHAIR,
     CA::SIT_ON_BARBER_CHAIR,
     CA::LEAVE_BARBER_CHAIR,
     CA::WAIT_FOR_HAIR_TO_GROW
    },
    {
     CA::CHECK_PLACE,
     CA::SALON_FULL,
     CA::WALK_AROUND
    }
};
static const std::vector<std::vector<BA>> BARBER_VALID_SEQ = {
    {
     BA::PICK_NEXT_CLIENT,
     BA::WAIT_CLIENT_AT_CHAIR,
     BA::CUT_CLIENT_HAIR
    },
    {
     BA::GO_TO_SLEEP,
     BA::WAKE_UP,
     BA::WAIT_CLIENT_AT_CHAIR,
     BA::CUT_CLIENT_HAIR
    },
    {// If we stop the simulation while the barber is asleep
     BA::GO_TO_SLEEP,
     BA::WAKE_UP,
     }
};

class Tests {
    public:
    static void testClientsSequence(const std::vector<std::vector<CA>>& clientsData);
    static void testBarberSequence(const std::vector<BA>& barberData);

    private:
    static bool isClientValidSequence(
        const std::vector<CA>& sequence, const std::vector<std::vector<CA>>& validSequences);

    static bool isBarberSequenceValid(
        const std::vector<BA>& sequence, const std::vector<std::vector<BA>>& validSequences);

    static const CA REPEATED_VALUE = CA::WAIT_FOR_TURN;
};

#endif  // PCO_TESTS_H
