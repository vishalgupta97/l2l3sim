#include "l2l3sim.h"
#include "cache.h"

MEMORY_ACCESS* access_trace;
int num_memory_access, curr_memory_access;
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
		for (unsigned int l2way = 0; l2way < l2.num_way; l2way++)
		{
			if (l2.block[l2set][l2way].valid)
			{
				int l3set = l3.get_set(l2.block[l2set][l2way].addr);
				unsigned long l3tag = l2.block[l2set][l2way].addr >> l3.log2_num_set;
				int match = -1;

				for (unsigned int l3way = 0; l3way < l3.num_way; l3way++)
					if (l3.block[l3set][l3way].valid && l3.block[l3set][l3way].tag == l3tag)
					{
						match = 1;
						break;
					}
				assert(match == 1);
			}
		}
	}
}

void check_exclusive()
{
	for (unsigned int l2set = 0; l2set < l2.num_set; l2set++)
	{
		for (unsigned int l2way = 0; l2way < l2.num_way; l2way++)
		{
			if (l2.block[l2set][l2way].valid)
			{
				int l3set = l3.get_set(l2.block[l2set][l2way].addr);
				unsigned long l3tag = l2.block[l2set][l2way].addr >> l3.log2_num_set;
				int match = -1;

				for (unsigned int l3way = 0; l3way < l3.num_way; l3way++)
					if (l3.block[l3set][l3way].valid && l3.block[l3set][l3way].tag == l3tag)
					{
						match = 1;
						break;
					}
				assert(match == -1);
			}
		}
	}
}

void operate()
{
	int way = -1;
	for (curr_memory_access = 0; curr_memory_access < num_memory_access; curr_memory_access++)
	{
		unsigned long long address = access_trace[curr_memory_access].addr;
		if (access_trace[curr_memory_access].type != 0) //L1 miss
		{
			way = l2.check_hit(address);
			if (way != -1) // L2 hit
			{
				l2.hit++;
				l2.update_replacement_state(l2.get_set(address), way);
			}
			else // L2 miss
			{
				l2.miss++;
				way = l3.check_hit(address);
				if (way != -1)
				{
					l3.hit++;
					l3.update_replacement_state(l3.get_set(address), way);
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
		}

		if (curr_memory_access % 1000 == 0)
		{
#ifdef INCLUSIVE
			check_inclusive();
#endif
#ifdef EXCLUSIVE
			check_exclusive();
#endif
		}
		if(curr_memory_access % 100000 == 0)
		{
			cout<< curr_memory_access<<endl;
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

			unique_addr.insert(access_trace[num_memory_access].addr);

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
