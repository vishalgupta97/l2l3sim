#include<bits/stdc++.h>
using namespace std;

#define L2C 1
#define LLC 2

class BLOCK
{
public:
	unsigned long long addr;
	unsigned long tag;
	unsigned int lru;
	bool valid;

};

class CACHE
{
public:
	BLOCK **block;
	unsigned int num_set, num_way, miss, hit, log2_num_set, type, back_invalidation;

	void init(int type, int num_set, int num_way);
	int check_hit(unsigned long long address);
	void update_replacement_state(int set, int way);
	void add_data(unsigned long long address);
	int get_eviction_way(int set);
	int get_set(unsigned long long address);
	void invalidate_data(unsigned long long address);
};

extern CACHE l2, l3;
