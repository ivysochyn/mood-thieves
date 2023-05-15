#pragma once

#include <mpi.h>

#include "mood_thieves/utils.hpp"

namespace mood_thieves
{

/**
 * A thief that wanders around town and steals other people's moods.
 */
class MoodThieve
{
private:
    /**
     * Creates and broadcasts the message to all other thieves including itself.
     *
     * @param message_type The type of message to send.
     * @param resource_type The type of resource to send.
     */
    void sendMessage(int message_type, int resource_type);

    /**
     * Request a weapon from other thieves.
     *
     * @param resource_type The type of resource to request for.
     */
    void sendRequest(int resource_type);

    utils::LamportClock clock; ///< The Lamport clock.
    MPI_Datatype message_type; ///< The type of message to use for communication with other thieves.
    int size;                  ///< The total number of thieves.

public:
    /**
     * Constructor
     *
     * @param message_type The type of message to use for communication with other thieves.
     * @param id The identifier of the thief.
     * @param size The total number of thieves.
     */
    MoodThieve(MPI_Datatype message_type, int id, int size);

    /**
     * Destructor
     */
    ~MoodThieve(){};

    /**
     * Start the thief.
     */
    void start();
};

} // namespace mood_thieves
