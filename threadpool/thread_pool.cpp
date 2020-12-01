#include "thread_pool.hpp"

// function implementations

/*
    members -
    std::atomic<bool> done;
    std::vector<std::thread> worker_threads;
    std::vector<std::function<void()> > worker_tasks;
    joiner_threads joiner;

    functions - 
    thread_pool();
    ~thread_pool();
    template<typename Func>
    void submit_tasks(Func& func);

*/

thread_pool::thread_pool(): done(false), joiner(worker_threads){

    // create a thread-pool based on hardware concurrency
    const int max_threads = std::thread::hardware_concurrency();

    try{
        for(int i=0; i<max_threads; i++){
        worker_threads.push_back(std::thread(&thread_pool::worker_thread, this));
        }
    }catch(...){
        done = true;
        throw;
    }
}

thread_pool::~thread_pool(){
    done = true;
}

template<typename Func>
void thread_pool::submit_tasks(Func& func){
    worker_tasks.push(std::function<void()>(func));
}