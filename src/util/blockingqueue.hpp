#ifndef UTIL_BLOCKINGQUEUE_HPP
#define UTIL_BLOCKINGQUEUE_HPP

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace util {

/**
 * This class describe a thread safe queue with blocking pop() method.
 * @author Cyrille
 */
template <
    class T,
    class Container = std::deque<T>
>
class BlockingQueue {
    using Queue = Container;
    using Mutex = std::mutex;
    using Lock  = std::unique_lock<Mutex>;
    using CVar  = std::condition_variable;

public:
    using size_type  = typename Queue::size_type;
    using value_type = T;

    template<class Type>
    using RequireValueType = typename std::enable_if<
        std::is_same<typename std::decay<Type>::type, T>::value
    >::type;

private:
    Queue         _queue;
    CVar          _cv;
    mutable Mutex _mutex;

public:
    BlockingQueue() {}                                ///< Empty constructor
    BlockingQueue(BlockingQueue const &) = delete;    ///< Copy constructor is deleted
    BlockingQueue(BlockingQueue &&);                  ///< Move constructor

    /** @return the size of the queue. */
    size_type size() const;

    /** @param t the value to push at the end of the queue */
    template <class Type, class = RequireValueType<Type>>
    void push(Type && t);

    /**
     * Getting first value of the queue. If the queue is empty, this method block
     * the execution until a new element is added to the queue.
     * @return the first element of the queue.
     */
    T pop();

    /**
     * Getting first value of the queue. If the queue is empty, this method block
     * the execution until a new element is added to the queue or until the time is
     * finished.
     * @return a pair containing the first element of the queue and a boolean indicating
     * if it is a valid value (boolean = true if valid).
     */
    template <class Rep, class Period>
    std::pair<T, bool> tryPop(std::chrono::duration<Rep, Period> const & time);
};


template <class T, class C>
BlockingQueue<T, C>::BlockingQueue(BlockingQueue && m)
    : _queue(std::move(m._queue)),
      _mutex(std::move(m._mutex)),
      _cv(std::move(m._cv))
{}

template <class T, class C>
typename BlockingQueue<T, C>::size_type BlockingQueue<T, C>::size() const {
    Lock lock(_mutex);
    return _queue.size();
}

template <class T, class C>
template <class Type, class>
void BlockingQueue<T, C>::push(Type && t) {
    {
        Lock lock(_mutex);
        _queue.push_back(std::forward<Type>(t));
    }
    _cv.notify_one();
}

template <class T, class C>
T BlockingQueue<T, C>::pop() {
    Lock lock(_mutex);
    _cv.wait(lock, [this] { return !_queue.empty(); });
    T t = std::move(_queue.front());
    _queue.pop_front();
    return t;
}

template <class T, class C>
template <class Rep, class Period>
std::pair<T, bool> BlockingQueue<T, C>::tryPop(
        std::chrono::duration<Rep, Period> const & time)
{
    Lock lock(_mutex);
    if (!_cv.wait_for(lock, time, [this] { return !_queue.empty(); }))
        return make_pair(T{}, false);
    T t = std::move(_queue.front());
    _queue.pop_front();
    return make_pair(t, true);
}

} // util

#endif // CORE_MSGQUEUE_H

