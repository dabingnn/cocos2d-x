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

#include "renderer/CCRenderer.h"

#include <algorithm>

#include "renderer/CCTrianglesCommand.h"
#include "renderer/CCQuadCommand.h"
#include "renderer/CCBatchCommand.h"
#include "renderer/CCCustomCommand.h"
#include "renderer/CCGroupCommand.h"
#include "renderer/CCPrimitiveCommand.h"
#include "renderer/CCGLProgramCache.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCMeshCommand.h"
#include "renderer/CCVertexIndexBuffer.h"
#include "base/CCConfiguration.h"
#include "base/CCDirector.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerCustom.h"
#include "base/CCEventType.h"
#include "2d/CCCamera.h"
#include "2d/CCScene.h"

NS_CC_BEGIN

// helper
static bool compareRenderCommand(RenderCommand* a, RenderCommand* b)
{
    return a->getGlobalOrder() < b->getGlobalOrder();
}

static bool compare3DCommand(RenderCommand* a, RenderCommand* b)
{
    return  a->getDepth() > b->getDepth();
}

// queue

void RenderQueue::push_back(RenderCommand* command)
{
    float z = command->getGlobalOrder();
    if(z < 0)
    {
        _commands[QUEUE_GROUP::GLOBALZ_NEG].push_back(command);
    }
    else if(z > 0)
    {
        _commands[QUEUE_GROUP::GLOBALZ_POS].push_back(command);
    }
    else
    {
        if(command->is3D())
        {
            if(command->isTransparent())
            {
                _commands[QUEUE_GROUP::TRANSPARENT_3D].push_back(command);
            }
            else
            {
                _commands[QUEUE_GROUP::OPAQUE_3D].push_back(command);
            }
        }
        else
        {
            _commands[QUEUE_GROUP::GLOBALZ_ZERO].push_back(command);
        }
    }
}

ssize_t RenderQueue::size() const
{
    ssize_t result(0);
    for(int index = 0; index < QUEUE_GROUP::QUEUE_COUNT; ++index)
    {
        result += _commands[index].size();
    }
    
    return result;
}

void RenderQueue::sort()
{
    // Don't sort _queue0, it already comes sorted
    std::sort(std::begin(_commands[QUEUE_GROUP::TRANSPARENT_3D]), std::end(_commands[QUEUE_GROUP::TRANSPARENT_3D]), compare3DCommand);
    std::sort(std::begin(_commands[QUEUE_GROUP::GLOBALZ_NEG]), std::end(_commands[QUEUE_GROUP::GLOBALZ_NEG]), compareRenderCommand);
    std::sort(std::begin(_commands[QUEUE_GROUP::GLOBALZ_POS]), std::end(_commands[QUEUE_GROUP::GLOBALZ_POS]), compareRenderCommand);
}

RenderCommand* RenderQueue::operator[](ssize_t index) const
{
    for(int queIndex = 0; queIndex < QUEUE_GROUP::QUEUE_COUNT; ++queIndex)
    {
        if(index < static_cast<ssize_t>(_commands[queIndex].size()))
            return _commands[queIndex][index];
        else
        {
            index -= _commands[queIndex].size();
        }
    }
    
    CCASSERT(false, "invalid index");
    return nullptr;


}

void RenderQueue::clear()
{
    _commands.clear();
    for(int index = 0; index < QUEUE_COUNT; ++index)
    {
        _commands.push_back(std::vector<RenderCommand*>());
    }
}

void RenderQueue::saveRenderState()
{
    _isDepthEnabled = glIsEnabled(GL_DEPTH_TEST) != GL_FALSE;
    _isCullEnabled = glIsEnabled(GL_CULL_FACE) != GL_FALSE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &_isDepthWrite);
    
    CHECK_GL_ERROR_DEBUG();
}

void RenderQueue::restoreRenderState()
{
    CommandBufferCulling().setEnable(_isCullEnabled).apply();
    CommandBufferDepth().setEnable(_isDepthEnabled).setFunction(GL_LEQUAL).setWriteMask(_isDepthWrite).apply();
}

//
//
//
static const int DEFAULT_RENDER_QUEUE = 0;

//
// constructors, destructors, init
//
Renderer::Renderer()
:_lastMaterialID(0)
,_lastBatchedMeshCommand(nullptr)
,_filledVertex(0)
,_filledIndex(0)
,_numberQuads(0)
,_glViewAssigned(false)
,_isRendering(false)
,_isDepthTestFor2D(false)
#if CC_ENABLE_CACHE_TEXTURE_DATA
,_cacheTextureListener(nullptr)
#endif
{
    _groupCommandManager = new (std::nothrow) GroupCommandManager();
    
    _commandGroupStack.push(DEFAULT_RENDER_QUEUE);
    
    RenderQueue defaultRenderQueue;
    _renderGroups.push_back(defaultRenderQueue);
    _batchedCommands.reserve(BATCH_QUADCOMMAND_RESEVER_SIZE);

    // default clear color
    _clearColor = Color4F::BLACK;

    _verts.buffer = nullptr;
    _indices = nullptr;
    _quadVerts.buffer = nullptr;
    _quadIndices = nullptr;
}

Renderer::~Renderer()
{
    _renderGroups.clear();
    _groupCommandManager->release();
    
    CC_SAFE_RELEASE_NULL(_verts.buffer);
    CC_SAFE_RELEASE_NULL(_indices);
    CC_SAFE_RELEASE_NULL(_quadVerts.buffer);
    CC_SAFE_RELEASE_NULL(_quadIndices);

#if CC_ENABLE_CACHE_TEXTURE_DATA
    Director::getInstance()->getEventDispatcher()->removeEventListener(_cacheTextureListener);
#endif
}

void Renderer::initGLView()
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
    _cacheTextureListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom* event){
        /** listen the event that renderer was recreated on Android/WP8 */
        this->setupBuffer();
    });
    
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_cacheTextureListener, -1);
#endif
    
    setupBuffer();

    _glViewAssigned = true;
}

void Renderer::setupBuffer()
{
    // init triangles verts and indices info
    _verts.buffer = VertexBuffer::create(sizeof(V3F_C4B_T2F), VBO_SIZE);
    _verts.buffer->retain();
    _verts.stride = sizeof(V3F_C4B_T2F);
    _verts.layout.push_back({VertexSemantic::POSIITON,  VertexElementType::FLOAT,   3, false,   (void*)offsetof( V3F_C4B_T2F, vertices)});
    _verts.layout.push_back({VertexSemantic::COLOR,     VertexElementType::UBYTE,   4, true,    (void*)offsetof( V3F_C4B_T2F, colors)});
    _verts.layout.push_back({VertexSemantic::TEXCOORD0, VertexElementType::FLOAT,   2, false,   (void*)offsetof( V3F_C4B_T2F, texCoords)});
    
    _indices = IndexBuffer::create(IndexBuffer::IndexType::INDEX_TYPE_SHORT_16, INDEX_VBO_SIZE);
    _indices->retain();

    // init quadVerts and quadIndices info
    _quadVerts.buffer = VertexBuffer::create(sizeof(V3F_C4B_T2F), VBO_SIZE);
    _quadVerts.buffer->retain();
    _quadVerts.stride = sizeof(V3F_C4B_T2F);
    _quadVerts.layout.push_back({VertexSemantic::POSIITON,  VertexElementType::FLOAT,   3, false,   (void*)offsetof( V3F_C4B_T2F, vertices)});
    _quadVerts.layout.push_back({VertexSemantic::COLOR,     VertexElementType::UBYTE,   4, true,    (void*)offsetof( V3F_C4B_T2F, colors)});
    _quadVerts.layout.push_back({VertexSemantic::TEXCOORD0, VertexElementType::FLOAT,   2, false,   (void*)offsetof( V3F_C4B_T2F, texCoords)});

    _quadIndices = IndexBuffer::create(IndexBuffer::IndexType::INDEX_TYPE_SHORT_16, INDEX_VBO_SIZE);
    _quadIndices->retain();
    //setup index data for quads
    GLushort *quadIndices = new GLushort[INDEX_VBO_SIZE];
    for( int i=0; i < VBO_SIZE/4; i++)
    {
        quadIndices[i*6+0] = (GLushort) (i*4+0);
        quadIndices[i*6+1] = (GLushort) (i*4+1);
        quadIndices[i*6+2] = (GLushort) (i*4+2);
        quadIndices[i*6+3] = (GLushort) (i*4+3);
        quadIndices[i*6+4] = (GLushort) (i*4+2);
        quadIndices[i*6+5] = (GLushort) (i*4+1);
    }
    _quadIndices->updateIndices(quadIndices, INDEX_VBO_SIZE, 0);
    CC_SAFE_DELETE_ARRAY(quadIndices);
}

void Renderer::addCommand(RenderCommand* command)
{
    int renderQueue =_commandGroupStack.top();
    addCommand(command, renderQueue);
}

void Renderer::addCommand(RenderCommand* command, int renderQueue)
{
    CCASSERT(!_isRendering, "Cannot add command while rendering");
    CCASSERT(renderQueue >=0, "Invalid render queue");
    CCASSERT(command->getType() != RenderCommand::Type::UNKNOWN_COMMAND, "Invalid Command Type");

    _renderGroups[renderQueue].push_back(command);
}

void Renderer::pushGroup(int renderQueueID)
{
    CCASSERT(!_isRendering, "Cannot change render queue while rendering");
    _commandGroupStack.push(renderQueueID);
}

void Renderer::popGroup()
{
    CCASSERT(!_isRendering, "Cannot change render queue while rendering");
    _commandGroupStack.pop();
}

int Renderer::createRenderQueue()
{
    RenderQueue newRenderQueue;
    _renderGroups.push_back(newRenderQueue);
    return (int)_renderGroups.size() - 1;
}

void Renderer::processRenderCommand(RenderCommand* command)
{
    auto commandType = command->getType();
    if( RenderCommand::Type::TRIANGLES_COMMAND == commandType)
    {
        //Draw if we have batched other commands which are not triangle command
        flush3D();
        flushQuads();
        
        //Process triangle command
        auto cmd = static_cast<TrianglesCommand*>(command);
        
        //Draw batched Triangles if necessary
        if(cmd->isSkipBatching() || _filledVertex + cmd->getVertexCount() > VBO_SIZE || _filledIndex + cmd->getIndexCount() > INDEX_VBO_SIZE)
        {
            CCASSERT(cmd->getVertexCount()>= 0 && cmd->getVertexCount() < VBO_SIZE, "VBO for vertex is not big enough, please break the data down or use customized render command");
            CCASSERT(cmd->getIndexCount()>= 0 && cmd->getIndexCount() < INDEX_VBO_SIZE, "VBO for index is not big enough, please break the data down or use customized render command");
            //Draw batched Triangles if VBO is full
            drawBatchedTriangles();
        }
        
        //Batch Triangles
        _batchedCommands.push_back(cmd);
        
        fillVerticesAndIndices(cmd);
        
        if(cmd->isSkipBatching())
        {
            drawBatchedTriangles();
        }
        
    }
    else if ( RenderCommand::Type::QUAD_COMMAND == commandType )
    {
        //Draw if we have batched other commands which are not quad command
        flush3D();
        flushTriangles();
        
        //Process quad command
        auto cmd = static_cast<QuadCommand*>(command);
        
        //Draw batched quads if necessary
        if(cmd->isSkipBatching()|| (_numberQuads + cmd->getQuadCount()) * 4 > VBO_SIZE )
        {
            CCASSERT(cmd->getQuadCount()>= 0 && cmd->getQuadCount() * 4 < VBO_SIZE, "VBO for vertex is not big enough, please break the data down or use customized render command");
            //Draw batched quads if VBO is full
            drawBatchedQuads();
        }
        
        //Batch Quads
        _batchQuadCommands.push_back(cmd);
        
        fillQuads(cmd);
        
        if(cmd->isSkipBatching())
        {
            drawBatchedQuads();
        }
    }
    else if (RenderCommand::Type::MESH_COMMAND == commandType)
    {
        flush2D();
        auto cmd = static_cast<MeshCommand*>(command);
        
        if (cmd->isSkipBatching() || _lastBatchedMeshCommand == nullptr || _lastBatchedMeshCommand->getMaterialID() != cmd->getMaterialID())
        {
            flush3D();
            
            if(cmd->isSkipBatching())
            {
                cmd->execute();
            }
            else
            {
                cmd->preBatchDraw();
                cmd->batchDraw();
                _lastBatchedMeshCommand = cmd;
            }
        }
        else
        {
            cmd->batchDraw();
        }
    }
    else if(RenderCommand::Type::GROUP_COMMAND == commandType)
    {
        flush();
        int renderQueueID = ((GroupCommand*) command)->getRenderQueueID();
        visitRenderQueue(_renderGroups[renderQueueID]);
    }
    else if(RenderCommand::Type::CUSTOM_COMMAND == commandType)
    {
        flush();
        auto cmd = static_cast<CustomCommand*>(command);
        cmd->execute();
    }
    else if(RenderCommand::Type::BATCH_COMMAND == commandType)
    {
        flush();
        auto cmd = static_cast<BatchCommand*>(command);
        cmd->execute();
    }
    else if(RenderCommand::Type::PRIMITIVE_COMMAND == commandType)
    {
        flush();
        auto cmd = static_cast<PrimitiveCommand*>(command);
        cmd->execute();
    }
    else
    {
        CCLOGERROR("Unknown commands in renderQueue");
    }
}

void Renderer::visitRenderQueue(RenderQueue& queue)
{
    queue.saveRenderState();
    
    //
    //Process Global-Z < 0 Objects
    //
    const auto& zNegQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::GLOBALZ_NEG);
    if (zNegQueue.size() > 0)
    {
        CommandBufferDepth().setEnable(_isDepthTestFor2D).setFunction(GL_LEQUAL).setWriteMask(_isDepthTestFor2D).apply();

        for (auto it = zNegQueue.cbegin(); it != zNegQueue.cend(); ++it)
        {
            processRenderCommand(*it);
        }
        flush();
    }
    
    //
    //Process Opaque Object
    //
    const auto& opaqueQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::OPAQUE_3D);
    if (opaqueQueue.size() > 0)
    {
        //Clear depth to achieve layered rendering
        CommandBufferDepth().setEnable(true).setFunction(GL_LEQUAL).apply();
        for (auto it = opaqueQueue.cbegin(); it != opaqueQueue.cend(); ++it)
        {
            processRenderCommand(*it);
        }
        flush();
    }
    
    //
    //Process 3D Transparent object
    //
    const auto& transQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::TRANSPARENT_3D);
    if (transQueue.size() > 0)
    {
        CommandBufferDepth().setEnable(true).setFunction(GL_LEQUAL).setWriteMask(false).apply();
        for (auto it = transQueue.cbegin(); it != transQueue.cend(); ++it)
        {
            processRenderCommand(*it);
        }
        flush();
    }
    
    //
    //Process Global-Z = 0 Queue
    //
    const auto& zZeroQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::GLOBALZ_ZERO);
    if (zZeroQueue.size() > 0)
    {
        CommandBufferDepth().setEnable(_isDepthTestFor2D).setFunction(GL_LEQUAL).setWriteMask(_isDepthTestFor2D).apply();
        for (auto it = zZeroQueue.cbegin(); it != zZeroQueue.cend(); ++it)
        {
            processRenderCommand(*it);
        }
        flush();
    }
    
    //
    //Process Global-Z > 0 Queue
    //
    const auto& zPosQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::GLOBALZ_POS);
    if (zPosQueue.size() > 0)
    {
        for (auto it = zPosQueue.cbegin(); it != zPosQueue.cend(); ++it)
        {
            processRenderCommand(*it);
        }
        flush();
    }
    
    queue.restoreRenderState();
}

void Renderer::render()
{
    //Uncomment this once everything is rendered by new renderer
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //TODO: setup camera or MVP
    _isRendering = true;
    
    if (_glViewAssigned)
    {
        //Process render commands
        //1. Sort render commands based on ID
        for (auto &renderqueue : _renderGroups)
        {
            renderqueue.sort();
        }
        visitRenderQueue(_renderGroups[0]);
    }
    clean();
    _isRendering = false;
}

void Renderer::clean()
{
    // Clear render group
    for (size_t j = 0 ; j < _renderGroups.size(); j++)
    {
        //commands are owned by nodes
        // for (const auto &cmd : _renderGroups[j])
        // {
        //     cmd->releaseToCommandPool();
        // }
        _renderGroups[j].clear();
    }

    // Clear batch commands
    _batchedCommands.clear();
    _batchQuadCommands.clear();
    _filledVertex = 0;
    _filledIndex = 0;
    _numberQuads = 0;
    _lastMaterialID = 0;
    _lastBatchedMeshCommand = nullptr;
}

void Renderer::clear()
{
    //Enable Depth mask to make sure glClear clear the depth buffer correctly
    CommandBufferDepth().setEnable(true).apply();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CommandBufferDepth().setEnable(false).apply();
}

void Renderer::setDepthTest(bool enable)
{
    if (enable)
    {
        glClearDepth(1.0f);
//        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }
    
    CommandBufferDepth().setEnable(enable).setFunction(GL_LEQUAL).apply();
    _isDepthTestFor2D = enable;
    CHECK_GL_ERROR_DEBUG();
}

void Renderer::fillVerticesAndIndices(const TrianglesCommand* cmd)
{
    int vertexCount = int(cmd->getVertexCount());
    
    std::vector<V3F_C4B_T2F> verts;
    verts.resize(vertexCount);
    memcpy(verts.data(), cmd->getVertices(), sizeof(V3F_C4B_T2F) * vertexCount);
    
    const Mat4& modelView = cmd->getModelView();
    for (auto& vert : verts)
    {
        modelView.transformPoint(&vert.vertices);
    }
    
    std::vector<unsigned short> indices;
    indices.resize(cmd->getIndexCount());
    for(int index = 0; index < cmd->getIndexCount(); ++index)
    {
        indices[index] = cmd->getIndices()[index] + _filledVertex;
    }
    
    _verts.buffer->updateVertices(verts.data(), vertexCount, _filledVertex);
    _indices->updateIndices(&indices[0], int(cmd->getIndexCount()), _filledIndex);
    _filledVertex += cmd->getVertexCount();
    _filledIndex += cmd->getIndexCount();
}

void Renderer::fillQuads(const QuadCommand *cmd)
{
    int quadVertsCount = (int)cmd->getQuadCount() * 4;

    std::vector<V3F_C4B_T2F> verts;
    verts.resize(quadVertsCount);
    memcpy(verts.data(), cmd->getQuads(), sizeof(V3F_C4B_T2F) * quadVertsCount);
    
    const Mat4& modelView = cmd->getModelView();
    for (auto& vert : verts)
    {
        modelView.transformPoint(&vert.vertices);
    }
    
    _quadVerts.buffer->updateVertices(verts.data(), quadVertsCount, _numberQuads * 4);
    _numberQuads += cmd->getQuadCount();
}

void Renderer::drawBatchedTriangles()
{
    //TODO: we can improve the draw performance by insert material switching command before hand.

    int indexToDraw = 0;
    int startIndex = 0;

    //Upload buffer to VBO
    if(_filledVertex <= 0 || _filledIndex <= 0 || _batchedCommands.empty())
    {
        return;
    }

    CommandBufferVertexStreams().addStream(_verts).apply();

    //Start drawing verties in batch
    for(const auto& cmd : _batchedCommands)
    {
        auto newMaterialID = cmd->getMaterialID();
        if(_lastMaterialID != newMaterialID || newMaterialID == MATERIAL_ID_DO_NOT_BATCH)
        {
            //Draw triangles
            if(indexToDraw > 0)
            {
                CommandBufferDrawIndexed(_indices, GeometryType::TRIANGLES, startIndex*_indices->getSizePerIndex(), indexToDraw).apply();
                _drawnBatches++;
                _drawnVertices += indexToDraw;

                startIndex += indexToDraw;
                indexToDraw = 0;
            }

            //Use new material
            cmd->useMaterial();
            _lastMaterialID = newMaterialID;
        }

        indexToDraw += cmd->getIndexCount();
    }

    //Draw any remaining triangles
    if(indexToDraw > 0)
    {
        CommandBufferDrawIndexed(_indices, GeometryType::TRIANGLES, startIndex*_indices->getSizePerIndex(), indexToDraw).apply();
        _drawnBatches++;
        _drawnVertices += indexToDraw;
    }

    _batchedCommands.clear();
    _filledVertex = 0;
    _filledIndex = 0;
}

void Renderer::drawBatchedQuads()
{
    //TODO: we can improve the draw performance by insert material switching command before hand.
    
    if(_numberQuads <= 0 || _batchQuadCommands.empty())
    {
        return;
    }

    CommandBufferVertexStreams().addStream(_quadVerts).apply();

    //Start drawing verties in batch
    int indexToDraw = 0;
    int startIndex = 0;

    for(const auto& cmd : _batchQuadCommands)
    {
        auto newMaterialID = cmd->getMaterialID();
        if(_lastMaterialID != newMaterialID || newMaterialID == MATERIAL_ID_DO_NOT_BATCH)
        {
            //Draw quads
            if(indexToDraw > 0)
            {
                CommandBufferDrawIndexed(_quadIndices, GeometryType::TRIANGLES, startIndex*_quadIndices->getSizePerIndex(), indexToDraw).apply();
                _drawnBatches++;
                _drawnVertices += indexToDraw;
                
                startIndex += indexToDraw;
                indexToDraw = 0;
            }
            
            //Use new material
            cmd->useMaterial();
            _lastMaterialID = newMaterialID;
        }
        
        indexToDraw += cmd->getQuadCount() * 6;
    }
    
    //Draw any remaining quad
    if(indexToDraw > 0)
    {
        CommandBufferDrawIndexed(_quadIndices, GeometryType::TRIANGLES, startIndex*_quadIndices->getSizePerIndex(), indexToDraw).apply();
        _drawnBatches++;
        _drawnVertices += indexToDraw;
    }

    _batchQuadCommands.clear();
    _numberQuads = 0;
}

void Renderer::flush()
{
    flush2D();
    flush3D();
}

void Renderer::flush2D()
{
    flushQuads();
    flushTriangles();
}

void Renderer::flush3D()
{
    if (_lastBatchedMeshCommand)
    {
        _lastBatchedMeshCommand->postBatchDraw();
        _lastBatchedMeshCommand = nullptr;
    }
}

void Renderer::flushQuads()
{
    if(_numberQuads > 0)
    {
        drawBatchedQuads();
        _lastMaterialID = 0;
    }
}

void Renderer::flushTriangles()
{
    if(_filledIndex > 0)
    {
        drawBatchedTriangles();
        _lastMaterialID = 0;
    }
}

// helpers
bool Renderer::checkVisibility(const Mat4 &transform, const Size &size)
{
    auto scene = Director::getInstance()->getRunningScene();
    // only cull the default camera. The culling algorithm is valid for default camera.
    if (scene && scene->_defaultCamera != Camera::getVisitingCamera())
        return true;
    
    // half size of the screenR
    Size screen_half = Director::getInstance()->getWinSize();
    screen_half.width /= 2;
    screen_half.height /= 2;

    float hSizeX = size.width/2;
    float hSizeY = size.height/2;

    Vec4 v4world, v4local;
    v4local.set(hSizeX, hSizeY, 0, 1);
    transform.transformVector(v4local, &v4world);

    // center of screen is (0,0)
    v4world.x -= screen_half.width;
    v4world.y -= screen_half.height;

    // convert content size to world coordinates
    float wshw = std::max(fabsf(hSizeX * transform.m[0] + hSizeY * transform.m[4]), fabsf(hSizeX * transform.m[0] - hSizeY * transform.m[4]));
    float wshh = std::max(fabsf(hSizeX * transform.m[1] + hSizeY * transform.m[5]), fabsf(hSizeX * transform.m[1] - hSizeY * transform.m[5]));

    // compare if it in the positive quadrant of the screen
    float tmpx = (fabsf(v4world.x)-wshw);
    float tmpy = (fabsf(v4world.y)-wshh);
    bool ret = (tmpx < screen_half.width && tmpy < screen_half.height);

    return ret;
}


void Renderer::setClearColor(const Color4F &clearColor)
{
    _clearColor = clearColor;
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}


//begin CommandBuffer
void Renderer::applyCommandBuffer(CommandBuffer *cmdBuf)
{
    switch (cmdBuf->_type) {
        case CommandBufferType::GPUPROGRAM:
        {
            CommandBufferGPUProgram& cmd = *static_cast<CommandBufferGPUProgram*>(cmdBuf);
            currentGLProgram = cmd.program;
            currentGLProgram->use();
            break;
        }
        case CommandBufferType::UNIFORM:
        {
            CommandBufferUniform& cmd = *static_cast<CommandBufferUniform*>(cmdBuf);
            for(int index = 0; index < cmd.buffer.names.size(); ++index)
            {
                auto name = cmd.buffer.names[index];
                auto slot = cmd.buffer.layout[index];
                auto data = cmd.buffer.data[index];
                if(data.getData() == nullptr) continue;
                if(UniformBuffer::ConstantType::FLOAT == slot.type)
                {
                    currentGLProgram->setUniformLocationWith1fv(slot.constantSlot, (const GLfloat*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::FLOAT2 == slot.type)
                {
                    currentGLProgram->setUniformLocationWith2fv(slot.constantSlot, (const GLfloat*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::FLOAT3 == slot.type)
                {
                    currentGLProgram->setUniformLocationWith3fv(slot.constantSlot, (const GLfloat*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::FLOAT4 == slot.type)
                {
                    currentGLProgram->setUniformLocationWith4fv(slot.constantSlot, (const GLfloat*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::FMAT3X3 == slot.type)
                {
                    currentGLProgram->setUniformLocationWithMatrix3fv(slot.constantSlot, (const GLfloat*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::FMAT4X4 == slot.type)
                {
                    currentGLProgram->setUniformLocationWithMatrix4fv(slot.constantSlot, (const GLfloat*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::INT == slot.type)
                {
                    currentGLProgram->setUniformLocationWith1iv(slot.constantSlot, (const GLint*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::INT2 == slot.type)
                {
                    currentGLProgram->setUniformLocationWith2iv(slot.constantSlot, (const GLint*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::INT3 == slot.type)
                {
                    currentGLProgram->setUniformLocationWith3iv(slot.constantSlot, (const GLint*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::INT4 == slot.type)
                {
                    currentGLProgram->setUniformLocationWith4iv(slot.constantSlot, (const GLint*)data.getData(), slot.count);
                }
                else if(UniformBuffer::ConstantType::TEXTURE == slot.type)
                {
                    const GLint* textureData = (const GLint*) data.getData();
                    currentGLProgram->setUniformLocationWith1i(slot.constantSlot, textureData[0]);
                    GL::bindTexture2DN(textureData[0], textureData[1]);
                }
                else if(UniformBuffer::ConstantType::TEXTURECUBE == slot.type)
                {
                    const GLint* textureData = (const GLint*) data.getData();
                    currentGLProgram->setUniformLocationWith1i(slot.constantSlot, textureData[0]);
                    GL::bindTextureN(textureData[0], textureData[1],GL_TEXTURE_CUBE_MAP);
                }
                else
                {
                    CCLOG("Error, invalid constant type");
                }
            }
            break;
        }
        case CommandBufferType::STEAMS:
        {
            CommandBufferVertexStreams& cmd = *static_cast<CommandBufferVertexStreams*>(cmdBuf);
            currentStreams = cmd.streams;
            break;
        }
        case CommandBufferType::DRAW:
        {
            break;
        }
        case CommandBufferType::DRAWINDEXED:
        {
            CommandBufferDrawIndexed& cmd = *static_cast<CommandBufferDrawIndexed*>(cmdBuf);
            
            // bind vertex buffer
            for (auto& s : currentStreams)
            {
                if (!s.buffer) {
                    CCLOG("Error, invalid VertexBuffer in VertexStream!.");
                    continue;
                }
                glBindBuffer(GL_ARRAY_BUFFER, s.buffer->getVBO());
                CHECK_GL_ERROR_DEBUG();
                
                auto& semanticMap = currentGLProgram->getAttributeBindings();
                GLuint indx;
                for (auto& e : s.layout) {
                    indx = semanticMap[e.semantic];
                    glEnableVertexAttribArray(indx);
                    glVertexAttribPointer(indx, e.count, VertexElementTypeToGLType(e.type), e.normalize ? GL_TRUE : GL_FALSE, s.stride, e.offset);
                    CHECK_GL_ERROR_DEBUG();
                }
            }
            
            // bind index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd.indices->getVBO());
            CHECK_GL_ERROR_DEBUG();
            
            // draw element
            glDrawElements(GeometryTypeToGLType(cmd.type), (GLsizei) cmd.count, IndexTypeToGLType(cmd.indices->getType()), (GLvoid*)cmd.start);
            CHECK_GL_ERROR_DEBUG();
            
            // unbind buffers
            for (auto& s : currentStreams)
            {
                auto& semanticMap = currentGLProgram->getAttributeBindings();
                for (auto& e : s.layout) {
                    glDisableVertexAttribArray(semanticMap[e.semantic]);
                    CHECK_GL_ERROR_DEBUG();
                }
            }
            //this line is added for backward compatibility
            GL::enableVertexAttribs(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            break;
        }
        case CommandBufferType::DEPTH:
        {
            CommandBufferDepth &cmd = *static_cast<CommandBufferDepth *>(cmdBuf);
            if (cmd.flags.setEnabled)
            {
                if (cmd.flags.enabled)
                {
                    glEnable(GL_DEPTH_TEST);
                }
                else
                {
                    glDisable(GL_DEPTH_TEST);
                }
            }
            if (cmd.flags.setFunction)
            {
                glDepthFunc(cmd.function);
            }
            if (cmd.flags.setWriteMask)
            {
                glDepthMask(cmd.flags.writeEnabled);
            }
            if (cmd.flags.setRangef)
            {
                glDepthRangef(cmd.rangefNear, cmd.rangfFar);
            }
            if (cmd.flags.setPolygonOffset)
            {
                glPolygonOffset(cmd.polygonOffsetFactor, cmd.polygonOffsetUnits);
            }
            break;
        }
        case CommandBufferType::BLEND:
        {
            CommandBufferBlend &cmd = *static_cast<CommandBufferBlend *>(cmdBuf);
            if (cmd.flags.setEnabled)
            {
                if (cmd.flags.enabled)
                {
                    glEnable(GL_BLEND);
                }
                else
                {
                    glDisable(GL_BLEND);
                }
            }
            if (cmd.flags.setColor)
            {
                glBlendColor(cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
            }
            if (cmd.flags.setEquation)
            {
                glBlendEquationSeparate(cmd.equation[0], cmd.equation[1]);
            }
            if (cmd.flags.setFunction)
            {
                glBlendFuncSeparate(cmd.srcFunc[0], cmd.dstFunc[0], cmd.srcFunc[1], cmd.dstFunc[1]);
            }
            break;
        }
        case CommandBufferType::STENCIL:
        {
            CommandBufferStencil &cmd = *static_cast<CommandBufferStencil *>(cmdBuf);
            if (cmd.flags.setEnabled) {
                if (cmd.flags.enabled)
                {
                    glEnable(GL_STENCIL_TEST);
                }
                else
                {
                    glDisable(GL_STENCIL_TEST);
                }
            }
            if (cmd.flags.setFuncFront) {
                glStencilFuncSeparate(GL_FRONT, cmd.func[0].func, cmd.func[0].ref, cmd.func[0].mask);
            }
            if (cmd.flags.setFuncBack) {
                glStencilFuncSeparate(GL_BACK, cmd.func[1].func, cmd.func[1].ref, cmd.func[1].mask);
            }
            if (cmd.flags.setMaskFront) {
                glStencilMaskSeparate(GL_FRONT, cmd.mask[0]);
            }
            if (cmd.flags.setMaskBack) {
                glStencilMaskSeparate(GL_BACK, cmd.mask[1]);
            }
            if (cmd.flags.setOpFront) {
                glStencilOpSeparate(GL_FRONT, cmd.op[0].sfail, cmd.op[0].dpfail, cmd.op[0].dppass);
            }
            if (cmd.flags.setOpBack) {
                glStencilOpSeparate(GL_BACK, cmd.op[1].sfail, cmd.op[1].dpfail, cmd.op[1].dppass);
            }
            break;
        }
        case CommandBufferType::CULLING:
        {
            CommandBufferCulling &cmd = *static_cast<CommandBufferCulling *>(cmdBuf);
            if (cmd.flags.setEnabled)
            {
                if (cmd.flags.enabled)
                {
                    glEnable(GL_CULL_FACE);
                }
                else
                {
                    glDisable(GL_CULL_FACE);
                }
            }
            if (cmd.flags.setCullFace)
            {
                glCullFace(cmd.cullFace);
            }
            if (cmd.flags.setFrontFace)
            {
                glFrontFace(cmd.frontFace);
            }
            break;
        }
        case CommandBufferType::SCISSOR:
        {
            CommandBufferScissor &cmd = *static_cast<CommandBufferScissor *>(cmdBuf);
            if (cmd.flags.setEnabled)
            {
                if (cmd.flags.enabled)
                {
                    glEnable(GL_SCISSOR_TEST);
                }
                else
                {
                    glDisable(GL_SCISSOR_TEST);
                }
            }
            if (cmd.flags.setBox)
            {
                glScissor(cmd.x, cmd.y, cmd.width, cmd.height);
            }
            break;
        }
        default:
            break;
    }
}
//end CommandBuffer

NS_CC_END
