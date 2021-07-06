/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
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

#include "GameScene.h"
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include <iostream>
#include <string>   
#include "external/json/document.h"
#include "external/json/error/en.h"
#include <platform/CCFileUtils.h>


USING_NS_CC;

Scene* GameScene::createScene()
{
    return GameScene::create();
}

// Выводит ошибку, если не удаётся загрузить файл
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in GameScene.cpp\n");
}

//Класс сущность
class Entity
{
private:
    std::string name = "None";
    int chance = 1;
public:
    std::string getName() 
    {
        return name;
    }
    void setName(std::string _name)
    {
        name = _name;
    }
    int getChance()
    {
        return chance;
    }
    void setChance(int _chance)
    {
        chance = _chance;
    }
};

//Класс фрукт
class Fruit : public Entity
{
private:
    int price;
public:
    int getPrice()
    {
        return price;
    }
    int* getPointerPrice() {
        return &price;
    }
    void setPrice(int _price)
    {
        price = _price;
    }
};
//Класс бомба
class Bomb : public Entity
{
private:
    int damage = 10;
public:
    int getDamage()
    {
        return damage;
    }
    int* getPointerDamage() {
        return &damage;
    }
    void setDamage(int _damage)
    {
        damage = _damage;
    }
};

Sprite* HealthBar;
Sprite* HealthBox;
Sprite* Cart;
Label* scoreLabel;
SpriteFrameCache* fruitsCache;
Size visibleSize;
Node* top;
std::vector<Fruit> fruitList;
std::vector<Bomb> bombList;
int scoreCount;
float health;
float healthFull;
bool isGameOver;
Label* gameOverLabel;
Sequence* gameOverSeq;
EventListenerMouse* listener;


bool GameScene::init()
{

    scoreCount = 0;
    health = 3.0f;
    healthFull = health;
    isGameOver = false;

    if ( !Scene::init() )
    {
        return false;
    }
    initWithPhysics();
    visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //Параметры
    int titleSize = 32;

    //Фон
    auto background = Sprite::create("BG_Sky.png");
    if (background == nullptr)
    {
        problemLoading("'BG_Sky.png'");
    }
    else {
        background->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
        background->setScale(visibleSize.width / background->getContentSize().width, visibleSize.height / background->getContentSize().height);

        this->addChild(background, -2);
    }
    //Облака
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Clouds.plist");

    for (int i = 1; i <= 12; i++) {
        auto cloud = Sprite::createWithSpriteFrameName("Cloud" + std::to_string(cocos2d::random(1, 4)) + ".png");
        cloud->setPosition(Vec2(cocos2d::random(0.0f, visibleSize.width), cocos2d::random(visibleSize.height / 2, 2 * visibleSize.width / 3)));
        cloud->setScale(cocos2d::random(1.0f, 2.5f));
        this->addChild(cloud, -1);
    }

    //Очки
    scoreLabel = Label::createWithTTF("Score: 0", "fonts/komicax.ttf", 24);
    if (scoreLabel == nullptr)
    {
        problemLoading("'fonts/komicax.ttf'");
    }
    else
    {
        scoreLabel->setAnchorPoint(Vec2(0,0));
        scoreLabel->setSystemFontSize(titleSize);
        scoreLabel->setPosition(Vec2(origin.x+16, origin.y+16));
        scoreLabel->enableShadow(Color4B::GRAY, Size(1, -1));

        this->addChild(scoreLabel, 15);
    }

    //Корзинка
    Cart = Sprite::create("Cart.png");
    if (Cart == nullptr)
    {
        problemLoading("'Cart.png'");
    }
    else
    {
        auto physicsBody = PhysicsBody::createBox(Size(Cart->getContentSize().width * 0.7f, Cart->getContentSize().height*0.45f), PhysicsMaterial(1.0f, 1.0f, 1.0f));
        physicsBody->setPositionOffset(Vec2(0, -Cart->getContentSize().height * 0.3f));
        Cart->setPhysicsBody(physicsBody);
        Cart->setPosition(origin.x + visibleSize.width / 2,origin.y+ visibleSize.height/2);
        this->addChild(Cart, 4);
        Cart->setTag(10);
        Cart->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
        Cart->getPhysicsBody()->setDynamic(false);
    }
    listener = EventListenerMouse::create();
    listener->onMouseMove = [](cocos2d::Event* event) {
        EventMouse* e = (EventMouse*)event;
        Cart->setPosition(e->getCursorX(), e->getCursorY());
        
    };
    

    //Нижний блок
    auto bottomPanel = Node::create();
    auto physicsBody = PhysicsBody::createBox(Size(visibleSize.width+1000, 32), PhysicsMaterial(1.0f, 1.0f, 1.0f));
    bottomPanel->setPhysicsBody(physicsBody);
    bottomPanel->setPosition(origin.x + visibleSize.width / 2, -128);
    this->addChild(bottomPanel, 10);
    bottomPanel->setTag(11);
    bottomPanel->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
    bottomPanel->getPhysicsBody()->setDynamic(false);



    //Фрукты
    fruitsCache = SpriteFrameCache::getInstance();
    fruitsCache->addSpriteFramesWithFile("Fruits.plist");

    std::string jsonString = CCFileUtils::sharedFileUtils()->getStringFromFile("fruitsdata.json");
    const char* jsonData = jsonString.c_str();

    rapidjson::Document document;
    document.Parse(jsonData);

    auto fruits = document["fruits"].GetArray();
    auto bombs = document["bombs"].GetArray();

    for (rapidjson::Value::ConstValueIterator itr = fruits.Begin(); itr != fruits.End(); ++itr) {
        const rapidjson::Value& fruits = *itr;
        const rapidjson::Value::ConstMemberIterator itr2 = fruits.MemberBegin();
        Fruit fruit;
        fruit.setName(itr2->name.GetString());
        fruit.setPrice(itr2->value.GetInt());
        fruitList.push_back(fruit);
    }
    for (rapidjson::Value::ConstValueIterator itr = bombs.Begin(); itr != bombs.End(); ++itr) {
        const rapidjson::Value& bombs = *itr;
        rapidjson::Value::ConstMemberIterator itr2 = bombs.MemberBegin();
        Bomb bomb;
        bomb.setName(itr2->name.GetString());
        bomb.setDamage(itr2->value.GetInt());
        bombList.push_back(bomb);
    }



    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = [&](cocos2d::PhysicsContact& contact) {
        auto nodeA = contact.getShapeA()->getBody()->getNode();
        auto nodeB = contact.getShapeB()->getBody()->getNode();

        if (nodeA && nodeB)
        {
            if (nodeA->getTag() == 10)
            {
                //Столкновение с фруктом
                if (nodeB->getTag() == 0) {
                    scoreCount += *static_cast<int*>(nodeB->getUserData());
                    if(this)this->addChild(createDigitOnPositionWithFade(nodeB->getPhysicsBody()->getPosition(), std::to_string(*static_cast<int*>(nodeB->getUserData()))), 15);
                }
                //Столкновение с бомбой
                if (nodeB->getTag() == 1) {
                    addHealth(-*static_cast<int*>(nodeB->getUserData()));
                    CheckGameOver();
                }
                nodeB->removeFromParentAndCleanup(true);
            }
            else if (nodeB->getTag() == 10)
            {
                //Столкновение с фруктом
                if (nodeA->getTag() == 0) {
                    scoreCount += *static_cast<int*>(nodeA->getUserData());
                    if (this)this->addChild(createDigitOnPositionWithFade(nodeA->getPhysicsBody()->getPosition(), std::to_string(*static_cast<int*>(nodeA->getUserData()))), 15);
                }
                //Столкновение с бомбой
                if (nodeA->getTag() == 1) {
                    addHealth(-*static_cast<int*>(nodeA->getUserData()));
                    CheckGameOver();
                }
                nodeA->removeFromParentAndCleanup(true);
            }
            else if (nodeA->getTag() == 11)
            {
                nodeB->removeFromParentAndCleanup(true);
            }
            else if (nodeB->getTag() == 11)
            {
                nodeA->removeFromParentAndCleanup(true);
            }
        }
        updateScoreCount();
        return true;
    };
    
    //Полоска жизней
    HealthBox = Sprite::create("healthbar.png");
    HealthBox->setPosition(visibleSize.width- HealthBox->getContentSize().width/2-16, HealthBox->getContentSize().height/2+16);
    this->addChild(HealthBox, 15);

    HealthBar = Sprite::create("health.png");
    HealthBar->setAnchorPoint(Vec2(0,0));
    HealthBar->setPosition(0, 0);
    HealthBox->addChild(HealthBar, 15);

    
    //Проигрыш
    gameOverLabel = Label::createWithTTF("Game Over", "fonts/komicax.ttf", 35);
    if (gameOverLabel == nullptr)
    {
        problemLoading("'fonts/komicax.ttf'");
    }
    else
    {
        gameOverLabel->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
        gameOverLabel->enableShadow(Color4B::GRAY, Size(1, -1));
        gameOverLabel->setOpacity(0.0f);
        this->addChild(gameOverLabel, 5);
    }


    top = Node::create();
    top->setPosition(visibleSize.width / 2, visibleSize.height);
    this->addChild(top);



    schedule(CC_SCHEDULE_SELECTOR(GameScene::tick), 0.2f);
    _eventDispatcher->addEventListenerWithFixedPriority(listener, 1);
    _eventDispatcher->addEventListenerWithFixedPriority(contactListener, 1);
    return true;


}

void GameScene::GameOver() {
    isGameOver = true;
    if (UserDefault::getInstance()->getIntegerForKey("highScore", 0) < scoreCount)UserDefault::getInstance()->setIntegerForKey("highScore", scoreCount);
    unschedule(CC_SCHEDULE_SELECTOR(GameScene::tick));
    auto fadeIn = FadeIn::create(1.0f);
    auto delay = DelayTime::create(2.0f);
    _eventDispatcher->removeEventListener(listener);
    
    Cart->setTag(11);
    fruitList.clear();
    bombList.clear();


    gameOverSeq = Sequence::create(fadeIn,delay, CallFunc::create([]() -> void {
        auto menuScene = HelloWorld::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5, menuScene, Color3B(0, 255, 255)));
        })
        , nullptr);
    gameOverLabel->runAction(gameOverSeq);
}


cocos2d::Label* GameScene::createDigitOnPositionWithFade(cocos2d::Vec2 pos, std::string text) {
    auto pufLabel = Label::createWithTTF(text, "fonts/komicax.ttf", 20);
    if (pufLabel == nullptr)
    {
        problemLoading("'fonts/komicax.ttf'");
    }
    else
    {
        pufLabel->setPosition(pos);
        pufLabel->enableShadow(Color4B::GRAY, Size(1, -1));
        auto moveTo = MoveBy::create(1, Vec2(20, 0));
        auto fadeOut = FadeOut::create(1.0f);
        auto spawn = Spawn::createWithTwoActions(moveTo, fadeOut);
        auto seq = Sequence::create(spawn, CallFunc::create([=]() -> void {pufLabel->removeFromParentAndCleanup(true); }) , nullptr);
        pufLabel->runAction(seq);
    }
    return pufLabel;
}


void GameScene::updateScoreCount() {
    scoreLabel->setString("Score: "+ std::to_string(scoreCount));
}

void GameScene::CheckGameOver() {
    if (health <= 0) {
        HealthBar->setScaleX(0);
        GameOver();
    }
}

void GameScene::addHealth(int data) {
        health += data;
        HealthBar->setScaleX(health/healthFull);
}


void GameScene::tick(float dt)
{
    if (!isGameOver) {
        std::string name;
        int* poiterData;
        int tag = 0;
        if (cocos2d::random(0, 10) <= 9) {
            std::vector <int>::size_type i = fruitList.size();
            int number = cocos2d::random(0, (int)fruitList.size() - 1);
            name = fruitList.at(number).getName();
            poiterData = fruitList.at(number).getPointerPrice();
        }
        else {
            int number = cocos2d::random(0, (int)bombList.size() - 1);
            name = bombList.at(number).getName();
            poiterData = bombList.at(number).getPointerDamage();
            tag = 1;
        }
        auto sprite1 = Sprite::createWithSpriteFrameName(name);
        auto physicsBody = PhysicsBody::createBox(Size(sprite1->getContentSize().width * 0.4f, sprite1->getContentSize().height * 0.4f), PhysicsMaterial(1.0f, 1.0f, 1.0f));
        physicsBody->setGravityEnable(true);
        physicsBody->setContactTestBitmask(0xFFFFFFFF);
        sprite1->setPosition(cocos2d::random(-visibleSize.width / 2 + 16, visibleSize.width / 2 - 16), 64);
        sprite1->setScale(0.4f, 0.4f);
        sprite1->setPhysicsBody(physicsBody);
        sprite1->setUserData(poiterData);
        sprite1->setTag(tag);
        top->addChild(sprite1);
    }
}


