/**
* @file tests.h
* @brief Tests class header file
* @author Aubry Mangold <aubry.mangold@heig-vd.ch>
* @author Timothée Van Hove <timothee.vanhove@heig-vd.ch>
* @date 2023-12-17
*/

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
    },
    {// Rare case when the barber is awake when the first client is entering the salon
     CA::CHECK_PLACE,
     CA::ENTER_SALON,
     CA::BARBER_AWAKE,
     CA::WALKING_TO_BARBER_CHAIR,
     CA::SIT_ON_BARBER_CHAIR,
     CA::LEAVE_BARBER_CHAIR,
     CA::WAIT_FOR_HAIR_TO_GROW
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
     },
    {// Rare case when the barber is awake when the first client is entering the salon
     BA::WAIT_CLIENT_AT_CHAIR,
     BA::CUT_CLIENT_HAIR
     }
};

/**
 * @class Tests
 * @authors Aubry Mangold, Timothée Van Hove
 * @brief Provides methods to test the sequences of actions taken by clients and the barber in the simulation.
 */
class Tests {
    public:
    /**
     * @brief Tests the sequences of actions taken by clients against predefined valid sequences.
     *
     * Analyses the sequences recorded for each client and checks if they follow one of the valid sequences
     * defined for client actions.
     *
     * @param clientsData A vector of vectors, each representing the sequence of actions for a client.
     */
    static void testClientsSequence(const std::vector<std::vector<CA>>& clientsData);

    /**
     * @brief Tests the sequence of actions taken by the barber against predefined valid sequences.
     *
     * Analyses the barber's recorded sequence of actions and checks if they follow one of the valid
     * sequences defined for barber actions.
     *
     * @param barberData A vector representing the sequence of actions taken by the barber.
     */
    static void testBarberSequence(const std::vector<BA>& barberData);

    private:

    /**
     * @brief Checks if a given sequence of client actions is valid.
     *
     * Compares a given sequence of actions against a set of valid sequences to determine
     * if the sequence is valid for a client.
     *
     * @param sequence A vector representing a sequence of client actions.
     * @param validSequences A vector of vectors, each representing a valid sequence of client actions.
     * @return true If the sequence matches any of the valid sequences, false otherwise.
     */
    static bool isClientValidSequence(
        const std::vector<CA>& sequence, const std::vector<std::vector<CA>>& validSequences);

    /**
     * @brief Checks if a given sequence of barber actions is valid.
     *
     * Compares a given sequence of actions against a set of valid sequences to determine
     * if the sequence is valid for the barber.
     *
     * @param sequence A vector representing a sequence of barber actions.
     * @param validSequences A vector of vectors, each representing a valid sequence of barber actions.
     * @return true If the sequence matches any of the valid sequences, false otherwise.
     */
    static bool isBarberSequenceValid(
        const std::vector<BA>& sequence, const std::vector<std::vector<BA>>& validSequences);

    /**
     * @brief Action that can be repeated in a client sequence
     */
    static const CA REPEATED_ACTION = CA::WAIT_FOR_TURN;
};

#endif  // PCO_TESTS_H
