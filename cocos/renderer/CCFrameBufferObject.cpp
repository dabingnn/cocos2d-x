/****************************************************************************
 Copyright (c) 2015 Chukong Technologies Inc.
 
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

#include "renderer/CCFrameBufferObject.h"
#include "base/CCDirector.h"
#include "base/CCEventCustom.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventType.h"

NS_CC_BEGIN

GLuint FrameBufferObject::_defaultFBO(0);
std::set<FrameBufferObject*> FrameBufferObject::_frameBufferObjects;

RenderTargetBase::RenderTargetBase()
{
    
}

RenderTargetBase::~RenderTargetBase()
{
    
}

bool RenderTargetBase::init(unsigned int width, unsigned int height)
{
    _width = width;
    _height = height;
    return true;
}

RenderTarget* RenderTarget::create(unsigned int width, unsigned int height, Texture2D::PixelFormat format/* = Texture2D::PixelFormat::RGBA8888*/)
{
    auto result = new (std::nothrow) RenderTarget();
    if(result && result->init(width, height,format))
    {
        result->autorelease();
        return result;
    }
    else
    {
        CC_SAFE_DELETE(result);
        return nullptr;
    }
}

bool RenderTarget::init(unsigned int width, unsigned int height, Texture2D::PixelFormat format)
{
    if(!RenderTargetBase::init(width, height))
    {
        return false;
    }
    
    _texture = new (std::nothrow) Texture2D();
    if(nullptr == _texture) return false;
    //TODO: FIX me, get the correct bit depth for pixelformat
    auto dataLen = width * height * 4;
    auto data = malloc(dataLen);
    if( nullptr == data) return false;
    
    memset(data, 0, dataLen);
    if(_texture->initWithData(data, dataLen, format, width, height, Size(width, height)))
    {
        _texture->autorelease();
        CC_SAFE_RETAIN(_texture);
        free(data);
    }
    else
    {
        CC_SAFE_DELETE(_texture);
        free(data);
        return false;
    }
#if CC_ENABLE_CACHE_TEXTURE_DATA
    _rebuildTextureListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom* event){
        auto dataLen = _width * _height * 4;
        auto data = malloc(dataLen);
        _texture->initWithData(data, dataLen,_texture->getPixelFormat(), _width, _height, Size(_width, _height));
        free(data);
        CCLOG("RenderTarget Texture recreated is %d", _texture->getName());
    });
    
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_rebuildTextureListener, -1);
#endif
    return true;
}

RenderTarget::RenderTarget()
: _texture(nullptr)
#if CC_ENABLE_CACHE_TEXTURE_DATA
, _rebuildTextureListener(nullptr)
#endif
{
}

RenderTarget::~RenderTarget()
{
    CC_SAFE_RELEASE_NULL(_texture);
#if CC_ENABLE_CACHE_TEXTURE_DATA
    Director::getInstance()->getEventDispatcher()->removeEventListener(_rebuildTextureListener);
#endif
}

RenderTargetDepthStencil::RenderTargetDepthStencil()
: _depthStencilBuffer(0)
#if CC_ENABLE_CACHE_TEXTURE_DATA
, _reBuildDepthStencilListener(nullptr)
#endif
{
}

RenderTargetDepthStencil::~RenderTargetDepthStencil()
{
    if(glIsRenderbuffer(_depthStencilBuffer))
    {
        glDeleteRenderbuffers(1, &_depthStencilBuffer);
        _depthStencilBuffer = 0;
    }
#if CC_ENABLE_CACHE_TEXTURE_DATA
    Director::getInstance()->getEventDispatcher()->removeEventListener(_reBuildDepthStencilListener);
#endif
}

bool RenderTargetDepthStencil::init(unsigned int width, unsigned int height)
{
    if(!RenderTargetBase::init(width, height)) return false;
    GLint oldRenderBuffer(0);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer);
    
    //generate depthStencil
    glGenRenderbuffers(1, &_depthStencilBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer);
    
#if CC_ENABLE_CACHE_TEXTURE_DATA
    _reBuildDepthStencilListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom* event){
        /** listen the event that renderer was recreated on Android/WP8 */
        GLint oldRenderBuffer(0);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer);
        
        glGenRenderbuffers(1, &_depthStencilBuffer);
        //generate depthStencil
        glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height);
        glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer);
        CCLOG("RenderTargetDepthStencil recreated, _depthStencilBuffer is %d", _depthStencilBuffer);
    });
    
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_reBuildDepthStencilListener, -1);
#endif
    
    return true;
}


RenderTargetDepthStencil* RenderTargetDepthStencil::create(unsigned int width, unsigned int height)
{
    auto result = new (std::nothrow) RenderTargetDepthStencil();
    
    if(result && result->init(width, height))
    {
        result->autorelease();
        return result;
    }
    else
    {
        CC_SAFE_DELETE(result);
        return nullptr;
    }
}

void FrameBufferObject::initDefaultFBO()
{
    GLint fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
    _defaultFBO = fbo;
}

void FrameBufferObject::applyDefaultFBO()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBO);
}

void FrameBufferObject::clearAllFBOs()
{
    for (auto fbo : _frameBufferObjects)
    {
        fbo->clearFBO();
    }
}

FrameBufferObject* FrameBufferObject::create(uint8_t fid, unsigned int width, unsigned int height)
{
    auto result = new (std::nothrow) FrameBufferObject();
    if(result && result->init(fid, width, height))
    {
        result->autorelease();
        return result;
    }
    else
    {
        CC_SAFE_DELETE(result);
        return nullptr;
    }
}

bool FrameBufferObject::init(uint8_t fid, unsigned int width, unsigned int height)
{
    _fid = fid;
    _width = width;
    _height = height;
    
    GLint oldfbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfbo);

    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, oldfbo);
    
//    _rt = RenderTarget::create(width, height);
//    if(nullptr == _rt) return false;
    
#if CC_ENABLE_CACHE_TEXTURE_DATA
    _dirtyFBOListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom* event){
        GLint oldfbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfbo);

        glGenFramebuffers(1, &_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, oldfbo);
        CCLOG("Recreate FrameBufferObject _fbo is %d", _fbo);
        _fboBindingDirty = true;
    });
    
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_dirtyFBOListener, -1);
#endif
    return true;
}

FrameBufferObject::FrameBufferObject()
: _clearColor(Color4F(0, 0, 0, 1))
, _clearDepth(1.0)
, _clearStencil(0)
, _fbo(0)
, _rt(nullptr)
, _rtDepthStencil(nullptr)
, _fboBindingDirty(true)
#if CC_ENABLE_CACHE_TEXTURE_DATA
, _dirtyFBOListener(nullptr)
#endif
{
    _frameBufferObjects.insert(this);
}

FrameBufferObject::~FrameBufferObject()
{
    CC_SAFE_RELEASE_NULL(_rt);
    CC_SAFE_RELEASE_NULL(_rtDepthStencil);
    glDeleteFramebuffers(1, &_fbo);
    _fbo = 0;
    _frameBufferObjects.erase(this);
#if CC_ENABLE_CACHE_TEXTURE_DATA
    Director::getInstance()->getEventDispatcher()->removeEventListener(_dirtyFBOListener);
#endif
}

void FrameBufferObject::clearFBO()
{
    applyFBO();
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClearDepth(_clearDepth);
    glClearStencil(_clearStencil);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    applyDefaultFBO();
}

void FrameBufferObject::AttachRenderTarget(RenderTarget* rt)
{
    CC_ASSERT(rt);
    if(rt->getWidth() != _width || rt->getHeight() != _height)
    {
        CCLOG("Error, attach a render target with different size, Skip.");
        return;
    }
    CC_SAFE_RETAIN(rt);
    CC_SAFE_RELEASE(_rt);
    _rt = rt;
    _fboBindingDirty = true;
}

void FrameBufferObject::applyFBO()
{
    CHECK_GL_ERROR_DEBUG();
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    CHECK_GL_ERROR_DEBUG();
    if(_fboBindingDirty)
    {
        CHECK_GL_ERROR_DEBUG();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _rt->getTexture()->getName(), 0);
        CHECK_GL_ERROR_DEBUG();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, nullptr == _rtDepthStencil ? 0 : _rtDepthStencil->getDepthStencilBuffer());
        CHECK_GL_ERROR_DEBUG();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, nullptr == _rtDepthStencil ? 0 : _rtDepthStencil->getDepthStencilBuffer());
        CHECK_GL_ERROR_DEBUG();
        CCLOG("FBO is %d _fbo %d color, %d ds", _fbo, _rt->getTexture()->getName(), _rtDepthStencil->getDepthStencilBuffer());
        _fboBindingDirty = false;
    }
    if(GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
        CCLOG("FrameBuffer Status Error %d", (int)glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    CHECK_GL_ERROR_DEBUG();
}

void FrameBufferObject::AttachDepthStencilTarget(RenderTargetDepthStencil* rt)
{
    if(nullptr != rt && (rt->getWidth() != _width || rt->getHeight() != _height))
    {
        CCLOG("Error, attach a render target Depth stencil with different size, Skip.");
        return;
    }
    CC_SAFE_RETAIN(rt);
    CC_SAFE_RELEASE(_rtDepthStencil);
    _rtDepthStencil = rt;
    _fboBindingDirty = true;
}
NS_CC_END