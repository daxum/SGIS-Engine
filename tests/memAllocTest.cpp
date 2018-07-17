#include <iostream>
#include <vector>

#include "../src/MemoryAllocator.hpp"
#include "../src/ExtraMath.hpp"

const size_t MEM_AMOUNT = 100;
const size_t ALLOC_AMOUNT = 11;
const size_t ALLOC_MIN = 1;
const size_t ALLOC_MAX = 20;
//If true, allocations are always ALLOC_AMOUNT bytes,
//otherwise they're random amounts between ALLOC_MIN and
//ALLOC_MAX.
const bool RANDOM_ALLOC = true;

void printAllocations(const std::vector<std::shared_ptr<AllocInfo>>& allocs) {
	for (std::shared_ptr<AllocInfo> alloc : allocs) {
		std::cout << (alloc->inUse ? "Allocated: " : "Free: ") <<
		alloc->start << ", " <<
		alloc->size << ", " <<
		"Evicted: " << alloc->evicted << "\n";
	}
}

void testAllocator(MemoryAllocator& alloc) {
	std::cout << "Filling memory...\n";
	size_t totalAlloc = 0;
	std::vector<std::shared_ptr<AllocInfo>> allocations;

	//Allocate until memory runs out
	try {
		while (true) {
			size_t amount = 0;

			if (RANDOM_ALLOC) {
				amount = ExMath::randomInt(ALLOC_MIN, ALLOC_MAX);
			}
			else {
				amount = ALLOC_AMOUNT;
			}

			std::cout << "Allocating " << amount << " bytes\n";
			allocations.push_back(alloc.getMemory(amount));
			totalAlloc += amount;

			if (totalAlloc > MEM_AMOUNT) {
				throw std::runtime_error("Test failed - runaway allocator! Ignore success messages below!");
			}
		}
	}
	catch (const std::runtime_error& e) {
		std::cout << "Got expected error - what(): " << e.what() << "\n";
		std::cout << "Successfully allocated " << totalAlloc << " bytes\n";
	}

	std::cout << "Current state:\n" << alloc.printMemory() << "\n";
	std::cout << "Recieved allocations:\n";
	printAllocations(allocations);

	std::cout << "Freeing random names...\n";
	bool oneFreed = false;

	while (!oneFreed && !allocations.empty()) {
		for (std::shared_ptr<AllocInfo> info : allocations) {
			if (ExMath::randomBool()) {
				std::cout << "Freeing " << info->start << "\n";
				info->inUse = false;
				totalAlloc -= info->size;
				oneFreed = true;
			}
		}
	}

	std::cout << "Current state:\n" << alloc.printMemory() << "\n";

	std::cout << "Allocating one more time...\n";

	try {
		while (true) {
			size_t amount = 0;

			if (RANDOM_ALLOC) {
				amount = ExMath::randomInt(ALLOC_MIN, ALLOC_MAX);
			}
			else {
				amount = ALLOC_AMOUNT;
			}

			std::cout << "Allocating " << amount << " bytes\n";
			allocations.push_back(alloc.getMemory(amount));
			totalAlloc += amount;

			if (totalAlloc > MEM_AMOUNT) {
				throw std::runtime_error("Test failed - runaway allocator! Ignore success messages below!");
			}
		}
	}
	catch (const std::runtime_error& e) {
		std::cout << "Got expected error - what(): " << e.what() << "\n";
		std::cout << "Successfully allocated " << totalAlloc << " bytes\n";
	}

	std::cout << "Current state:\n" << alloc.printMemory() << "\n";
	std::cout << "All allocations:\n";
	printAllocations(allocations);

	std::cout << "Defragmenting pool...\n";
	alloc.defragment();

	std::cout << "Current state:\n" << alloc.printMemory() << "\n";
	std::cout << "All allocations:\n";
	printAllocations(allocations);

	std::cout << "--------------------------------------- END ALLOCATOR TEST ---------------------------------------\n";
}

int main(int argc, char** argv) {
	MemoryAllocator alloc1(MEM_AMOUNT);

	try {
		testAllocator(alloc1);
	}
	catch (const std::exception& e) {
		std::cout << "Test failed! Reason: " << e.what() << "\n";
	}

	return 0;
}
