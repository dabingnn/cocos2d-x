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

#include "base/CCEventListenerAcceleration.h"
#include "base/CCEventAcceleration.h"
#include "base/CCConsole.h"

NS_CC_BEGIN

const std::string EventListenerAcceleration::LISTENER_ID = "__cc_acceleration";
const std::string EventListenerGyroscope::LISTENER_ID = "__cc_gyroscope";

EventListenerAcceleration::EventListenerAcceleration()
{

}

EventListenerAcceleration::~EventListenerAcceleration()
{
    CCLOGINFO("In the destructor of AccelerationEventListener. %p", this);
}

EventListenerAcceleration* EventListenerAcceleration::create(const std::function<void(Acceleration*, Event*)>& callback)
{
    EventListenerAcceleration* ret = new (std::nothrow) EventListenerAcceleration();
    if (ret && ret->init(callback))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

bool EventListenerAcceleration::init(const std::function<void(Acceleration*, Event* event)>& callback)
{
    auto listener = [this](Event* event){
        auto accEvent = static_cast<EventAcceleration*>(event);
        this->onAccelerationEvent(&accEvent->_acc, event);
    };
    
    if (EventListener::init(Type::ACCELERATION, LISTENER_ID, listener))
    {
        onAccelerationEvent = callback;
        return true;
    }
    
    return false;
}

EventListenerAcceleration* EventListenerAcceleration::clone()
{
    auto ret = new (std::nothrow) EventListenerAcceleration();
    
    if (ret && ret->init(onAccelerationEvent))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

bool EventListenerAcceleration::checkAvailable()
{
    CCASSERT(onAccelerationEvent, "");
    
    return true;
}

EventListenerGyroscope::EventListenerGyroscope()
{
    
}

EventListenerGyroscope::~EventListenerGyroscope()
{
    CCLOGINFO("In the destructor of GyroscopeEventListener. %p", this);
}

EventListenerGyroscope* EventListenerGyroscope::create(const std::function<void(Gyroscope*, Event*)>& callback)
{
    auto ret = new (std::nothrow) EventListenerGyroscope();
    if (ret && ret->init(callback))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

bool EventListenerGyroscope::init(const std::function<void(Gyroscope*, Event* event)>& callback)
{
    auto listener = [this](Event* event){
        auto gyroEvent = static_cast<EventGyroscope*>(event);
        this->onGyroscopeEvent(&gyroEvent->_val, event);
    };
    
    if (EventListener::init(Type::GYROSCOPE, LISTENER_ID, listener))
    {
        onGyroscopeEvent = callback;
        return true;
    }
    
    return false;
}

EventListenerGyroscope* EventListenerGyroscope::clone()
{
    auto ret = new (std::nothrow) EventListenerGyroscope();
    
    if (ret && ret->init(onGyroscopeEvent))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}

bool EventListenerGyroscope::checkAvailable()
{
    CCASSERT(onGyroscopeEvent, "");
    
    return true;
}

NS_CC_END
