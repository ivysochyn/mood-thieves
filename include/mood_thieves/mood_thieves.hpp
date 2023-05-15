#pragma once

#include <mpi.h>

#include "mood_thieves/utils.hpp"

namespace mood_thieves
{

class MoodThieve
{
private:
    utils::LamportClock clock;
    MPI_Datatype message_type;
    int size;

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
    ~MoodThieve(){};

    /**
     * Start the thief.
     */
    void start();
};

} // namespace mood_thieves
