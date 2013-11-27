#pragma once
#include <map>

template <class Type>
class ManagedNode
{
protected:
	static const int MAX_ID_NUM = 10000000;
	
	explicit ManagedNode(Type *i) 
		: id(-1), node_(i)
	{
		node_container_.emplace(std::make_pair(create_id(), node_));
	}

	~ManagedNode(void)
	{
		node_container_.erase(id_);
	}

	inline get_id()
	{
		return id_;
	}

	static void clear_id() {
		last_id_ = 0;
	}

	int create_id() 
	{
		int id_ = -1;
		do 
		{
			id_ = last_id_++;
			if(last_id_ > MAX_ID_NUM)
			{
				assert(false && "Exceed Maximum num_id");
			}
		} while(node_container_.find(id_) != node_container_.end());
		return id;
	}

	void set_id(int i) 
	{
		//TODO: implement for future edit support	
	}

	static Type *get_node(int id)
	{
		typename stdMap<int,Type*>::Iterator it = instances_id.find(id);
		if(it == instances_id.end()) return 0;
		return it->data;
	}

	static inline std::map<int, Type>::iterator nodes_begin() 
	{
		return nodeContainer_.begin();
	}

	static inline std::map<int, Type>::iterator nodes_end() 
	{
		return nodeContainer_.end();
	}

	static inline int nodeContainerSize() 
	{
		return nodeContainer_.size();
	}

private:
	int id_;
	Type *node_;

	static int last_id_;
	static std::map<int, Type*> node_container_;

};

template <class Type> int ManagedNode<Type>::last_id_ = 0;
template <class Type> std::map<int,Type*> ManagedNode<Type>::node_container_;
