#include "headers.hpp"
#include "thread_safe_queue.hpp"
#include "joiner_threads.hpp"

class thread_pool{

private:

    // atomic bool variable to determine if the thread-pool us active
    // if done = true, discard the thread-pool
    std::atomic<bool> done;

    // vector of worker threads
    // these worker threads pick up task from the pool of tasks
    std::vector<std::thread> worker_threads;

    // this vector will contain a pool of tasks
    // this is a vector of function pointers
    // this is a hard coded vector which contains pointer to functions with return type void and no arguments
    thread_safe_queue<std::function<void()> > worker_tasks;

    // joiner object guarantees joining of every thread
    joiner_threads joiner;

    void worker_thread() {
        while(!done){
            std::function<void()> task;
            if(worker_tasks.pop(task)){
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

public:

    // the constructor is responsible to spawn the threads in the thread-pool
    thread_pool();

    ~thread_pool();

    template<typename Func>
    void submit_tasks(Func& func);

};