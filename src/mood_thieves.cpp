#include "mood_thieves/mood_thieves.hpp"

namespace mood_thieves
{

MoodThieve::MoodThieve(MPI_Datatype message_type, int id, int size)
    : clock(utils::LamportClock{0, id}), message_type(message_type), size(size)
{
    this->receiving_thread = std::thread(&MoodThieve::receiveMessages, this);
}

MoodThieve::~MoodThieve()
{
    this->end.store(true);
    this->receiving_thread.join();
}

void MoodThieve::receiveMessages()
{
    utils::message_data_t message_data;
    MPI_Status status;
    int message_available = 0;
    while (this->end.load() == false)
    {
        // Check if there is a message available
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &message_available, &status);

        if (!message_available)
        {
            continue;
        }

        MPI_Recv(&message_data, 1, this->message_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG)
        {
        case utils::MessageType::REQUEST:
            printf("Thief %d received REQUEST from %d\n", clock.id, status.MPI_SOURCE);
            break;
        default:
            printf("[WARNING]: Thief %d received UNKNOWN message from %d\n", clock.id, status.MPI_SOURCE);
            break;
        }
    }
}

void MoodThieve::start()
{
    MPI_Barrier(MPI_COMM_WORLD);

    sendRequest(utils::ResourceType::WEAPON);

    // FIXME: Wait for 1 second
    sleep(1);
}

void MoodThieve::sendRequest(int resource_type)
{
    clock.increment();
    sendMessage(utils::MessageType::REQUEST, resource_type);
}

void MoodThieve::sendMessage(int message_type, int resource_type)
{
    utils::message_data_t message_data = {clock, resource_type};
    for (int i = 0; i < size; i++)
    {
        MPI_Send(&message_data, 1, this->message_type, i, message_type, MPI_COMM_WORLD);
    }
}

} // namespace mood_thieves
