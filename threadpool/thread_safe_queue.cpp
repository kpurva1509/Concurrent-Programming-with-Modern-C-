#include "thread_safe_queue.hpp"

// function implementations

/*
    bool pop(T&);
    std::shared_ptr<T> pop();
	void wait_pop(T&);
	std::shared_ptr<T> wait_pop();
	bool empty();
*/

// default instantiation of a thread-safe queue
template<typename T>
thread_safe_queue<T>::thread_safe_queue(){}

// insert a new task into the thread-pool
template<typename T>
void thread_safe_queue<T>::push(T& new_task){
    std::lock_guard<std::mutex> lg(m);
    queue.push(std::make_shared<T>(new_task));
    cv.notify_all();
}

// to return if pop was successful or not
template<typename T>
bool thread_safe_queue<T>::pop(T& task_ref){
    std::lock_guard<std::mutex> lg(m);
    if(queue.empty()){
        return false;
    } else {
        task_ref = queue.front();
        queue.pop();
        return true;
    }
}

// return a shared pointer to the popped task
template<typename T>
std::shared_ptr<T> thread_safe_queue<T>::pop(){
    std::lock_guard<std::mutex> lg(m);
    if(queue.empty()){
        return std::shared_ptr<T>();
    } else {
        std::shared_ptr<T> ref(queue.front());
        queue.pop();
        return ref;
    }
}

template<typename T>
void thread_safe_queue<T>::wait_pop(T& task){
    std::unique_lock<std::mutex> ul(m);
    cv.wait(ul, [this](){
        return !queue.empty();
    });
    queue.pop();
}

template<typename T>
std::shared_ptr<T> thread_safe_queue<T>::wait_pop(){
    std::unique_lock<std::mutex> ul(m);
    cv.wait(ul, [this](){
        return !queue.empty();
    });
    std::shared_ptr<T> ref = queue.front();
    queue.pop();
    return ref;
}

template<typename T>
bool thread_safe_queue<T>::empty() {
    std::lock_guard<std::mutex> lg(m);
    return queue.empty();
}
