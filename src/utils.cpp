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

void initialize_message_type(MPI_Datatype &MPI_PAKIET_T)
{
    const int nitems = 3;
    int blocklengths[nitems] = {1, 1, 1};
    MPI_Datatype typy[nitems] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[nitems];

    // Set the offsets for each field
    offsets[0] = offsetof(LamportClock, clock);
    offsets[1] = offsetof(LamportClock, id);
    offsets[2] = offsetof(message_data_t, resource_type);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);
}

void free_message_type(MPI_Datatype &MPI_PAKIET_T) { MPI_Type_free(&MPI_PAKIET_T); }

} // namespace utils
} // namespace mood_thieves
