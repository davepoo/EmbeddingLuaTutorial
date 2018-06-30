#pragma once
#include <assert.h>
#include <cstdio>
#include <string.h>

/*! \brief Allocates from a fixed pool, does not deallocate. Asserts when out of memory. */
struct ArenaAllocator
{
	void* m_begin;
	void* m_end;
	char* m_curr;

	static constexpr int ALIGNMENT = 8;

	ArenaAllocator(void* begin, void* end) :
		m_begin(begin),
		m_end(end),
		m_curr(static_cast<char*>(begin))
	{
	}

	void* Allocate(size_t sizeBytes)
	{
		m_curr = (char*)( (uintptr_t)m_curr + (ALIGNMENT - 1) & ~(ALIGNMENT - 1) );
		assert(m_curr + sizeBytes < m_end); //we have run out of memory!!
		printf("Allocated %d bytes\n", (int)sizeBytes);
		void* ptr = m_curr;
		m_curr += sizeBytes;
		return ptr;
	}

	void DeAllocate(void* ptr, size_t osize)
	{
		assert(ptr != nullptr);		//can't decallocate null!!!
		printf("DeAllocated %d bytes\n", (int)osize);
		// we are just buring through memory.
	}

	void* ReAllocate(void* ptr, size_t osize, size_t nsize)
	{
		printf("ReAllocated %d bytes\n", (int)nsize);
		void* newPtr = Allocate(nsize);
		memcpy(newPtr, ptr, osize);
		DeAllocate(ptr, osize);
		return newPtr;
	}

	static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
		ArenaAllocator * pool = static_cast<ArenaAllocator *>(ud);
		if (nsize == 0)
		{
			if (ptr != nullptr)
			{
				pool->DeAllocate(ptr, osize);
			}
			return NULL;
		}
		else
		{
			if (ptr == nullptr)
			{
				return pool->Allocate(nsize);
			}
			else
			{
				return pool->ReAllocate(ptr, osize, nsize);
			}
		}
	}
};