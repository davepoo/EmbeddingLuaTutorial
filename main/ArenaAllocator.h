#pragma once
#include <assert.h>
#include <cstdio>
#include <string.h>

/*! \brief Allocates from a fixed pool.
*	Aligns all memory to 8 bytes
*	Has a min allocation of 64 bytes
*	Puts all free'd blocks on a free list.
*	Asserts when out of memory. */
struct ArenaAllocator
{
	void* m_begin;
	void* m_end;
	char* m_curr;

	static constexpr int ALIGNMENT = 8;
	static constexpr int MIN_BLOCK_SIZE = ALIGNMENT * 8;

	struct FreeList
	{
		FreeList* m_next;
	};

	FreeList* m_freeListHead;

	ArenaAllocator(void* begin, void* end) :
		m_begin(begin),
		m_end(end)
	{
		Reset();
	}

	void Reset()
	{
		m_freeListHead = nullptr;
		m_curr = static_cast<char*>(m_begin);
	}

	size_t SizeToAllocate(size_t size)
	{
		size_t allocatedSize = size;
		if (allocatedSize < MIN_BLOCK_SIZE)
		{
			allocatedSize = MIN_BLOCK_SIZE;
		}
		return allocatedSize;
	}

	void* Allocate(size_t sizeBytes)
	{
		size_t allocatedBytes = SizeToAllocate(sizeBytes);
		if (allocatedBytes <= MIN_BLOCK_SIZE && m_freeListHead)
		{
			//printf("-- allocated from the freelist --\n");
			void* ptr = m_freeListHead;
			m_freeListHead = m_freeListHead->m_next;
			return ptr;
		}
		else
		{
			m_curr = (char*)((uintptr_t)m_curr + (ALIGNMENT - 1) & ~(ALIGNMENT - 1));
			assert(m_curr + allocatedBytes < m_end); //we have run out of memory!!
			//printf("Allocated %d bytes\n", (int)allocatedBytes);
			void* ptr = m_curr;
			m_curr += allocatedBytes;
			return ptr;
		}
	}

	void DeAllocate(void* ptr, size_t osize)
	{
		assert(ptr != nullptr);		//can't decallocate null!!!
		size_t allocatedBytes = SizeToAllocate(osize);
		//printf("DeAllocated %d bytes\n", (int)allocatedBytes);
		if (allocatedBytes >= MIN_BLOCK_SIZE)
		{
			//printf("-- deallocated to the freelist --\n");
			FreeList* newHead = static_cast<FreeList*>( ptr );
			newHead->m_next = m_freeListHead;
			m_freeListHead = newHead;
		}
		else
		{
			// we are just buring through memory.
		}
	}

	void* ReAllocate(void* ptr, size_t osize, size_t nsize)
	{
		//printf("ReAllocated %d bytes\n", (int)nsize);
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