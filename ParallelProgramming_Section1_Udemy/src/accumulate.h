#include<iostream>
#include<vector>
#include<thread>
#include<chrono>
#include<memory>
#include<algorithm>
#include<functional>
#include<time.h>
#include<numeric>

#pragma once

#define MIN_BLOCK_SIZE 1000

using std::cout;
using std::endl;

template<typename iterator, typename T>
void accumulate(iterator begin, iterator end, T& ref){
	ref = std::accumulate(begin, end, 0);
}

template<typename iterator, typename T>
T parallel_accumulate(iterator begin, iterator end, T& ref){

	int input_size = std::distance(begin, end);
	int allowed_threads_by_elements = (input_size)/MIN_BLOCK_SIZE;
	int allowed_threads_by_hardware = std::thread::hardware_concurrency();
	int num_threads = std::min(allowed_threads_by_elements, allowed_threads_by_hardware);

	int block_size = (input_size+1)/num_threads; // round off block size to higher value

	// we will have as many results as the number of threads
	// T is the datatype of the values we are accumulating
	// we then have to accumulate the results of this vector for the final round
	// std::accumulate(results.begin(), results,end(), 0);
	std::vector<T> results(num_threads);

	// we have a vector of threads spawned
	std::vector<std::thread> threads(num_threads-1);

	// for loop for launching threads

	iterator last;

	for(int i=0; i < num_threads-1; i++){
		last = begin;
		std::advance(last, block_size);
		threads[i] = std::thread(accumulate<iterator,T>, begin, last, std::ref(results[i]));
		begin = last;
	}

	results[num_threads - 1] = std::accumulate(begin, end, 0);

	/*std::for_each(threads.begin(), threads.end(),
			std::mem_fn(&std::thread::join())); */

	for(int i=0; i<num_threads-1;i++){
		threads[i].join();
	}

	return std::accumulate(results.begin(), results.end(), ref);

}
