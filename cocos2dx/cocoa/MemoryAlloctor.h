
#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H
namespace Memory
{
    class StdAllocPolicy
    {
    public:
        static void* allocBytes(size_t sz)
        {
            return malloc(sz);
        }
        static void deAllocBytes(void* mem)
        {
            return free(mem);
        }
    };
    
    class NedAllocPolicy
    {
    public:
        static void* allocBytes(size_t sz);
        static void deAllocBytes(void* mem);
    };
    
    class NedPoolAllocPolicy
    {
    public:
        static void* allocBytes(size_t sz);
        static void deAllocBytes(void* mem);
    };
    
    template <class Alloc>
    class MemoryAllocator
    {
    public:
        explicit MemoryAllocator() {}
        ~MemoryAllocator() {}
        
        void* operator new(size_t sz)
        {
            return Alloc::allocBytes(sz);
        }
        void* operator new(size_t sz, const char* file, int line, const char* func)
        {
            return Alloc::allocBytes(sz);
        }

        void* operator new[](size_t sz)
        {
            return Alloc::allocBytes(sz);
        }
        void* operator new[](size_t sz, const char* file, int line, const char* func)
        {
            return Alloc::allocBytes(sz);
        }
        
        void operator delete(void* ptr)
        {
            return Alloc::deAllocBytes(ptr);
        }
        void operator delete(void* ptr, const char*, int, const char*)
        {
            return Alloc::deAllocBytes(ptr);
        }
        
        void operator delete[](void* ptr)
        {
            return Alloc::deAllocBytes(ptr);
        }
        void operator delete[](void* ptr, const char*, int, const char*)
        {
            return Alloc::deAllocBytes(ptr);
        }
    };
}

#endif
