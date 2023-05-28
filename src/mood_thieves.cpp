#include "mood_thieves/mood_thieves.hpp"
#define DEBUG false
#include <algorithm>

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
    utils::message_data_t message_data = {utils::LamportClock{-1, -1}, -1};
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

        if (DEBUG)
        {
            printf("[DEBUG] Thief %d received message from %d with tag %d", this->clock.id, status.MPI_SOURCE,
                   status.MPI_TAG);
            printf(" with clock %d and with resource %d\n", message_data.clock.clock, message_data.resource_type);
        }
        utils::message_t message = {status.MPI_TAG, message_data};
        message_vector.push_back(message);

        // Sort the vector by clock
        std::sort(message_vector.begin(), message_vector.end(),
                  [](const utils::message_t &a, const utils::message_t &b)
                  { return a.data.clock.clock < b.data.clock.clock; });
    }
}

void MoodThieve::start()
{
    MPI_Barrier(MPI_COMM_WORLD);

    // TODO: Replace with business logic
    sendRequest(utils::ResourceType::WEAPON);
    sendRelease(utils::ResourceType::LABORATORY);
    for (int i = 0; i < size; i++)
    {
        sendAck(utils::ResourceType::WEAPON, i);
    }

    // FIXME: Wait for 1 second
    sleep(1);
}

void MoodThieve::sendRequest(int resource_type)
{
    clock.increment();
    sendMessage(utils::MessageType::REQUEST, resource_type);
}

void MoodThieve::sendAck(int resource_type, int thief_id)
{
    utils::message_data_t message_data = {clock, resource_type};
    MPI_Send(&message_data, 1, this->message_type, thief_id, utils::MessageType::ACK, MPI_COMM_WORLD);
}

void MoodThieve::sendRelease(int resource_type)
{
    clock.increment();
    sendMessage(utils::MessageType::RELEASE, resource_type);
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
