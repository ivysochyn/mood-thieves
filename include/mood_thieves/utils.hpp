#pragma once

#include <mpi.h>
#include <mutex>

namespace mood_thieves
{
namespace utils
{

// Enum representing the resource type
enum ResourceType
{
    WEAPON,
    LABORATORY
};

// Enum representing the message type
enum MessageType
{
    REQUEST,
    ACK,
    RELEASE
};

/**
 * Struct to hold data for a messages to send.
 */
struct message_data_t
{
    int id;            ///< Id of the thread
    int clock;         ///< Lamport clock value
    int resource_type; ///< Type of the resource
};

/**
 * Struct to hold a message to receive.
 */
struct message_t
{
    int type;            ///< Type of the message
    message_data_t data; ///< Data of the message
};

/**
 * Struct to hold the Lamport clock and the id of the thread.
 */
struct LamportClock
{
    /**
     * Lock the clock.
     *
     * Applies the mutex to the clock.
     */
    void lock() { _mutex.lock(); }

    /**
     * Unlock the clock.
     *
     * Releases the mutex from the clock.
     */
    void unlock() { _mutex.unlock(); }

    /**
     * Update the clock with the maximum of the current clock and the given clock.
     *
     * @param other_clock The clock to compare with.
     */
    void update(const LamportClock &other_clock) { clock = std::max(clock, other_clock.clock); }

    /**
     * Increment the clock.
     */
    void increment() { clock++; }

    /**
     * Constructor.
     *
     * @param id The id of the thread.
     */
    LamportClock(int id) : clock(0), id(id), _mutex(){};

    /**
     * Constructor.
     *
     * @param clock The clock to copy.
     */
    LamportClock(const LamportClock &clock) : clock(clock.clock), id(clock.id), _mutex(){};

    int clock;         ///< Lamport clock
    int id;            ///< Id of the thread
    std::mutex _mutex; ///< Mutex responsible for locking the clock
};

/**
 * Check the thread support level of MPI.
 *
 * @param provided The thread support level provided by MPI.
 *
 * @return Status code, -1 if insufficient thread support.
 */
int check_thread_support(int provided);

/**
 * Initialize the message type for MPI.
 * The message type is a struct containing the Lamport clock
 * and the id of the thread.
 *
 * @param message_type The MPI_Datatype to initialize.
 */
void initialize_message_type(MPI_Datatype &message_type);

/**
 * Free the message type for MPI.
 *
 * @param message_type The MPI_Datatype to free.
 */
void free_message_type(MPI_Datatype &message_type);

} // namespace utils
} // namespace mood_thieves
