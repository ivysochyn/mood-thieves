#pragma once

namespace mood_thieves
{
namespace utils
{

/**
 * Check the thread support level of MPI.
 *
 * @param provided The thread support level provided by MPI.
 *
 * @return Status code, -1 if insufficient thread support.
 */
int check_thread_support(int provided);

} // namespace utils
} // namespace mood_thieves
