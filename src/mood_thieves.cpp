#include "mood_thieves/mood_thieves.hpp"

namespace mood_thieves
{

MoodThieve::MoodThieve(MPI_Datatype message_type, int id, int size)
    : clock(utils::LamportClock{0, id}), message_type(message_type), size(size)
{
}

void MoodThieve::start()
{
    // FIXME: Do something
}

} // namespace mood_thieves
