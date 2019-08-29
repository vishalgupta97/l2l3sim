#include<bits/stdc++.h>
using namespace std;

#define L2C 1
#define LLC 2

class CACHE
{
public:
	list<unsigned long long> *sets;

	unsigned int num_set, num_way, miss, hit, log2_num_set, type, back_invalidation;

	void init(int type, int num_set, int num_way);
	bool check_hit(unsigned long long address);
	void update_replacement_state(unsigned long long address);
	void add_data(unsigned long long address);
	int get_set(unsigned long long address);
	void invalidate_data(unsigned long long address);
};

extern CACHE l2, l3;
