#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Layer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init() override;  
    virtual void update(float delta) override;
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* scene();
    
    // a selector callback
    void menuCloseCallback(Ref* sender);
    
    // implement the "static node()" method manually
    CREATE_FUNC(HelloWorld);
private:
    cocos2d::Terrain* _terrain;
    cocos2d::Camera* _camera;
    cocos2d::Camera* _camera2;
    cocos2d::Node* _headNode;
    cocos2d::Node* _objectNode;
};

#endif // __HELLOWORLD_SCENE_H__
