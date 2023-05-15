#pragma once

#include <mpi.h>

namespace mood_thieves
{
namespace utils
{

struct LamportClock
{
    int clock;
    int id;
};

/**
 * Struct to hold data for each thread.
 */
struct thread_data
{
    // Personal clock for each thread
    LamportClock clock;
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
