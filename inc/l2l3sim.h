#include<bits/stdc++.h>
#include "config.h"
using namespace std;

#define MAX_TRACE_SIZE 15400000

#define BLOCK_SIZE 64
#define LOG2_BLOCK_SIZE 6

#define L2_SET 1024
#define L2_WAY 8
#define L3_SET 1 // 2048
#define L3_WAY 32768 //16

#define belady_optimal // run belady's optimal algorithm on LLC
		       // comment this to run LRU on LLC

class MEMORY_ACCESS
{
public:
	char iord; //Instruction or data miss
	char type; // Zero means cache hit and non-zero cache miss
	unsigned long long addr; // Miss address without block offset
	unsigned pc; // Program counter of load/store instruction that missed
	MEMORY_ACCESS()
	{
		iord = type = addr = pc = 0;
	}
};

extern MEMORY_ACCESS* access_trace;
extern int num_memory_access, curr_memory_access;
extern unordered_map<unsigned long long, set<int> > addr_index;


