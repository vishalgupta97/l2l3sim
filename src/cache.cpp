#include "cache.h"
#include "l2l3sim.h"

void CACHE::init(int type, int num_set, int num_way)
{
	this->type = type;
	this->num_set = num_set;
	this->num_way = num_way;
	this->log2_num_set = (int) log2(num_set);

	block = new BLOCK*[num_set];
	for (int i = 0; i < num_set; i++)
	{
		block[i] = new BLOCK[num_way];
		for (int j = 0; j < num_way; j++)
		{
			block[i][j].lru = j;
			block[i][j].valid = false;
		}
	}

	miss = 0;
	hit = 0;
}

int CACHE::check_hit(unsigned long long address)
{
	unsigned long set = get_set(address);
	unsigned long tag = address >> log2_num_set;

	for (int i = 0; i < num_way; i++)
		if (block[set][i].valid && block[set][i].tag == tag)
		{
			return i;
		}

	return -1;
}

int CACHE::get_set(unsigned long long address)
{
	return address && ((1 << log2_num_set) - 1);
}

void CACHE::update_replacement_state(int set, int way)
{
	for (int i = 0; i < num_way; i++)
		if (block[set][i].lru < block[set][way].lru)
			block[set][i].lru++;
	block[set][way].lru = 0;
}

int CACHE::get_eviction_way(int set)
{
	int lru_index = 0;
	for (int i = 0; i < num_way; i++)
	{
		if (!block[set][i].valid)
			return i;

		if (block[set][i].lru == num_way - 1)
			lru_index = i;
	}

	return lru_index;
}

void CACHE::add_data(unsigned long long address)
{
	unsigned long set = address && ((1 << log2_num_set) - 1);
	unsigned long tag = address >> log2_num_set;

	unsigned long way = get_eviction_way(set);

#ifdef INCLUSIVE
	if (type == LLC && block[set][way].valid)
	{
		l2.invalidate_data(block[set][way].addr);
		back_invalidation++;
	}
#endif

#ifdef EXCLUSIVE
	if(type == L2C && block[set][way].valid)
	{
		l3.add_data(block[set][way].addr);
	}
#endif

	update_replacement_state(set, way);
	block[set][way].tag = tag;
	block[set][way].addr = address;
	block[set][way].valid = true;
}

void CACHE::invalidate_data(unsigned long long address)
{
	int set = get_set(address);
	unsigned long tag = address >> log2_num_set;
	for (int i = 0; i < num_way; i++)
		if (block[set][i].valid && block[set][i].tag == tag)
			block[set][i].valid = false;
}
