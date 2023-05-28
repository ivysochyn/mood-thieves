#pragma once

#include <mpi.h>

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
 * Struct to hold the Lamport clock and the id of the thread.
 */
struct LamportClock
{
    /**
     * Increment the clock by one.
     */
    void increment() { clock++; }

    /**
     * Decrement the clock by one.
     */
    void decrement() { clock--; }

    int clock; ///< Lamport clock
    int id;    ///< Id of the thread
};

/**
 * Struct to hold data for a messages to send.
 */
struct message_data_t
{
    // Personal clock for each thread
    LamportClock clock;

    // Type of the util the message is about
    int resource_type;
};

/**
 * Struct to hold a message to receive.
 */
struct message_t
{
    // Type of the message
    int type;

    // Data of the message
    message_data_t data;
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
