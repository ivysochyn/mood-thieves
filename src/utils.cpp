#include <mpi.h>

#include "mood_thieves/utils.hpp"

namespace mood_thieves
{
namespace utils
{

int check_thread_support(int provided)
{
    switch (provided)
    {
    case MPI_THREAD_FUNNELED:
        printf("Only threads that called mpi_init_thread can make calls to the MPI library\n");
        break;
    case MPI_THREAD_SERIALIZED:
        // Locks are needed around MPI library calls
        printf("Only one thread at a time can make calls to the MPI library\n");
        break;
    case MPI_THREAD_MULTIPLE:
        break;
    case MPI_THREAD_SINGLE:
        printf("No thread support, exiting\n");
        fprintf(stderr, "[ERROR]: Insufficient thread support\n");
        return -1;
    default:
        fprintf(stderr, "[ERROR]: Cannot determine thread support level\n");
        return -1;
    }
    return 0;
}

} // namespace utils
} // namespace mood_thieves
