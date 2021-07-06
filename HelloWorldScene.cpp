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

#include "HelloWorldScene.h"
#include "GameScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

bool HelloWorld::init()
{
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    //Параметры
    int titleSize = 48;
    float coefficentLogoSize;


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
        auto cloud = Sprite::createWithSpriteFrameName("Cloud"+std::to_string(cocos2d::random(1, 4))+".png");
        cloud->setPosition(Vec2(cocos2d::random(0.0f, visibleSize.width), cocos2d::random(visibleSize.height / 2, 2*visibleSize.width/3)));
        cloud->setScale(cocos2d::random(1.0f, 2.5f));
        this->addChild(cloud, -1);
    }
    
    


    //Логотип и тайтл
    auto LogoBox = Node::create();
    auto logo = Sprite::create("logo.png");
    if (logo == nullptr)
    {
        problemLoading("'logo.png'");
    }
    else
    {
        coefficentLogoSize = 2 * titleSize / logo->getContentSize().height;
        logo->setScale(coefficentLogoSize, coefficentLogoSize);
        LogoBox->addChild(logo, 1);
    }

    auto title = Label::createWithTTF("Fruit rain", "fonts/komicax.ttf", 24);
    if (title == nullptr)
    {
        problemLoading("'fonts/komicax.ttf'");
    }
    else
    {
        title->setSystemFontSize(titleSize);
        title->enableShadow(Color4B::GRAY, Size(2, -2));
        LogoBox->addChild(title, 1);
    }
    logo->setPosition(Vec2(-title->getContentSize().width/2-coefficentLogoSize*logo->getContentSize().width/2-16, 0));
    LogoBox->setPosition(Vec2(visibleSize.width/2+coefficentLogoSize * logo->getContentSize().width / 2 + 16, visibleSize.height-logo->getContentSize().height* coefficentLogoSize /2-16));
    this->addChild(LogoBox, 1);



    //Лучший результат
    int highScore = UserDefault::getInstance()->getIntegerForKey("highScore", 0);
    auto highScoreLabel = Label::createWithTTF("High Score: "+std::to_string(highScore), "fonts/komicax.ttf", titleSize/2);
    if (highScoreLabel == nullptr)
    {
        problemLoading("'fonts/komicax.ttf'");
    }
    else
    {
        highScoreLabel->enableShadow(Color4B::GRAY, Size(2, -2));
        highScoreLabel->setPosition(Vec2(visibleSize.width/2, visibleSize.height - logo->getContentSize().height * coefficentLogoSize-highScoreLabel->getContentSize().height - 32));
        this->addChild(highScoreLabel, 1);
    }

    //Меню

    //Новая игра
    auto startItem = MenuItemImage::create("BtnNormal.png", "BtnSelected.png", CC_CALLBACK_1(HelloWorld::menuStartCallback, this));

    if (startItem == nullptr || startItem->getContentSize().width <= 0 || startItem->getContentSize().height <= 0)
    {
        problemLoading("'BtnNormal.png' and 'BtnSelected.png'");
    }
    else
    {
        auto text = Label::createWithTTF("New game", "fonts/komicax.ttf", startItem->getContentSize().height-20);
        text->setPosition(Vec2(text->getContentSize().width/2+16, text->getContentSize().height / 2+5));
        text->enableShadow(Color4B::GRAY, Size(1, -1));
        startItem->addChild(text, 1);
    }
    auto closeItem = MenuItemImage::create("BtnNormal.png","BtnSelected.png",CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));


    //Выход из игры
    if (closeItem == nullptr ||  closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0)
    {
        problemLoading("'BtnNormal.png' and 'BtnSelected.png'");
    }
    else
    {
        auto text = Label::createWithTTF("Quit game", "fonts/komicax.ttf", startItem->getContentSize().height - 20);
        text->setPosition(Vec2(text->getContentSize().width / 2 + 16, text->getContentSize().height / 2 + 5));
        text->enableShadow(Color4B::GRAY, Size(1, -1));
        closeItem->addChild(text, 1);
        closeItem->setPosition(Vec2(0, -closeItem->getContentSize().height- 16));
    }

    // Добавление меню
    Vector<MenuItem*> MenuItems;
    MenuItems.pushBack(startItem);
    MenuItems.pushBack(closeItem);

    auto menu = Menu::createWithArray(MenuItems);
    menu->setPosition(Vec2(visibleSize.width/2 , visibleSize.height - logo->getContentSize().height * coefficentLogoSize - highScoreLabel->getContentSize().height - (startItem->getContentSize().height+16) - 48));
    this->addChild(menu, 1);



    return true;


}

void HelloWorld::menuStartCallback(Ref* pSender)
{
    auto dirs = Director::getInstance();
    auto gameScene = GameScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5, gameScene, Color3B(0, 255, 255)));
}
void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}
