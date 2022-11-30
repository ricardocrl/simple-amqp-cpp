#include "ThreadUtils.hpp"

void ThreadUtils::SimpleEvent::wait()
{
    std::unique_lock<std::mutex> lk{mMutex};
    mConditionVariable.wait(lk, [this] { return mDone; });
}

bool ThreadUtils::SimpleEvent::waitFor(Duration timeout)
{
    std::unique_lock<std::mutex> lk{mMutex};
    return mConditionVariable.wait_for(lk, timeout, [this] { return mDone; });
}

void ThreadUtils::SimpleEvent::notify()
{
    {
        std::unique_lock<std::mutex> lk{mMutex};
        mDone = true;
    }
    mConditionVariable.notify_one();
}

bool ThreadUtils::SimpleEvent::expired()
{
    std::unique_lock<std::mutex> lk{mMutex};
    return mDone;
}

void ThreadUtils::SimpleEvent::reset()
{
    std::unique_lock<std::mutex> lk{mMutex};
    mDone = false;
}
