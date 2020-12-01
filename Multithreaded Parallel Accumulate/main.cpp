#include "headers.hpp"

/* wrapper around std::accumulate to take reference */
template<typename iterator, typename T>
void accumulate(iterator start, iterator end, T& data_store){
  data_store = std::accumulate(start, end, 0);
}

template<typename iterator, typename T>
T parallel_accumulate(iterator start, iterator end, T& init){
  auto input_size = std::distance(start, end);
  int num_threads = std::min((input_size/MIN_BLOCK_SIZE), 
                              std::thread::hardware_concurrency());

  /* (input_size+1) to compensate for the roundmin() op */
  int data_block_size = (input_size+1)/num_threads;

  /* accumulate the results of each thread */
  vector<T> results(num_threads);
  /* num_threads-1 as one of the threads will be main() */
  vector<thread> threads(num_threads-1); 

  iterator last;

  /* spawning threads with data-block ranges */ 
  for(int i=0; i<num_threads-1; i++){
    last = start;
    std::advance(last, data_block_size);
    /* 
    * We can spawn new thread with std::accumulate
    * std::accumulate does has a return value
    * but how to transfer return value from one thread to other
    * we can pass a reference of a value from one thread to other
    * the change made to a value by second thread is visible
    * to the calling thread as well
    */

    threads[i] = thread(accumulate<iterator, T>, 
                        start, last, std::ref(results[i]));

    start = last;
  }
  results[num_threads-1] = std::accumulate(start, end, 0);

  /* mem_fn to store and execute a member function and a member object */
  std::for_each(threads.begin(), threads.end(),
                std::mem_fn(&std::thread::join));

  return std::accumulate(results.begin(), results.end(), init);

}

template<typename iterator, typename T>
T parallel_accumulate_async(iterator start, iterator end) {
  auto input_size = std::distance(start, end);
  if(input_size < MIN_BLOCK_SIZE) return std::accumulate(start, end, 0);

  iterator mid = start;
  std::advance(mid, (input_size+1)/2);

  /* recursive call to parallel_accumulate_async */
  /* compiler optimization for task launching */
  std::future<T> f1 = std::async(std::launch::deferred | std::launch::async,
                                parallel_accumulate_async<iterator, T>(mid, end));
  auto sum = parallel_accumulate_async(start, mid);
  return sum+f1.get();
}

int main() {
  
  return 0;
}
