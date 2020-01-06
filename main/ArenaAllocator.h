#pragma once
#include <assert.h>
#include <cstdio>
#include <string.h>

/*! \brief Allocates from global memory (NOTE: does not currently align memory) */
struct GlobalAllocator
{
	void* Allocate(size_t sizeBytes)
	{
		return ::operator new(sizeBytes);
	}

	void DeAllocate(void* ptr, size_t /*osize*/)
	{
		assert(ptr != nullptr);		//can't decallocate null!!!
		::operator delete(ptr);
	}

	void* ReAllocate(void* ptr, size_t osize, size_t nsize)
	{
		size_t bytesToCopy = osize;
		if (nsize < bytesToCopy)
		{
			bytesToCopy = nsize;
		}
		void* newPtr = Allocate(nsize);
		memcpy(newPtr, ptr, bytesToCopy);
		DeAllocate(ptr, osize);
		return newPtr;
	}

	static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
		GlobalAllocator * pool = static_cast<GlobalAllocator *>(ud);
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

/*! \brief Allocates from a fixed pool.
*	Aligns all memory to 8 bytes
*	Has a min allocation of 64 bytes
*	Puts all free'd blocks on a free list.
*	Falls back to GlobalAllocator when out of memory. */
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
	GlobalAllocator m_globalAllocator;

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
		if ( sizeBytes <= MIN_BLOCK_SIZE && m_freeListHead)
		{
			//printf("-- allocated from the freelist --\n");
			void* ptr = m_freeListHead;
			m_freeListHead = m_freeListHead->m_next;
			return ptr;
		}
		else
		{
			size_t allocatedBytes = SizeToAllocate( sizeBytes );
			m_curr = (char*)((uintptr_t)m_curr + (ALIGNMENT - 1) & ~(ALIGNMENT - 1));
			if (m_curr + allocatedBytes <= m_end)
			{
				//printf("Allocated %d bytes\n", (int)allocatedBytes);
				void* ptr = m_curr;
				m_curr += allocatedBytes;
				return ptr;
			}
			else
			{
				return m_globalAllocator.Allocate(sizeBytes);
			}
		}
	}

	void DeAllocate(void* ptr, size_t osize)
	{
		assert(ptr != nullptr);		//can't decallocate null!!!
		if (ptr >= m_begin && ptr <= m_end)
		{
			size_t allocatedBytes = SizeToAllocate(osize);
			//printf("DeAllocated %d bytes\n", (int)allocatedBytes);
			assert( allocatedBytes >= MIN_BLOCK_SIZE );			
			//printf("-- deallocated to the freelist --\n");
			FreeList* newHead = static_cast<FreeList*>(ptr);
			newHead->m_next = m_freeListHead;
			m_freeListHead = newHead;
			
		}
		else
		{
			m_globalAllocator.DeAllocate(ptr, osize);
		}
	}

	void* ReAllocate(void* ptr, size_t osize, size_t nsize)
	{
		//printf("ReAllocated %d bytes\n", (int)nsize);
		size_t bytesToCopy = osize;
		if (nsize < bytesToCopy)
		{
			bytesToCopy = nsize;
		}
		void* newPtr = Allocate(nsize);
		memcpy(newPtr, ptr, bytesToCopy);
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