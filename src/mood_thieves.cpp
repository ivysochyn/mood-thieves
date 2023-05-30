#include "mood_thieves/mood_thieves.hpp"
#define DEBUG false
#include <algorithm>

namespace mood_thieves
{

MoodThieve::MoodThieve(MPI_Datatype msg_t, int id, int size) : clock(utils::LamportClock{id}), msg_t(msg_t), size(size)
{
    logic_thread = std::thread(&MoodThieve::business_logic, this);
}

MoodThieve::~MoodThieve()
{
    end.store(true);
    logic_thread.join();
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
        message_available = 0;

        message_data = {-1, -1, -1};
        MPI_Recv(&message_data, 1, msg_t, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (DEBUG)
        {
            printf("[DEBUG] Thief %d received message from %d with tag %d", clock.id, status.MPI_SOURCE,
                   status.MPI_TAG);
            printf(" | ID | CLOCK | RESOURCE : %d | %d | %d |\n", message_data.id, message_data.clock,
                   message_data.resource_type);
        }
        utils::message_t message = {status.MPI_TAG, message_data};

        // Compare clocks
        clock.lock();
        clock.update(message_data.clock);
        clock.increment();
        clock.unlock();

        // If the message is request
        if (status.MPI_TAG == utils::MessageType::REQUEST)
        {
            // Add the message to the vector of messages and sort it in descending order
            message_data_vector_mutex.lock();
            message_data_vector.push_back(message.data);
            std::sort(message_data_vector.begin(), message_data_vector.end(),
                      [](const utils::message_data_t &a, const utils::message_data_t &b)
                      {
                          if (a.clock == b.clock)
                          {
                              return a.id < b.id;
                          }
                          else
                          {
                              return a.clock < b.clock;
                          }
                      });
            message_data_vector_mutex.unlock();
            if (message.data.id == clock.id)
            {
                cv.notify_one();
            }
            clock.lock();
            clock.increment();
            sendAck(message_data.resource_type, message_data.id);
            clock.unlock();
        }
        else if (status.MPI_TAG == utils::MessageType::RELEASE)
        {
            // Remove the message from the vector of messages
            message_data_vector_mutex.lock();
            message_data_vector.erase(std::remove_if(message_data_vector.begin(), message_data_vector.end(),
                                                     [message](const utils::message_data_t &m)
                                                     { return m.id == message.data.id; }),
                                      message_data_vector.end());
            if (message_data_vector.size() > 0 && message_data_vector[0].id == clock.id)
            {
                message_data_vector_mutex.unlock();
                cv.notify_one();
            }
            else
            {
                message_data_vector_mutex.unlock();
            }
        }
    }
}

void MoodThieve::business_logic()
{
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    while (1)
    {
        if (end.load())
        {
            break;
        }

        // Check whether in a queue
        message_data_vector_mutex.lock();
        if (std::find_if(message_data_vector.begin(), message_data_vector.end(),
                         [this](const utils::message_data_t &m)
                         { return m.id == this->clock.id; }) == message_data_vector.end())
        {
            // Send request for a critical section
            clock.lock();
            clock.increment();
            sendRequest(utils::ResourceType::WEAPON);
            clock.unlock();
            message_data_vector_mutex.unlock();

            // Wait until this process request is in the queue
            std::unique_lock<std::mutex> lk(cv_mutex);
            cv.wait(lk);
        }
        else
        {
            message_data_vector_mutex.unlock();
        }

        // Wait until the first message in the queue is the current process
        message_data_vector_mutex.lock();
        if (message_data_vector[0].id != clock.id)
        {
            message_data_vector_mutex.unlock();
            std::unique_lock<std::mutex> lk(cv_mutex);
            cv.wait(lk);
        }
        else
        {
            message_data_vector_mutex.unlock();
        }

        // Critical section
        printf("\n[%d] STEAL\n", clock.id);
        sleep(3);
        printf("[%d] RELEASE\n", clock.id);

        // Leave the critical section and send release message
        clock.lock();
        clock.increment();
        sendRelease(utils::ResourceType::WEAPON);
        clock.unlock();

        // Remove the message from the vector of messages (in order not to re-enter the critical section)
        message_data_vector_mutex.lock();
        message_data_vector.erase(std::remove_if(message_data_vector.begin(), message_data_vector.end(),
                                                 [this](const utils::message_data_t &m)
                                                 { return m.id == this->clock.id; }),
                                  message_data_vector.end());
        message_data_vector_mutex.unlock();
    }
}

void MoodThieve::sendRequest(int resource_type) { sendMessage(utils::MessageType::REQUEST, resource_type); }

void MoodThieve::sendAck(int resource_type, int thief_id)
{
    utils::message_data_t message_data = {clock.id, clock.clock, resource_type};
    MPI_Send(&message_data, 1, msg_t, thief_id, utils::MessageType::ACK, MPI_COMM_WORLD);
}

void MoodThieve::sendRelease(int resource_type) { sendMessage(utils::MessageType::RELEASE, resource_type); }

void MoodThieve::sendMessage(int message_type, int resource_type)
{
    utils::message_data_t message_data = {clock.id, clock.clock, resource_type};
    for (int i = 0; i < size; i++)
    {
        MPI_Send(&message_data, 1, msg_t, i, message_type, MPI_COMM_WORLD);
    }
}

} // namespace mood_thieves
