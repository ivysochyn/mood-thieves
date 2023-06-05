#include "mood_thieves/mood_thieves.hpp"
#define DEBUG false
#define LABORATORIES_N 1
#define SLEEP_TIME 3
#define WEAPONS_N 3
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
            if (message.data.resource_type == utils::ResourceType::WEAPON)
            {
                // Add the message to the vector of messages and sort it in descending order
                weapons_data_vector_mutex.lock();
                weapons_data_vector.push_back(message.data);
                std::sort(weapons_data_vector.begin(), weapons_data_vector.end(),
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

                if (isWeapon())
                {
                    wv.notify_one();
                }

                weapons_data_vector_mutex.unlock();
            } else if (message.data.resource_type == utils::ResourceType::LABORATORY)
            {
                laborotories_data_vector_mutex.lock();
                laborotories_data_vector.push_back(message.data);
                std::sort(laborotories_data_vector.begin(), laborotories_data_vector.end(),
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

                if (isLaboratory())
                {
                    lv.notify_one();
                }

                laborotories_data_vector_mutex.unlock();
            }

            if (message.data.id == clock.id)
            {
                lv.notify_one();
                wv.notify_one();
            }
            clock.lock();
            clock.increment();
            sendAck(message_data.resource_type, message_data.id);
            clock.unlock();

        }
        else if (status.MPI_TAG == utils::MessageType::RELEASE)
        {
            // Remove the message from the vector of messages
            if (message.data.resource_type == utils::ResourceType::WEAPON)
            {
                weapons_data_vector_mutex.lock();
                weapons_data_vector.erase(std::remove_if(weapons_data_vector.begin(), weapons_data_vector.end(),
                                                         [message](const utils::message_data_t &m)
                                                         { return m.id == message.data.id; }),
                                          weapons_data_vector.end());
                if (weapons_data_vector.size() > 0 && isWeapon())
                {
                    wv.notify_one();
                }
                weapons_data_vector_mutex.unlock();
            } else if (message.data.resource_type == utils::ResourceType::LABORATORY)
            {
                laborotories_data_vector_mutex.lock();
                laborotories_data_vector.erase(std::remove_if(laborotories_data_vector.begin(), laborotories_data_vector.end(),
                                                         [message](const utils::message_data_t &m)
                                                         { return m.id == message.data.id; }),
                                          laborotories_data_vector.end());
                if (laborotories_data_vector.size() > 0 && isLaboratory())
                {
                    lv.notify_one();
                }
                laborotories_data_vector_mutex.unlock();
            }
        } else if (status.MPI_TAG == utils::MessageType::ACK)
        {
            if (message.data.resource_type == utils::ResourceType::WEAPON)
            {
                weapons_data_vector_mutex.lock();
                weapons_ack++;
                if (isWeapon())
                {
                    wv.notify_one();
                }
                weapons_data_vector_mutex.unlock();
            } else if (message.data.resource_type == utils::ResourceType::LABORATORY)
            {
                laborotories_data_vector_mutex.lock();
                laboratories_ack++;
                if (isLaboratory())
                {
                    lv.notify_one();
                }
                laborotories_data_vector_mutex.unlock();
            }
        }
    }
}

void MoodThieve::business_logic()
{
    MPI_Barrier(MPI_COMM_WORLD);
    bool first = true;
    while (1)
    {
        if (end.load())
        {
            break;
        }

        // Check whether in a queue
        weapons_data_vector_mutex.lock();
        if (std::find_if(weapons_data_vector.begin(), weapons_data_vector.end(),
                         [this](const utils::message_data_t &m)
                         { return m.id == this->clock.id; }) == weapons_data_vector.end())
        {
            // Send request for a critical section
            clock.lock();
            clock.increment();
            sendRequest(utils::ResourceType::WEAPON);
            clock.unlock();
            weapons_data_vector_mutex.unlock();

            // Wait until this process request is in the queue
            std::unique_lock<std::mutex> lk(wv_mutex);
            wv.wait(lk);
        }
        else
        {
            weapons_data_vector_mutex.unlock();
        }

        if (first)
        {
            first = false;
            MPI_Barrier(MPI_COMM_WORLD);
        }

        std::unique_lock<std::mutex> lk(wv_mutex);
        do
        {
            weapons_data_vector_mutex.lock();
            if (isWeapon())
            {
                weapons_data_vector_mutex.unlock();
                break;
            }
            weapons_data_vector_mutex.unlock();
            // Wait for 100 micro seconds
            wv.wait_for(lk, std::chrono::microseconds(100));
        } while (1);

        // Take weapon
        printf("\n[%d] TAKE WEAPON\n", clock.id);
        weapons_data_vector_mutex.lock();
        weapons_ack = 0;
        weapons_data_vector_mutex.unlock();
        sleep(SLEEP_TIME);

        // // Request laboratory
        clock.lock();
        clock.increment();
        sendRequest(utils::ResourceType::LABORATORY);
        clock.unlock();
        lv.wait(lk);

        do
        {
            laborotories_data_vector_mutex.lock();
            if (isLaboratory())
            {
                laborotories_data_vector_mutex.unlock();
                break;
            }
            laborotories_data_vector_mutex.unlock();
            // Wait for 100 micro seconds
            lv.wait_for(lk, std::chrono::microseconds(100));
        } while (1);

        // Enter laboratory
        printf("[%d] ENTER LAB\n", clock.id);
        laborotories_data_vector_mutex.lock();
        laboratories_ack = 0;
        laborotories_data_vector_mutex.unlock();
        sleep(SLEEP_TIME);

        // Release laboratory
        printf("[%d] LEAVE LAB | CLOCK: %d \n", clock.id, clock.clock);
        clock.lock();
        clock.increment();
        sendRelease(utils::ResourceType::LABORATORY);
        clock.unlock();

        // Release weapon
        printf("[%d] RELEASE WEAPON | CLOCK: %d \n", clock.id, clock.clock);
        clock.lock();
        clock.increment();
        sendRelease(utils::ResourceType::WEAPON);
        clock.unlock();


        // Remove the message from the vector of messages (in order not to re-enter the critical section)
        laborotories_data_vector_mutex.lock();
        laborotories_data_vector.erase(std::remove_if(laborotories_data_vector.begin(), laborotories_data_vector.end(),
                                                      [this](const utils::message_data_t &m)
                                                      { return m.id == this->clock.id; }),
                                       laborotories_data_vector.end());
        laborotories_data_vector_mutex.unlock();

        // Remove the message from the vector of messages (in order not to re-enter the critical section)
        weapons_data_vector_mutex.lock();
        weapons_data_vector.erase(std::remove_if(weapons_data_vector.begin(), weapons_data_vector.end(),
                                                 [this](const utils::message_data_t &m)
                                                 { return m.id == this->clock.id; }),
                                  weapons_data_vector.end());
        weapons_data_vector_mutex.unlock();
    }
}

bool MoodThieve::isWeapon()
{
    if (weapons_ack != size)
    {
        return false;
    }
    int counter = 0;
    for (auto &m : weapons_data_vector)
    {
        if (m.id == clock.id)
        {
            return true;
        }
        counter++;
        if (counter == WEAPONS_N)
        {
            break;
        }
    }
    return false;
}

bool MoodThieve::isLaboratory()
{
    if (laboratories_ack != size)
    {
        return false;
    }
    int counter = 0;
    for (auto &m : laborotories_data_vector)
    {
        if (m.id == clock.id)
        {
            return true;
        }
        counter++;
        if (counter == LABORATORIES_N)
        {
            break;
        }
    }
    return false;
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
