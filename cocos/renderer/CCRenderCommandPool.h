/****************************************************************************
 Copyright (c) 2013-2014 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/


#ifndef __CC_RENDERCOMMANDPOOL_H__
#define __CC_RENDERCOMMANDPOOL_H__

#include <list>

#include "platform/CCPlatformMacros.h"
#include "base/ccCArray.h"

NS_CC_BEGIN

template <class T>
class FreePool
{
public:
    FreePool()
    {
        static const size_t INTERNAL_CAPACITY = 128;
        _array = (T**)malloc(sizeof(T*) * INTERNAL_CAPACITY);
        _capacity = INTERNAL_CAPACITY;
        _count = 0;
    }
    ~FreePool()
    {
        free(_array);
    }
    
    T* get()
    {
        if(0 == _count) return nullptr;
        return _array[--_count];
    }
    
    void push(T* ptr)
    {
        if(_count + 1 == _capacity)
        {
            _capacity = _capacity * 2;
            _array = (T**)realloc(_array, sizeof(T*) * _capacity);
        }
        _array[_count++] = ptr;
    }
    
    size_t count() const
    {
        return _count;
    }
    
    bool empty() const
    {
        return 0 == _count;
    }
    
private:
    T** _array;
    size_t _capacity;
    size_t _count;
};

template <class T>
class RenderCommandPool
{
public:

    static T* fetchCommand()
    {
        T* result = nullptr;
        if(_freePool.empty())
        {
            AllocateCommands();
        }
        result = _freePool.get();
        result->setInPool();
        return result;
    }
    
    static void pushBackCommand(T* ptr)
    {
        _freePool.push(ptr);
        
    }
private:
    static void AllocateCommands()
    {
        static const int  COMMANDS_ALLOCATE_BLOCK_SIZE= 128;
        _allocatedPoolBlocks.push_back(std::vector<T>());
        _allocatedPoolBlocks.back().resize(COMMANDS_ALLOCATE_BLOCK_SIZE);
        auto& commands = _allocatedPoolBlocks.back();
        for(int index = 0; index < COMMANDS_ALLOCATE_BLOCK_SIZE; ++index)
        {
            _freePool.push(&commands[index]);
        }
    }

    static std::list<std::vector<T>> _allocatedPoolBlocks;
    static FreePool<T> _freePool;
};
template <class T>
std::list<std::vector<T>> RenderCommandPool<T>::_allocatedPoolBlocks;
template <class T> FreePool<T> RenderCommandPool<T>::_freePool;
NS_CC_END

#endif
