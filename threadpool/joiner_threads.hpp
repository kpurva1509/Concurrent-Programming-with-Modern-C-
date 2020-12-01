#include "headers.hpp"
#include "thread_safe_queue.hpp"

class joiner_threads{

private:
    std::vector<std::thread> threads;

public:
    // do not capture threwds by const reference, or you will not be able to join them
    joiner_threads(std::vector<std::thread>& threads){
        for(auto& thread : threads){
            thread.join();
        }
    }
};