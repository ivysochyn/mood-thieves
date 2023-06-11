#pragma once

#include <atomic>
#include <condition_variable>
#include <mpi.h>
#include <mutex>
#include <queue>
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
     * Checks whether thieve can enter the critical section of weapons.
     *
     * @return True if the thief can enter the critical section of weapons, false otherwise.
     */
    bool isWeapon();

    /**
     * Checks whether thieve can enter the critical section of lobaroatories.
     *
     * @return True if the thief can enter the critical section of lobarotories, false otherwise.
     */
    bool isLaboratory();

    /**
     * Frees the weapon after a given timeout.
     * Should be executed in a parallel thread.
     *
     * @param timeout The timeout after which the weapon should be freed.
     */
    void free_weapon_with_timeout(int timeout);

    /**
     * Empties the weapon queue once the weapon is freed.
     * Should be executed in a parallel thread.
     */
    void free_weapon_queue();

    utils::LamportClock clock; ///< The Lamport clock.
    MPI_Datatype msg_t;        ///< The type of message to use for communication with other thieves.
    int size;                  ///< The total number of thieves.

    std::atomic<bool> end{false};                ///< Flag to indicate that the thief receiving thread should end.
    std::thread logic_thread;                    ///< The thread responsible for handling business logic.
    std::thread free_weapon_queue_thread;        ///< The thread responsible for freeing weapons.
    std::queue<std::thread> free_weapon_threads; ///< The threads responsible for freeing weapons.
    std::condition_variable wv; ///< Condition variable to unsleep the business logic thread for a weapon;
    std::condition_variable lv; ///< Condition variable to unsleep the business logic thread for a laboratory;
    std::mutex wv_mutex;        ///< Mutex to protect the condition variable for weapons.
    std::mutex lv_mutex;        ///< Mutex to protect the condition variable for laboratories.
    std::mutex new_mutex;       ///< Mutex to protect the condition variable for laboratories.

    std::vector<utils::message_data_t> weapons_data_vector;      ///< The queue of requests for a weapon.
    std::vector<utils::message_data_t> laborotories_data_vector; ///< The queue of requests for a weapon.
    std::mutex weapons_data_vector_mutex;                        ///< Mutex to protect the weapon requests queue.
    std::mutex laborotories_data_vector_mutex;                   ///< Mutex to protect the weapon requests queue.

    int weapons_ack = 0;
    int laboratories_ack = 0;

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
