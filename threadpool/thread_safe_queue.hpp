// include path in c_cpp_cproperties.json: /Users/purvakulkarni/Desktop/MSVC_Projects/threadpool/thread_safe_queue.hpp

#include "headers.hpp"

// This thread-safe queue will maintain a queue of tasks which can be picked up by the threadpool
// Queue of shared pointers is maintained to avoid duplicates/copies

template<typename T>
class thread_safe_queue{

private:
    std::mutex m;
    std::condition_variable cv;
    std::queue<std::shared_ptr<T> > queue;

    // deleting the copy constructor and copy assignment
    thread_safe_queue(const thread_safe_queue& rhs){}
    thread_safe_queue& operator=(const thread_safe_queue& rhs){}

public:

    // default constructor
    thread_safe_queue();

    // deleting the copy constructor and copy assignment
    // thread_safe_queue(const thread_safe_queue& rhs) = delete;
    // thread_safe_queue& operator=(const thread_safe_queue& rhs) = delete;

    // push a task into the queue
    void push(T&);

    bool pop(T&);

    std::shared_ptr<T> pop();

	void wait_pop(T&);
	std::shared_ptr<T> wait_pop();

	bool empty();

};