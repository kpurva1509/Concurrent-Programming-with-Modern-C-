#include<iostream>
#include<mutex>
#include<thread>
#include<condition_variable>
#include<future>
#include<algorithm>
#include<functional>
#include<numeric>
#include<chrono>
#include<vector>
#include<typeinfo>
#include<time.h>
#include<assert.h>

#include<atomic>
#include<mutex>

using std::cout;
using std::endl;

std::atomic<bool> x_atomic, y_atomic;
std::atomic<int> z_atomic;

class spinlock;

void write_x_atomic(){
	x_atomic.store(true, std::memory_order_seq_cst);
}

void write_y_atomic(){
	y_atomic.store(true, std::memory_order_seq_cst);
}

void read_x_then_y(){
	// loop till x_atomic is true
	while(!x_atomic.load(std::memory_order_seq_cst));

	// read when y_atomic is true
	if(y_atomic.load(std::memory_order_seq_cst))
		z_atomic++;
}

void read_y_then_x(){
	// loop till y_atomic is true
	while(!y_atomic.load(std::memory_order_seq_cst));

	// read when x_atomic is true
	if(x_atomic.load(std::memory_order_seq_cst))
		z_atomic++;
}

void write_x_then_y(){
	x_atomic.store(true, std::memory_order_relaxed);
	y_atomic.store(true, std::memory_order_relaxed);
}

void read_y_then_x_relaxed(){
	while(!y_atomic.load(std::memory_order_relaxed));
	if(x_atomic.load(std::memory_order_relaxed))
		z_atomic++;
}

void write_x_then_y_acquire_release(){
	x_atomic.store(true, std::memory_order_relaxed);
	y_atomic.store(true, std::memory_order_release);
}

void read_y_then_x_acquire_release(){
	while(!y_atomic.load(std::memory_order_acquire));
	if(x_atomic.load(std::memory_order_relaxed))
		z_atomic++;
}

class spinlock{
	std::atomic_flag flag = ATOMIC_FLAG_INIT; // default false

public:
	spinlock() {}

	void lock(){
		while(flag.test_and_set(std::memory_order_acquire));
	}

	void unlock() {
		flag.clear(std::memory_order_release);
	}
};

spinlock spinlock;

void func() {
	std::lock_guard<spinlock> lg(spinlock);
	std::cout << std::this_thread::get_id() << endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

int main() {

	// std::atomic_flag flag2 = true;
	std::atomic_flag flag2 = ATOMIC_FLAG_INIT;

	// set the value of the atomic flag to true and return the previous value
	cout << "1. Previous value of flag_2 is " << flag2.test_and_set() << endl;
	cout << "2. Previous value of flag_2 is " << flag2.test_and_set() << endl;

	flag2.clear();
	cout << "3. Previous value of flag_2 is " << flag2.test_and_set() << endl;

	// cout << typeid(flag2).name() << endl;

	// atomic<bool> OR atomic_bool

	std::atomic<bool> flag_3; // default initialization to true
	cout << "Value of flag3 is " << flag_3 << endl;

	// Cannot copy construct
	// std::atomic<bool> flag4(flag3);

	// Cannot copy assign
	// std::atomic<bool> flag4 = flag3;

	// Can copy assign from non-atomic values
	bool non_atomic_bool = true;
	std::atomic<bool> flag_4{non_atomic_bool};

	cout << "Value of flag4 is " << flag_4 << endl;

	cout << endl;

	// Functions offered by atomic<bool>
	// 1. is_lock_free()
	// 2. store()
	// 3. load()
	// 4. exchange()
	// 5. compare_exchange_weak
	// 6. compare_exchange_strong

	// is_lock_free operations
	std::atomic<bool> flag{false};
	cout << "Atomic bool is implemented lock-free: " <<
			(flag.is_lock_free() ? "yes" : "no") << endl;

	std::atomic_bool y{true};

	// store operations
	flag.store(true);
	flag.store(y);

	// load operations
	cout << "Loading the value of flag: " << flag.load() << endl;
	cout << "Loading the value of y: " << y.load() << endl;

	// exchange operations - stores the newly provided value into the variable and returns the previous value
	bool z = flag.exchange(false);

	cout << "Previous value of flag was " << z << endl;
	cout << "New value of flag is " << flag.load() << endl;

	cout << endl;

	// compare_exchange_weak
	// bool result = actual.compare_exchange_weak(T& expected, T desired);
	// compares actual and expected
	// if actual == expected, update actual = desired
	// if actual != expected, update expected = actual
	// returns true if store is successful, else returns false

	std::atomic<int> actual{20};
	int expected{20};
	int desired{6};

	cout << "Previous actual value: " << actual.load() << endl;
	cout << "Previous expected value " << expected << endl;

	bool result = actual.compare_exchange_weak(expected,desired);

	cout << "compare_exchange_weak successful: "
			<< (result? "yes" : "no") << endl;

	cout << "Current actual value: " << actual.load() << endl;
	cout << "Current expected value " << expected << endl;

	cout << endl;

	// comapre_exchange_strong
	// guarantees the operations
	// differs from machine to machine
	// may use more instructions

	// Atomic pointers - atomic<T*>
	// pointed object is not atomic, the pointer is atomic
	// operation on this pointer is atomic, not the object it points to

	std::vector<int> vec(20);
	static int i = 1;
	for(auto& item:vec)
		item = i++;

	std::atomic<int*> atomic_ptr = &vec[0];
	cout << "Atomic pointer is implemented lock-free: " <<
		(atomic_ptr.is_lock_free() ? "yes" : "no") << endl;
	cout << "Value held by atomic_ptr is " << *(atomic_ptr.load()) << endl;

	int* non_atomic_ptr = &vec[3];
	atomic_ptr.store(non_atomic_ptr);
	cout << "Value held by atomic_ptr is " << *(atomic_ptr.load()) << endl;

	bool ret = atomic_ptr.compare_exchange_weak(non_atomic_ptr, &vec[10]);

	cout << "compare_exchange_weak successful: "
		<< (ret ? "yes" : "no") << endl;

	cout << "Current atomic_ptr value: " << *(atomic_ptr.load()) << endl;
	cout << "Current non_atomic_ptr value " << *non_atomic_ptr << endl;

	cout << endl;

	// Additional operations on atomic pointers
	// 1. fetch_add +=
	// 2. fetch_sub -=
	// 3. ++ increment
	// 4. -- decrement

	// fetch_add
	std::atomic<int*> x_pointer = &vec[0];
	cout << "After initialization, value pointed to by x_pointer is " << *x_pointer << endl;

	int* x_pointer_prev = x_pointer.fetch_add(12);
	cout << "Previous value pointed to by x_pointer after fetch_add is " << *x_pointer_prev << endl;
	cout << "New value pointed to by x_pointer after fetch_add is " << *x_pointer << endl;

	// fetch_sub
	int* x_pointer_prev2 = x_pointer.fetch_sub(10);
	cout << "Previous value pointed to by x_pointer after fetch_sub is " << *x_pointer_prev2 << endl;
	cout << "New value pointed to by x_pointer after fetch_sub is " << *x_pointer << endl;

	// ++ increment
	x_pointer++;
	cout << "New value pointed to by x_pointer after ++ is " << *x_pointer << endl;

	// ++ increment
	x_pointer--;
	cout << "New value pointed to by x_pointer after -- is " << *x_pointer << endl;

	cout << endl;

	// Memory ordering operations
	// 1. memory_order_seq_cst (default)
	// 2. memory_order_relaxed (OoO execution)
	// 3. memory_order_acquire (Load)
	// 4. memory_order_release (Store)
	// 5. memory_order_consume (Special ACQUIRE)
	// 6. memory_order_acq_rel (RMW - exchange, comp_exchange ops)

	std::atomic<int> x;
	x.store(5);

	x.store(10, std::memory_order_release);
	x.load(std::memory_order_acquire);

	int value = 11;
	bool exch = x.compare_exchange_weak(value, 13,
			std::memory_order_relaxed, std::memory_order_release); // 2 options, success scenario, and failure scenario
	cout << exch << endl;

	x_atomic = false;
	y_atomic = false;

	std::thread thread_x(write_x_atomic);
	std::thread thread_y(write_y_atomic);
	std::thread thread_xy(read_x_then_y);
	std::thread thread_yx(read_y_then_x);

	thread_x.join();
	thread_y.join();
	thread_xy.join();
	thread_yx.join();

	assert(z_atomic != 0);

	cout << "Value of z_atomic after operations " << z_atomic.load() << endl;

	// resetting the flags to false

	x_atomic.store(false);
	y_atomic.store(false);

	cout << endl;

	std::thread thread_write_x_then_y(write_x_then_y);
	std::thread thread_read_y_then_x_relaxed(read_y_then_x_relaxed);

	thread_write_x_then_y.join();
	thread_read_y_then_x_relaxed.join();

	// cout << "Value of z_atomic after operations " << z_atomic.load() << endl;

	// assert(z_atomic > 2);

	cout << "Value of z_atomic after operations " << z_atomic.load() << endl;

	// reset flags to false

	x_atomic.store(false);
	y_atomic.store(false);

	cout << endl;

	std::thread thread_write_x_then_y_acquire_release(write_x_then_y_acquire_release);
	std::thread thread_read_y_then_x_acquire_release(read_y_then_x_acquire_release);

	thread_write_x_then_y_acquire_release.join();
	thread_read_y_then_x_acquire_release.join();

	// assert(z_atomic > 1);

	cout << "Value of z_atomic after operations " << z_atomic.load() << endl;

	// reset flags to false

	x_atomic.store(false);
	y_atomic.store(false);

	cout << endl;

	// Spinlock implementation MUTEX
	// multicore or multiprocessor scenarios

	return 0;
}
