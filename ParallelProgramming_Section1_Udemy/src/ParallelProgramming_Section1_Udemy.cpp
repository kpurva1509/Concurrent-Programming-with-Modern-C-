#include<iostream>
#include<vector>
#include<thread>
#include<chrono>
#include<memory>
#include<algorithm>
#include<functional>
#include<time.h>
#include<numeric>

#include "accumulate.h"

#pragma once

using std::cout;
using std::endl;

void test(){
	cout << "test() function running with thread ID" << std::this_thread::get_id() << endl;
}

class Test{
public:
	void operator()(){
		cout << "Hello from class overloaded function call operator(functor) called Test with thread ID" << std::this_thread::get_id() << endl;
	}
};

void functionA(){
	cout << "functionA() running, thread ID: " << std::this_thread::get_id() << endl;
	cout << "Launching a thread called tC(threadC from tA(threadA))" << endl;
	std::thread tC(test);
	tC.join();
}

void functionB(){

	cout << "functionB() running, thread ID: " << std::this_thread::get_id() << endl;
}

void other_operations() {
	cout << "This is the other_operations() function" << endl;
	throw std::runtime_error("This is a runtime error");
}

// RAII mechanism
class thread_guard{

private:

	std::thread& t;

public:

	explicit thread_guard(std::thread& t):t(t){}

	~thread_guard(){
		if(t.joinable()){
			t.join();
		}
	}

	thread_guard(const thread_guard&) = delete;
	const thread_guard operator=(const thread_guard&) = delete;
};


int main() {

	std::thread t1(test);
	t1.join();

	Test test;
	std::thread t2(test);
	t2.join();

	auto lambda = [](){
		cout << "Hello from lambda function with thread ID" << std::this_thread::get_id() << endl;
	};

	std::thread t3(lambda);
	t3.join();

	cout << endl;

	std::thread tA(functionA);
	tA.join();

	// std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	std::thread tB(functionB);
	tB.join();

	// handling join() in exception scenarios
	std::thread t(test);
	thread_guard tg(t);

	try{
		other_operations();
		t.join();
	}catch(std::exception& e){
		// t.join();
	}

	// Trivial sail ship model

	auto clean = [](){
		cout << "\nEntered the clean function"<< endl;
	};

	auto propel = [](){
		cout << "Entered the propel function, have to wait for propel to finish" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	};

	auto stop = [](){
		cout << "Entered the stop function, have to wait for ship to come to complete stop" << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	};

	int option;

	printf("Enter 1 for cleaning the sail ship\n");
	printf("Enter 2 for propelling the ship full speed ahead\n");
	printf("Enter 3 for stopping the ship\n");
	printf("Enter 100 to exit\n");

	// printf("Enter your option: ");
	// scanf("%d", &option);

	do{
		printf("Enter your option: ");
		scanf("%d", &option);
		switch(option){
		case 1:
			{
				printf("Sending out a command to clean the ship\n");
				std::thread tClean(clean);
				tClean.detach();
			}
			break;
		case 2:
			{
				printf("Sending out command to propel ship forward\n");
				std::thread tPropel(propel);
				thread_guard tg(tPropel);
			}
			break;
		case 3:
		{
				printf("Sending out command to stop the ship\n");
				std::thread tStop(stop);
				thread_guard tg2(tStop);
		}
			break;
		case 100:
			printf("Exiting program");
			break;
		default:
			printf("Entered the wrong option, try again!\n");
			break;
		}
	}while(option!=100);

	// transferring ownership of threads from one to other
	// copy constructor and copy assignment operators are deleted
	// move constructors and move assignment are available

	std::thread thread1(test);

	// std::thread thread2 = thread1; // not allowed

	std::thread thread2 = std::move(thread1); // this is valid, now thread_1 is empty

	thread1 = std::thread(test); // rhs is a temporary value so implicit move occurs
	// now threa1is NOT empty

	std::thread thread3(test);
	// thread1 = std::move(thread3); // errors out as thread1 is NOT empty

	thread1.join();
	thread2.join();
	thread3.join();

	// useful functions on thread
	// 1. get_id() - std::this_thread::get_id();
	// 2. sleep_for() - std::this_thread::sleep_for()
	// 3. yield() - std::this_thread::yield()
	// 4. hardware_concurreny() - std::thread::hardware_concurrency

	cout << "Hardware concurrency of the machine: " << std::thread::hardware_concurrency() << endl;

	// Parallel accumulate algorithm - Divide and Conquer



	return 0;
}
