#include<iostream>
#include<vector>
#include<functional>
#include<algorithm>
#include<typeinfo>
#include<chrono>
#include<memory>
#include<time.h>
#include<string>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<future>
#include<numeric>

using std::cout;
using std::endl;

#define MIN_ELEMENT_COUNT 1000

template<typename T>
class thread_safe_queue{

private:
	std::mutex m;
	std::condition_variable cv;
	std::queue<std::shared_ptr<T>> queue;

public:
	thread_safe_queue();
	thread_safe_queue(const thread_safe_queue&) = delete;
	const thread_safe_queue operator=(const thread_safe_queue&) = delete;

	void push(T&);
	bool pop(T&);
	std::shared_ptr<T> pop();

	void wait_pop(T&);
	std::shared_ptr<T> wait_pop();

	bool empty();

};

// function implementations
template<typename T>
thread_safe_queue<T>::thread_safe_queue(){

}

/*template<typename T>
thread_safe_queue<T>::thread_safe_queue(const thread_safe_queue& other){

}

template<typename T>
const thread_safe_queue thread_safe_queue<T>::operator=(const thread_safe_queue& other){

}*/

template<typename T>
void thread_safe_queue<T>::push(T& value){
	std::lock_guard<std::mutex> lg(m);
	queue.push(std::make_shared<T>(value));
	cv.notify_all();
}

template<typename T>
bool thread_safe_queue<T>::pop(T& ref){
	std::lock_guard<std::mutex> lg(m);
	if(queue.empty())
		return false;
	else{
		return queue.pop();
	}
}

template<typename T>
std::shared_ptr<T> thread_safe_queue<T>::pop(){
	// returning a reference of the popped value
	std::lock_guard<std::mutex> lg(m);
	if(queue.empty())
		return std::shared_ptr<T>();
	else{
		std::shared_ptr<T> ref(queue.front());
		queue.pop();
		return ref;
	}
}

template<typename T>
void thread_safe_queue<T>::wait_pop(T& value){
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
bool thread_safe_queue<T>::empty(){
	std::lock_guard<std::mutex> lg(m);
	return queue.empty();
}

void functionA(){
	cout << "Running functionA with thread ID: " << std::this_thread::get_id() << endl;
}

int functionB(int a, int b){
	cout << "Running functionB with thread ID: " << std::this_thread::get_id() << endl;
	std::this_thread::sleep_for(std::chrono::seconds(3));
	return a+b;
}

int functionC(int a, int b){
	cout << "Running functionC with thread ID: " << std::this_thread::get_id() << endl;
	return a-b;
}

template<typename iterator>
int parallel_accumulate(iterator begin, iterator end){

	long length = std::distance(begin, end);

	if(length <= MIN_ELEMENT_COUNT){
		cout << std::this_thread::get_id() << endl;
		return std::accumulate(begin, end, 0);
	}

	iterator mid = begin;
	std::advance(mid, (length+1)/2);

	// Recursive call to parallel_accumulate
	std::future<int> f = std::async(std::launch::async|std::launch::deferred, parallel_accumulate<iterator>, mid, end);
	auto sum = parallel_accumulate(begin, mid);
	return sum+f.get();
}

int main() {

	// thread safe queue data structure
	// push
	// pop
	// front
	// empty
	// size

	// Race conditions inherit from the interface are:
	// 1. Empty v/s front
	// 2. Empty v/s back
	// 3. Pop v/s back

	// Futures and Async

	int a = 100;
	int b = 75;

	std::future<void> f1 = std::async(std::launch::async, functionA);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	std::future<int> f2 = std::async(std::launch::deferred, functionB, a,b);
	std::this_thread::sleep_for(std::chrono::seconds(2));

	std::future<int> f3 = std::async(std::launch::async|std::launch::deferred, functionC,a,b);
	std::this_thread::sleep_for(std::chrono::seconds(3));

	cout << "Result of function call to functionB is " << f2.get() << endl;
	cout << "Result of function call to functionC is " << f3.get() << endl;

	// Parallel Accumulate using future and async

	// Package tasks
	// Runs synchronously, same thread ID as launching thread
	// Not copyabble
	// To use move semantics t move task to thread and launch in asynchronously

	// Promises




	return 0;
}
