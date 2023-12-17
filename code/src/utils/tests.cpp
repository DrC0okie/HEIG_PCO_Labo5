//
// Created by tim on 17.12.23.
//

#include <algorithm>
#include <iostream>
#include "tests.h"

bool Tests::isClientValidSequence(const std::vector<CA>& sequence, const std::vector<std::vector<CA>>& validSequences) {
    // Check if a sequence is valid, considering repeated values
    auto isSequenceValid = [&sequence](const std::vector<CA>& validSeq) {
        auto it = sequence.begin();
        for (CA val : validSeq) {
            if (val == REPEATED_VALUE) {
                // Skip all consecutive repeated values in the sequence
                it = std::find_if_not(it, sequence.end(), [](CA x) { return x == REPEATED_VALUE; });
            } else {
                // If the current value is not the repeadted value, it must match the value in the valid sequence
                if (it == sequence.end() || *it != val)
                    return false;
                ++it;
            }
        }
        // Check if we have reached the end of the sequence
        return it == sequence.end();
    };

    return std::any_of(validSequences.begin(), validSequences.end(), isSequenceValid);
}

void Tests::testClientsSequence(const std::vector<std::vector<CA>>& clientsData) {
    for (unsigned i = 0; i < clientsData.size(); i++) {
        std::cout << "Client " << i << " : ";
        unsigned start = 0;
        while (start < clientsData[i].size()) {
            unsigned end = std::find(clientsData[i].begin() + start + 1, clientsData[i].end(), CA::CHECK_PLACE) - clientsData[i].begin();
            // If the next '0' is not found, take the rest of the vector
            if (end > clientsData[i].size())
                end = clientsData[i].size();

            std::vector<CA> segment(clientsData[i].begin() + start, clientsData[i].begin() + end);

            // Validate the sequence without considering the last '100'
            if (segment.back() != CA::GO_HOME && !isClientValidSequence(segment, CLIENT_VALID_SEQ)) {
                std::cout << "Invalid sequence: ";
                for (CA val : segment)
                    std::cout << (int)val << " ";

                std::cout << std::endl;
                return;
            }
            start = end;
        }
        std::cout << "All sequences valid for client " << i << std::endl;
    }
}

bool Tests::isBarberSequenceValid(const std::vector<BA>& sequence, const std::vector<std::vector<BA>>& validSequences) {
    return std::any_of(validSequences.begin(), validSequences.end(), [&sequence](const std::vector<BA>& validSeq) {
        return sequence == validSeq;
    });
}

void Tests::testBarberSequence(const std::vector<BA>& barberData) {
    const BA endAction = BA::CUT_CLIENT_HAIR; // The action that marks the end of a sequence.
    unsigned start = 0;
    bool isValid = true;

    while (start < barberData.size()) {
        // Find the next occurrence of the end action to determine the end of the current segment
        unsigned end = std::find(barberData.begin() + start, barberData.end(), endAction) - barberData.begin();

        // If the end action is found, extend the segment to include it
        if (end < barberData.size())
            end++;

        // Extract the current segment
        std::vector<BA> segment(barberData.begin() + start, barberData.begin() + end);

        // Check if the current segment is a valid sequence
        if (!isBarberSequenceValid(segment, BARBER_VALID_SEQ)) {
            std::cout << "Invalid sequence for barber: ";
            for (BA action : segment)
                std::cout << static_cast<int>(action) << " ";

            std::cout << std::endl;
            isValid = false;
            break;
        }
        start = end; // Move to the next segment.
    }

    if (isValid)
        std::cout << "Barber's sequences are all valid." << std::endl;
}