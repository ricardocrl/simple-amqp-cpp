#pragma once

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

class ThreadUtils
{
public:
    /**
     * @class SimpleEvent
     * @brief Allows a calling thread to wait until notified, whilst avoiding spurious wakeups.
     */
    class SimpleEvent
    {
    public:
        using Duration = std::chrono::nanoseconds;

        SimpleEvent() = default;

        /**
         * @brief Wait until notify() is called.
         *
         * This function will return immediately if notify() is called before.
         */
        void wait();

        /**
         * @brief Wait until notify() is called, or until the specified timeout occurs.
         * @param timeout Time to wait before the current thread wakes up.
         * @return True if notify() was called, false if the timeout occurred.
         */
        bool waitFor(Duration timeout);

        /**
         * @brief Notify the waiting thread, if any.
         */
        void notify();

        /**
         * @brief Reset event.
         */
        void reset();

        /**
         * @brief Check if the event was already fired and is, hence, expired
         */
        bool expired();

    private:
        std::mutex mMutex;
        std::condition_variable mConditionVariable;
        bool mDone{false};
    };

    using SimpleEventPtr = std::shared_ptr<SimpleEvent>;

    ThreadUtils()                    = delete;
    ThreadUtils(const ThreadUtils&)  = delete;
    ThreadUtils(const ThreadUtils&&) = delete;
    ~ThreadUtils()                   = delete;
};
