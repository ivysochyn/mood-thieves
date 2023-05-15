#include "mood_thieves/mood_thieves.hpp"

namespace mood_thieves
{

MoodThieve::MoodThieve(MPI_Datatype message_type, int id, int size)
    : clock(utils::LamportClock{0, id}), message_type(message_type), size(size)
{
}

void MoodThieve::start() { sendRequest(utils::ResourceType::WEAPON); }

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
