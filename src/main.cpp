#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "mood_thieves/utils.hpp"

namespace mood_thieves
{

struct thread_data
{
    // Personal clock
    // int clock;
    // FIFO queue of messages
    // std::queue<Message> queue;
};

void startFunc(int rank, int size)
{
    printf("Starting %d of %d\n", rank, size);
    printf("Finishing %d of %d\n", rank, size);
}

} // namespace mood_thieves

int main(int argc, char **argv)
{
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    int err = mood_thieves::utils::check_thread_support(provided);

    if (err == -1)
    {
        printf("Error: MPI does not have MPI_THREAD_MULTIPLE support\n");
        MPI_Abort(MPI_COMM_WORLD, err);
        return 1;
    }

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    mood_thieves::startFunc(rank, size);

    MPI_Finalize();
    return 0;
}
