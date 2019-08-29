#include "l2l3sim.h"
#include "cache.h"

MEMORY_ACCESS* access_trace;
int num_memory_access, curr_memory_access;
unordered_map<unsigned long long, set<int> > addr_index;
CACHE l2, l3;

void initialize()
{
	access_trace = new MEMORY_ACCESS[MAX_TRACE_SIZE];
	num_memory_access = 0;

	l2.init(L2C, L2_SET, L2_WAY);
	l3.init(LLC, L3_SET, L3_WAY);
}

void check_inclusive()
{
	for (unsigned int l2set = 0; l2set < l2.num_set; l2set++)
	{
		if (l2.sets[l2set].size() > l2.num_way)
		{
			cout << l2set << " " << l2.sets[l2set].size() << endl;
			assert(l2.sets[l2set].size() <= l2.num_way);
		}
		for (auto l2addr_it = l2.sets[l2set].begin(); l2addr_it != l2.sets[l2set].end(); l2addr_it++)
		{
			int l3set = l3.get_set(*l2addr_it);

			assert(find(l3.sets[l3set].begin(), l3.sets[l3set].end(), *l2addr_it) != l3.sets[l3set].end());
		}
	}
}

void check_exclusive()
{
	for (unsigned int l2set = 0; l2set < l2.num_set; l2set++)
		{
		for (auto l2addr_it = l2.sets[l2set].begin(); l2addr_it != l2.sets[l2set].end(); l2addr_it++)
			{
			int l3set = l3.get_set(*l2addr_it);

			assert(find(l3.sets[l3set].begin(), l3.sets[l3set].end(), *l2addr_it) == l3.sets[l3set].end());
			}
		}
}

void operate()
{
	set <int> addr_set;

#ifdef belady_optimal
	for (curr_memory_access = 0; curr_memory_access < num_memory_access; curr_memory_access++)
	{
		unsigned long long address = access_trace[curr_memory_access].addr;
		auto itr = addr_index.find(address);
		if(itr == addr_index.end()){
			addr_set.clear();
			addr_set.insert(curr_memory_access);
			addr_index.insert(make_pair(address,addr_set));
		}
		else
			itr->second.insert(curr_memory_access);

	}

	unordered_map<unsigned long long, set<int> >::iterator itr;
	int sum = 0;
	for (itr = addr_index.begin(); itr != addr_index.end(); ++itr) {
        	sum += itr->second.size();
    }
    assert(sum==num_memory_access);

#endif

	for (curr_memory_access = 0; curr_memory_access < num_memory_access; curr_memory_access++)
	{
		unsigned long long address = access_trace[curr_memory_access].addr;
		if (access_trace[curr_memory_access].type != 0) //L1 miss
		{

			if (l2.check_hit(address)) // L2 hit
			{
				l2.hit++;
				l2.update_replacement_state(address);
			}
			else // L2 miss
			{
				l2.miss++;
				if (l3.check_hit(address))
				{
					l3.hit++;
#ifndef belady_optimal
					l3.update_replacement_state(address);
#endif
					l2.add_data(address);

#ifdef EXCLUSIVE
					l3.invalidate_data(address);
#endif
				}
				else
				{
					l3.miss++;

#if defined(INCLUSIVE) || defined(NONINCLUSIVE)
					l3.add_data(address);
					l2.add_data(address);
#endif

#ifdef EXCLUSIVE
					l2.add_data(address);
#endif
				}
			}

			assert(l2.sets[l2.get_set(address)].size() <= l2.num_way);
			assert(l3.sets[l3.get_set(address)].size() <= l3.num_way);
		}

		if (curr_memory_access % 100000 == 0)
		{
#ifdef INCLUSIVE
			check_inclusive();
#endif
#ifdef EXCLUSIVE
			check_exclusive();
#endif
			cout << "Instruction Completed: " << curr_memory_access << endl;
		}
	}
}

int main(int argc, char** argv)
{

#ifdef INCLUSIVE
	cout << "L2 INCLUSIVE of L3" << endl;
#endif
#ifdef NONINCLUSIVE
	cout << "L2 NONINCLUSIVE of L3" << endl;
#endif
#ifdef EXCLUSIVE
	cout << "L2 EXCLUSIVE of L3" << endl;
#endif
#ifdef belady_optimal
	cout<<"Belady at L3"<<endl;
#else
	cout<<"LRU at L3"<<endl;
#endif


	initialize();

	int numtraces = atoi(argv[2]);

	set<unsigned long long> unique_addr;

	for (int i = 0; i < numtraces; i++)
	{
		char* input_name = (char*) malloc(100 * sizeof(char));
		sprintf(input_name, "%s_%d", argv[1], i);
		FILE* fp = fopen(input_name, "rb");
		assert(fp != NULL);

		while (!feof(fp))
		{
			fread(&(access_trace[num_memory_access].iord), sizeof(char), 1, fp);
			fread(&(access_trace[num_memory_access].type), sizeof(char), 1, fp);
			fread(&(access_trace[num_memory_access].addr), sizeof(unsigned long long), 1, fp);
			fread(&(access_trace[num_memory_access].pc), sizeof(unsigned), 1, fp);

			access_trace[num_memory_access].addr >>= LOG2_BLOCK_SIZE;

#ifdef belady_optimal
			unique_addr.insert(access_trace[num_memory_access].addr);
#endif

			num_memory_access++;
		}
	}

	operate();

	cout << "Total L2 accesses: " << num_memory_access << endl;
	cout << "Total unique accesses / Cold misses: " << unique_addr.size() << " percent: " << ((unique_addr.size() * 100.0) / num_memory_access) << endl;
	cout << "L2 hits: " << l2.hit << endl;
	cout << "L2 miss: " << l2.miss << endl;
	cout << "L3 hits: " << l3.hit << endl;
	cout << "L3 miss: " << l3.miss << endl;
	cout << "L3 back-invalidation requests: " << l3.back_invalidation << endl;
}
