/* Lock free Data Structures and Algorithms */

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

template<typename T>
class lock_free_stack{

	struct node {
		std::shared_ptr<T> data;
		node* next;

		node(const T& data): data(std::make_shared<T>(data)){}
	};

	std::atomic<node*> head;

	// used for deleting nodes in multithreaded scenario
	std::atomic<int> threads_in_pop;
	std::atomic<node*> to_be_deleted;

	void delete_nodes(node* nodes){
		while(nodes){
			node* next = nodes->next;
			delete nodes;
			nodes = next;
		}
	}

	void try_reclaim(node* old_head){

		if(threads_in_pop == 1){

			// delete the only head remaining
			delete old_head;

			// reclaim the to_be_deleted list as it empties out after this step
			// update to_be_deleted = nullptr;
			// claim_list_back = old value of to_be_deleted
			node* claim_list_back = to_be_deleted.exchange(nullptr);

			if(!--threads_in_pop){

				delete_nodes(claim_list_back);

			}

			else if(claim_list_back){

				node* last = claim_list_back;
				while(last && last-> next){
					last = last->next;

				}

				// Update the claimed list with newly added elements to to_be_deleted
				// newly added to_be_deleted elements appended to the end of the claim_List_back list
				last->next = to_be_deleted;

				// now, update head pointer of to_be_deleted list to head of claim_list_back
				while(!to_be_deleted.compare_exchage_weak(last->next, claim_list_back));
			}
		}

		// if threads_in_pop > 1
		else {

			// add node pointed by old head to to_be_deleted list
			// the old_head becomes the top of the to_be_deleted list
			// therefore, we are basically making a "stack" of the nodes to be deleted
			// the very first node to be deleted is the last member of this list
			// the latest member is added to the top of the list
			old_head->next = to_be_deleted;
			while(!to_be_deleted.compare_exchange_weak(old_head->next, old_head));
			threads_in_pop--;

		}
	}

public:

	void push(const T& data){

		node* const new_node = new node(data);

		// atomic operations
		new_node->next = head.load();
		while(!head.compare_exchange_weak(new_node->next, new_node));
	}

	// pop using thread counting
	std::shared_ptr<T> pop(T& result){

		++threads_in_pop;
		node* old_head = head.load();
	 	while(old_head && !head.compare_exchange_weak(old_head,old_head->next));

		std::shared_ptr<T> res;

		if(old_head){
			res.swap(old_head->data);
		}

		try_reclaim(old_head);

		//return old_head ? old_head->data : std::shared_ptr<T>();

		// delete old_head;
		// memory reclamation needs to be smart

		// 1. Memory reclaim using thread counting

		return res;

	}

};

template<typename T>
class lock_free_stack_hazard{

	struct node{

		T data;
		node* next;

		// constructor for node
		node(const T& data): data(data){}
	};

	struct hazard_pointer{

		// contains thread ID
		// contains node pointer
		std::atomic<std::thread::id> id;
		std::atomic<void*> pointer;
	};

	std::atomic<node*> head;
	static const int max_hazard_pointers = 100;
	hazard_pointer hazard_pointers[max_hazard_pointers];
	std::atomic<node*> nodes_to_reclaim;

	// class for managing hazard_pointers to the threads

	class hp_manager{

		hazard_pointer* hp;

	public:

		hp_manager():hp(nullptr){
			for(unsigned int i = 0; i < max_hazard_pointers; i++){
				std::thread::id old_id;
				if(hazard_pointers[i].id.compare_exchange_strong(old_id,std::this_thread::get_id())){
					hp = &hazard_pointers[i];
					break;
				}
			}

			if(!hp){
				throw std::runtime_error("No hazard pointers available");
			}
		}

		std::atomic<void*>& get_pointer(){
			return hp->pointer;
		}

		~hp_manager(){
			hp->pointer.store(nullptr);
			hp->id.store(std::this_thread::get_id());
		}

	};

	std::atomic<void*> get_hp_for_current_thread(){

		// we want one object per thread
		// we can't use just static, as it will create one object for all threads
		// therefore we have to use thread_local as well

		static thread_local hp_manager hp_owner;
		return hp_owner.get_pointer();

	}

	bool any_outstanding_hazards(node* p){
		for(unsigned int i = 0; i < max_hazard_pointers; i++){
			if(hazard_pointers[i].pointer.load() == p){
				return true;
			}
			return false;
		}
	}

	void reclaim_later(node* node){
		node->next = nodes_to_reclaim.load();
		while(!nodes_to_reclaim.compare_exchange_strong(node->next, node));
	}

	void delete_nodes_with_no_hazard() {

		node* current = nodes_to_reclaim.exchange(nullptr);

		while(current){

			node* const next = current->next;
			if(!any_outstanding_hazards(current)){
				delete current;
			} else {
				reclaim_later(current);
			}

			current = next;

		}

	}

public:

	lock_free_stack_hazard(){}

	void push(const T& data){

		node* const new_node = new node(data);

		// atomic operations
		new_node->next = head.load();
		while(!head.compare_exchange_weak(new_node->next, new_node));
	}

	void pop(T& result) {

		// we firstly claim the hazard_pointer for this thread
		std::atomic<void*> &hp = get_hp_for_current_thread();
		node* old_head = head.load();

		do{

			// setting old_head as the hazard_pointer
			hp.store(old_head);

		}while(old_head && !head.compare_exchange_weak(old_head,old_head->next));

		// cannot just delete the node
		// it could be referenced by other thread which may try to access the contents at the location
		// therefore, we make an entry with the node to be deleted, and the thread trying to delete the node
		// this is in the hazard pointer table (LUT)
		// delete old_head;

		hp.store(nullptr);

		std::shared_ptr<T> res;

		if(old_head){

			res.swap(old_head->data);

			if(any_outstanding_hazards(old_head)){

				// add the node to the to_be_deleted list
				reclaim_later(old_head);

			} else {

				delete old_head;

			}
		}
	}
};

int main() {




	return 0;
}
