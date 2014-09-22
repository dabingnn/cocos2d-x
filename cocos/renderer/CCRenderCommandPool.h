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

NS_CC_BEGIN

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
        result = _freePool.front();
        _freePool.pop_front();
        result->setInPool();
        return result;
    }
    
    static void pushBackCommand(T* ptr)
    {
        _freePool.push_front(ptr);
        
    }
private:
    static void AllocateCommands()
    {
        static const int COMMANDS_ALLOCATE_BLOCK_SIZE = 128;
        _allocatedPoolBlocks.push_back(std::vector<T>());
        _allocatedPoolBlocks.back().resize(COMMANDS_ALLOCATE_BLOCK_SIZE);
        auto& commands = _allocatedPoolBlocks.back();
        for(int index = 0; index < COMMANDS_ALLOCATE_BLOCK_SIZE; ++index)
        {
            _freePool.push_back(&commands[index]);
        }
    }

    static std::list<std::vector<T>> _allocatedPoolBlocks;
    static std::list<T*> _freePool;
};
template <class T>
std::list<std::vector<T>> RenderCommandPool<T>::_allocatedPoolBlocks;
template <class T>
std::list<T*> RenderCommandPool<T>::_freePool;

NS_CC_END

#endif
