#pragma once

#include <atomic>
#include <mpi.h>
#include <thread>
#include <vector>

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

    /**
     * Acknowledge to a specific thief that the message has been received.
     *
     * @param resource_type The type of resource to acknowledge about.
     * @param thief_id The identifier of the thief to acknowledge to.
     */
    void sendAck(int resource_type, int thief_id);

    /**
     * Sends the release message to all other thieves including itself.
     *
     * @param resource_type The type of resource to release.
     */
    void sendRelease(int resource_type);

    /**
     * Receives messages from other thieves until the end flag is set.
     */
    void receiveMessages();

    utils::LamportClock clock; ///< The Lamport clock.
    MPI_Datatype message_type; ///< The type of message to use for communication with other thieves.
    int size;                  ///< The total number of thieves.

    std::atomic<bool> end{false};                 ///< Flag to indicate that the thief receiving thread should end.
    std::thread receiving_thread;                 ///< The thread that receives messages from other thieves.
    std::vector<utils::message_t> message_vector; ///< The queue of messages received from other thieves.

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
    ~MoodThieve();

    /**
     * Start the thief.
     */
    void start();
};

} // namespace mood_thieves
