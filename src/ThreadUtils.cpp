#include "ThreadUtils.hpp"

namespace utils
{

void SimpleEvent::wait()
{
    std::unique_lock<std::mutex> lk{mMutex};
    mConditionVariable.wait(lk, [this] { return mDone; });
}

bool SimpleEvent::waitFor(Duration timeout)
{
    std::unique_lock<std::mutex> lk{mMutex};
    return mConditionVariable.wait_for(lk, timeout, [this] { return mDone; });
}

void SimpleEvent::notify()
{
    {
        std::unique_lock<std::mutex> lk{mMutex};
        mDone = true;
    }
    mConditionVariable.notify_one();
}

bool SimpleEvent::expired()
{
    std::unique_lock<std::mutex> lk{mMutex};
    return mDone;
}

void SimpleEvent::reset()
{
    std::unique_lock<std::mutex> lk{mMutex};
    mDone = false;
}

} // namespace utils
