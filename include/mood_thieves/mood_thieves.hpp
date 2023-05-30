#pragma once

#include <atomic>
#include <condition_variable>
#include <mpi.h>
#include <mutex>
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
     * Checks whether thieve's clock is the smallest among all other thieves.
     *
     * @return True if the thief's clock is the smallest, false otherwise.
     */
    bool isSmallestClock();

    utils::LamportClock clock; ///< The Lamport clock.
    MPI_Datatype msg_t;        ///< The type of message to use for communication with other thieves.
    int size;                  ///< The total number of thieves.

    std::atomic<bool> end{false};         ///< Flag to indicate that the thief receiving thread should end.
    std::mutex message_data_vector_mutex; ///< Mutex to protect the message data vector.
    std::thread logic_thread;             ///< The thread responsible for handling business logic.
    std::condition_variable cv;           ///< Condition variable to unsleep the business logic thread;
    std::mutex cv_mutex;                  ///< Mutex to protect the condition variable.

    std::vector<utils::message_data_t> message_data_vector; ///< The queue of messages received from other thieves.

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
     * Business logic of the thief in an infinity loop.
     */
    void business_logic();

    /**
     * Receives messages from other thieves in an infinity loop.
     */
    void receiveMessages();
};

} // namespace mood_thieves
