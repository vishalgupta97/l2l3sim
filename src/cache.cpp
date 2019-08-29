#include "cache.h"
#include "l2l3sim.h"

void CACHE::init(int type, int num_set, int num_way)
{
	this->type = type;
	this->num_set = num_set;
	this->num_way = num_way;
	this->log2_num_set = (int) log2(num_set);

	sets = new list<unsigned long long> [num_set];
	belady_access = new map<int, unsigned long long> [num_set];

	miss = 0;
	hit = 0;
}

bool CACHE::check_hit(unsigned long long address)
{
	unsigned long set = get_set(address);
	return find(sets[set].begin(), sets[set].end(), address) != sets[set].end();
}

int CACHE::get_set(unsigned long long address)
{
	return address & ((1 << log2_num_set) - 1);
}

void CACHE::update_replacement_state(unsigned long long address)
{
	unsigned long set = get_set(address);
	auto it = find(sets[set].begin(), sets[set].end(), address);
	sets[set].erase(it);
	sets[set].push_front(address);

	assert(sets[set].size() <= num_way);
}

/*int CACHE::get_eviction_way(int set)
{
	for(unsigned int i = 0; i< num_way; i++)//Common for belady and LRU
		if (!block[set][i].valid)
                        return i;
if( type == LLC ){
#ifdef belady_optimal
	int access_index[num_way];
        for (unsigned int i = 0; i < num_way; i++)
		access_index[i] = INT_MAX;
        for (unsigned int i = 0; i < num_way; i++){
		unsigned long long dummy_address = block[set][i].addr;
		auto itr = addr_index.find(dummy_address);
		for (auto it = itr->second.begin();it != itr->second.end(); ++it){
			if(*it >= curr_memory_access){
				access_index[i] = *it;
				break;
			}
		}
	}

	int max = 0,return_way;
        for (unsigned int i = 0; i < num_way; i++){
		if(access_index[i] > max){
			max = access_index[i];
			return_way = i;
		}
	}
	return return_way;

#endif
}

	int lru_index = 0;
	for (unsigned int i = 0; i < num_way; i++)
	{

		if (block[set][i].lru == num_way - 1)
			lru_index = i;
	}

	assert(block[set][lru_index].lru == num_way - 1);

	return lru_index;
 }*/

void CACHE::add_data(unsigned long long address)
{
	unsigned long set = get_set(address);
	long long evicted_addr = -1;

	//New address is added at MRU position (Front of list) in case of LRU
	if (sets[set].size() < num_way)
	{
#if defined(belady_optimal)
		sets[set].push_back(address);
#else
		sets[set].push_front(address);
#endif
	}
	else
	{
		if (type == L2C)
		{
			auto it = sets[set].end();
			it--;
			evicted_addr = *it;
			sets[set].erase(it);
			sets[set].push_front(address);
		}
		else
		{
#if defined(belady_optimal)
			unsigned long long evict_addr;
			int access_index, max_access_index = 0;
			for(auto itr=sets[set].begin();itr!=sets[set].end();itr++)
			{
				auto itr1 = addr_index.find(*itr);
				access_index = -1;
				for (auto it = itr1->second.begin(); it != itr1->second.end(); ++it)
				{
					if(*it >= curr_memory_access)
					{
						access_index= *it;
						if (it != itr1->second.begin())
						{
							it--;
							itr1->second.erase(itr1->second.begin(), it);
						}
						break;
					}
				}
				if(access_index == -1)
				{
					evict_addr = *itr;
					break;
				}
				if (access_index > max_access_index)
				{
					max_access_index = access_index;
					evict_addr = *itr;
				}
			}
			evicted_addr = evict_addr;
			sets[set].erase(find(sets[set].begin(), sets[set].end(), evict_addr));
			sets[set].push_back(address);
#else
			auto it = sets[set].end();
			it--;
			evicted_addr = *it;
			sets[set].erase(it);
			sets[set].push_front(address);
#endif
		}
	}

	assert(sets[set].size() <= num_way);

#ifdef INCLUSIVE
	if (type == LLC && evicted_addr != -1)
	{
		l2.invalidate_data(evicted_addr);
		back_invalidation++;
	}
#endif

#ifdef EXCLUSIVE
	if (type == L2C && evicted_addr != -1)
	{
		l3.add_data(evicted_addr);
	}
#endif

}

void CACHE::invalidate_data(unsigned long long address)
{
	int set = get_set(address);
	auto it = find(sets[set].begin(), sets[set].end(), address);

	if (it != sets[set].end())
		sets[set].erase(it);

	assert(sets[set].size() <= num_way);
}
