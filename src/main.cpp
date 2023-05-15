#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Threads */
#include <pthread.h>

/* Semaphores: sem_init, sem_destroy, sem_post, sem_wait */
// #include <semaphore.h>
/* Flags for open */
// #include <fcntl.h>

/* Boolean */
#define TRUE 1
#define FALSE 0

/* Message types */
#define FINISH 1
#define APP_MSG 2

char passive = FALSE;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int appdata;
    int other_field; // You can change the name to something more appropriate
} packet_t;

void startFunc(int rank, int size)
{
    printf("Starting %d of %d\n", rank, size);
    printf("Finishing %d of %d\n", rank, size);
}

int main(int argc, char **argv)
{
    printf("Beginning\n");
    int provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    switch (provided)
    {
    case MPI_THREAD_SINGLE:
        printf("No thread support, exiting\n");
        // We have to exit
        fprintf(stderr, "Insufficient thread support - exiting!\n");
        MPI_Finalize();
        exit(-1);
        break;
    case MPI_THREAD_FUNNELED:
        printf("Only threads that called mpi_init_thread can make calls to the MPI library\n");
        break;
    case MPI_THREAD_SERIALIZED:
        // Locks are needed around MPI library calls
        printf("Only one thread at a time can make calls to the MPI library\n");
        break;
    case MPI_THREAD_MULTIPLE:
        // Everything is fine
        break;
    default:
        printf("Nobody knows anything\n");
    }

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    startFunc(rank, size);

    MPI_Finalize();
}
