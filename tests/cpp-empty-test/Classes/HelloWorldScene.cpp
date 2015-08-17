#include "HelloWorldScene.h"
#include "AppMacros.h"
#include "extensions/cocos-ext.h"
#include "editor-support/cocostudio/CocoStudio.h"

USING_NS_CC;


Scene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    //auto visibleSize = Director::getInstance()->getVisibleSize();
    //auto origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
//    auto closeItem = MenuItemImage::create(
//                                        "CloseNormal.png",
//                                        "CloseSelected.png",
//                                        CC_CALLBACK_1(HelloWorld::menuCloseCallback,this));
//    
//    closeItem->setPosition(origin + Vec2(visibleSize) - Vec2(closeItem->getContentSize() / 2));
//
//    // create menu, it's an autorelease object
//    auto menu = Menu::create(closeItem, nullptr);
//    menu->setPosition(Vec2::ZERO);
//    this->addChild(menu, 1);
//    
//    /////////////////////////////
//    // 3. add your codes below...
//
//    // add a label shows "Hello World"
//    // create and initialize a label
//    
//    auto label = LabelTTF::create("Hello World", "Arial", TITLE_FONT_SIZE);
//    
//    // position the label on the center of the screen
//    label->setPosition(origin.x + visibleSize.width/2,
//                            origin.y + visibleSize.height - label->getContentSize().height);
//
//    // add the label as a child to this layer
//    this->addChild(label, 1);

//    // add "HelloWorld" splash screen"
//    auto sprite = Sprite::create("HelloWorld.png");
//
//    // position the sprite on the center of the screen
//    sprite->setPosition(Vec2(visibleSize / 2) + origin);
//
//    // add the sprite as a child to this layer
//    this->addChild(sprite);
    
    initScene();
    scheduleUpdate();
    
    return true;
}

//void HelloWorld::initScene()
//{
//    Size visibleSize = Director::getInstance()->getVisibleSize();
//    auto vp = Camera::getDefaultViewport();
//    auto node = Node::create();
//    _headNode = node;
//    node->setPosition3D(Vec3(-1,1.6f,4));
//    scheduleUpdate();
//    
//    //use custom camera
//    {
//        _camera = Camera::createPerspective(80,visibleSize.width/visibleSize.height * 0.5,0.1f,800);
//        _camera->setCameraFlag(CameraFlag::USER1);
//        
//        _camera->setPosition3D(Vec3(-0.01,0,0));
//        //_camera->setViewport(experimental::Viewport(0,0,visibleSize.width/2, visibleSize.height));
//        _camera->setFrameBufferObject(Director::getInstance()->getDefaultFBO());
//        _camera->setViewport(experimental::Viewport(vp._left,vp._bottom, vp._width/2, vp._height));
//        node->addChild(_camera);
//        
//        _camera2 = Camera::createPerspective(80,visibleSize.width/visibleSize.height * 0.5,0.1f,800);
//        _camera2->setCameraFlag(CameraFlag::USER2);
//        
//        _camera2->setPosition3D(Vec3(0.01,0,0));
//        
//        //_camera2->setViewport(experimental::Viewport(0,0,visibleSize.width/2, visibleSize.height));
//        _camera2->setFrameBufferObject(Director::getInstance()->getDefaultFBO());
//        _camera2->setViewport(experimental::Viewport(vp._left + vp._width/2,vp._bottom, vp._width/2, vp._height));
//        node->addChild(_camera2);
//    }
//    
//    addChild(node);
//    
//    _objectNode = Node::create();
//    
//    auto player = Sprite3D::create("vr/girl.c3b");
//    player->setCameraMask(2);
//    player->setScale(0.008f);
//    
//    //    auto moveBy = MoveBy::create(2.0, Vec3(6, 0, 2));
//    auto pos = _objectNode->getPosition3D();
//    _objectNode->setPosition3D(pos + Vec3(-3,1,0));
//    //    player->runAction(
//    //                      RepeatForever::create(Sequence::createWithTwoActions(moveBy, moveBy->reverse()))
//    //    );
//    addChild(_objectNode);
//    _objectNode->addChild(player);
//    player->setCameraMask((unsigned short)CameraFlag::USER1|(unsigned short)CameraFlag::USER2);
//    
//    Terrain::DetailMap r("vr/dirt.jpg"),g("vr/Grass2.jpg"),b("vr/road.jpg"),a("vr/GreenSkin.jpg");
//    
//    Terrain::TerrainData data("vr/heightmap16.jpg","vr/alphamap.png",r,g,b,a);
//    
//    _terrain = Terrain::create(data,Terrain::CrackFixedType::SKIRT);
//    _terrain->setLODDistance(3.2f,6.4f,9.6f);
//    _terrain->setMaxDetailMapAmount(4);
//    addChild(_terrain);
//    _terrain->setCameraMask((unsigned short)CameraFlag::USER1|(unsigned short)CameraFlag::USER2);
//    _terrain->setDrawWire(false);
//    
//    // add Particle3D for test blend
//    auto rootps = PUParticleSystem3D::create("vr/Particle3D/scripts/mp_torch.pu");
//    rootps->setCameraMask((unsigned short)CameraFlag::USER1|(unsigned short)CameraFlag::USER2);
//    rootps->startParticleSystem();
//    
//    this->addChild(rootps, 0, 0);
//}

static cocos2d::Label* _movementLabel = nullptr;
static cocos2d::Mat4 _originalHeadRotation = Mat4::IDENTITY;
static cocos2d::Vec3 _originalTranslation = Vec3::ZERO;
static float _moveSpeed = 5.0;

bool HelloWorld::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *evt)
{
    //_isMoving = true;
    return true;
}

void HelloWorld::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *evt)
{
    auto delta = touch->getDelta();
    _moveSpeed += delta.x/10;
    _moveSpeed = clampf(_moveSpeed, 5, 100);
}

void HelloWorld::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *evt)
{
    //_isMoving = false;
    //MOVESCALE *= 2;
}

void HelloWorld::initScene()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto vp = Camera::getDefaultViewport();
    auto sceneNode = CSLoader::createNode("res/Scene3DNoParticle.csb");
    sceneNode->setCameraMask((unsigned short)CameraFlag::USER1, true);
    addChild(sceneNode);
    _headNode = sceneNode->getChildByTag(57);
    
    {
        _originalHeadRotation = _headNode->getNodeToParentTransform();
        _originalHeadRotation.getTranslation(&_originalTranslation);
        Quaternion q;
        _originalHeadRotation.getRotation(&q);
        Mat4::createRotation(q, &_originalHeadRotation);
        Vec3 forward(0,0,-1);
        _originalHeadRotation.transformVector(&forward);
        forward.y = 0;
        forward.normalize();
        Mat4::createLookAt(Vec3::ZERO, forward, Vec3(0,1,0), &_originalHeadRotation);
        
    }
    
    {
        _camera = Camera::createPerspective(60,visibleSize.width/visibleSize.height * 0.5,0.1f,800);
        _camera->setCameraFlag(CameraFlag::USER1);
        //
        //        _camera->setPosition3D(Vec3(-0.01,0,0));
        _camera->setFrameBufferObject(Director::getInstance()->getDefaultFBO());
        _camera->setViewport(experimental::Viewport(vp._left,vp._bottom, vp._width/2, vp._height));
        _headNode->addChild(_camera);
        
        _camera2 = Camera::createPerspective(60,visibleSize.width/visibleSize.height * 0.5,0.1f,800);
        _camera2->setCameraFlag(CameraFlag::USER1);
        //
        //        _camera->setPosition3D(Vec3(-0.01,0,0));
        _camera2->setFrameBufferObject(Director::getInstance()->getDefaultFBO());
        _camera2->setViewport(experimental::Viewport(vp._left + vp._width/2,vp._bottom, vp._width/2, vp._height));
        _headNode->addChild(_camera2);
    }
    
    //add skybox
    {
        auto textureCube = TextureCube::create("skybox/left.jpg", "skybox/right.jpg",
                                           "skybox/top.jpg", "skybox/bottom.jpg",
                                           "skybox/front.jpg", "skybox/back.jpg");
        auto skyBox = Skybox::create();
        skyBox->retain();
        
        skyBox->setTexture(textureCube);
        addChild(skyBox);
        skyBox->setCameraMask((unsigned short)CameraFlag::USER1);
    }
    
    //add label
    {
        auto label = Label::createWithSystemFont("", "Arial", 8);
        label->setAnchorPoint(Vec2(0, 1.0));
        label->setPosition(0, visibleSize.height);
        //label->setCameraMask((unsigned short)CameraFlag::USER2);
        _movementLabel = label;
        addChild(label);
        
//        auto camera = Camera::createPerspective(60,visibleSize.width/visibleSize.height * 0.5,0.1f,800);
//        camera->setCameraFlag(CameraFlag::USER2);
//        camera->setFrameBufferObject(Director::getInstance()->getDefaultFBO());
//        camera->setViewport(experimental::Viewport(vp._left,vp._bottom, vp._width/2, vp._height));
//        _headNode->addChild(camera);
        
//        auto node = Node::create();
//        node->setPosition3D(_headNode->getPosition3D());
//        node->setRotationQuat(_headNode->getRotationQuat());
//        sceneNode->addChild(node);
//        node->addChild(camera);
    }
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    CCASSERT(_headNode, "");
}

void HelloWorld::update(float delta)
{
    Mat4 transform = Director::getInstance()->getHeadTransform();
    Quaternion q;
    transform.getRotation(&q);
    q.inverse();
    //CCLOG("head rotation is %lf, %lf, %lf, %lf", q.x, q.y,q.z,q.w);
    _headNode->setRotationQuat(q);
    
    auto v = Director::getInstance()->getDeviceMove();
    auto v2 = v;
    _originalHeadRotation.transformVector(v.x, v.y, v.z, 0, &v2);
    _headNode->setPosition3D(_originalTranslation + v2 * _moveSpeed);
    v2 = _headNode->getPosition3D();
    //CCLOG("Device move is %.6f, %.6f, %.6f", v.x, v.y, v.z);
    if(_movementLabel)
    {
        auto str = StringUtils::format("Position %.4f, %.4f, %.4f, Speed %.4f", v2.x, v2.y, v2.z, _moveSpeed);
        _movementLabel->setString(str);
    }
    //_headNode->setPosition3D(_headNode->getPosition3D() + v * MOVESCALE);
    
    //add moving logic
//    if(_isMoving)
//    {
//        Mat4 headTM;
//        Mat4::createRotation(q, &headTM);
//        Vec3 toward;
//        headTM.transformVector(0, 0, -1, 0, &toward);
//        toward.y = 0;
//        toward.normalize();
//        const float MOVE_SPEED = 2.0;
//        Vec3 pos = _headNode->getPosition3D();
//        pos += toward * MOVE_SPEED;
//        _headNode->setPosition3D(pos);
//        
//    }
}

void HelloWorld::menuCloseCallback(Ref* sender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
