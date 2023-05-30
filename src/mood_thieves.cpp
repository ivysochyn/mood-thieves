#include "mood_thieves/mood_thieves.hpp"
#define DEBUG false
#include <algorithm>

namespace mood_thieves
{

MoodThieve::MoodThieve(MPI_Datatype message_type, int id, int size)
    : clock(utils::LamportClock{id}), message_type(message_type), size(size)
{
    logic_thread = std::thread(&MoodThieve::business_logic, this);
    clocks = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++)
    {
        clocks[i] = 0;
    }
}

MoodThieve::~MoodThieve()
{
    end.store(true);
    logic_thread.join();
    free(clocks);
}

void MoodThieve::receiveMessages()
{
    utils::message_data_t message_data;
    MPI_Status status;
    int message_available = 0;
    while (1)
    {
        if (end.load())
        {
            break;
        }

        // Check if there is a message available
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &message_available, &status);

        if (!message_available)
        {
            continue;
        }

        message_data = {-1, -1, -1};
        MPI_Recv(&message_data, 1, message_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (DEBUG)
        {
            printf("[DEBUG] Thief %d received message from %d with tag %d", clock.id, status.MPI_SOURCE,
                   status.MPI_TAG);
            printf(" | ID | CLOCK | RESOURCE : %d | %d | %d |\n", message_data.id, message_data.clock,
                   message_data.resource_type);
        }
        utils::message_t message = {status.MPI_TAG, message_data};

        // Update the clocks status with the latest clock
        clocks[message_data.id] = message_data.clock;

        // Update the clock
        clock.lock();
        clock.update(message_data.clock);
        clock.increment();
        clock.unlock();

        // If the message is request
        if (status.MPI_TAG == utils::MessageType::REQUEST)
        {
            // Add the message to the vector of messages and sort it in descending order
            message_vector.push_back(message);
            std::sort(message_vector.begin(), message_vector.end(),
                      [](const utils::message_t &a, const utils::message_t &b)
                      {
                          if (a.data.clock == b.data.clock)
                          {
                              return a.data.id < b.data.id;
                          }
                          else
                          {
                              return a.data.clock < b.data.clock;
                          }
                      });
            clock.lock();
            clock.increment();
            sendAck(message_data.resource_type, message_data.id);
            clock.unlock();
        }
        else if (status.MPI_TAG == utils::MessageType::RELEASE)
        {
            // Remove the message from the vector of messages
            message_vector.erase(std::remove_if(message_vector.begin(), message_vector.end(),
                                                [message](const utils::message_t &m)
                                                { return m.data.id == message.data.id; }),
                                 message_vector.end());
        }
    }
}

void MoodThieve::business_logic()
{
    MPI_Barrier(MPI_COMM_WORLD);
    if (clock.id == 0)
    {
        clock.lock();
        clock.increment();
        sendRequest(utils::ResourceType::WEAPON);
        clock.unlock();
    }
    while (1)
    {
        if (end.load())
        {
            break;
        }

        clock.lock();
        // if can't find a request with clock id equal to this clock.id
        if (std::find_if(message_vector.begin(), message_vector.end(),
                         [this](const utils::message_t &m)
                         { return m.data.id == this->clock.id; }) == message_vector.end())
        {
            clock.increment();
            sendRequest(utils::ResourceType::WEAPON);
        }
        clock.unlock();

        if (message_vector.size() == 0)
        {
            continue;
        }
        // If the first message is from the thief itself
        if (message_vector[0].data.id == clock.id)
        {
            // FIXME: Remove this message
            printf("Thief %d is stealing the weapon\n", clock.id);
            sleep(3);
            clock.lock();
            clock.increment();
            message_vector.erase(message_vector.begin());
            sendRelease(utils::ResourceType::WEAPON);
            clock.unlock();
            // FIXME: Remove this message
            printf("Thief %d has released the weapon\n", clock.id);
        }
    }
}

void MoodThieve::sendRequest(int resource_type) { sendMessage(utils::MessageType::REQUEST, resource_type); }

void MoodThieve::sendAck(int resource_type, int thief_id)
{
    utils::message_data_t message_data = {clock.id, clock.clock, resource_type};
    MPI_Send(&message_data, 1, this->message_type, thief_id, utils::MessageType::ACK, MPI_COMM_WORLD);
}

void MoodThieve::sendRelease(int resource_type) { sendMessage(utils::MessageType::RELEASE, resource_type); }

void MoodThieve::sendMessage(int message_type, int resource_type)
{
    utils::message_data_t message_data = {clock.id, clock.clock, resource_type};
    for (int i = 0; i < size; i++)
    {
        MPI_Send(&message_data, 1, this->message_type, i, message_type, MPI_COMM_WORLD);
    }
}

} // namespace mood_thieves
