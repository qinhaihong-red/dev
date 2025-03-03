// core/application.cpp  
#include "application.h"  
#include <iostream>  

Application::Application()   
    : window(sf::VideoMode(1024, 768), "Slay the Spire Clone", sf::Style::Close) {  
    
    // 设置窗口属性  
    window.setFramerateLimit(60);  
    window.setVerticalSyncEnabled(true);  
    
    // 初始化游戏系统  
    initializeSystems();  
}  

Application::~Application() {  
    // 系统会通过智能指针自动清理  
}  

void Application::initializeSystems() {  
    // 初始化核心系统  
    eventSystem = std::make_unique<EventSystem>();  
    database = std::make_unique<Database>("data/game.db");  
    database->initialize();  
    
    // 初始化游戏系统  
    gameStateSystem = std::make_unique<GameStateSystem>(registry, *eventSystem);  
    combatSystem = std::make_unique<CombatSystem>(registry, *eventSystem);  
    cardSystem = std::make_unique<CardSystem>(registry, *eventSystem, *database);  
    enemyAISystem = std::make_unique<EnemyAISystem>(registry, *eventSystem);  
    mapSystem = std::make_unique<MapSystem>(registry, *eventSystem, *database);  
    relicSystem = std::make_unique<RelicSystem>(registry, *eventSystem, *database);  
    saveSystem = std::make_unique<SaveSystem>(registry, *database);  
    
    // 初始化UI系统  
    uiManager = std::make_unique<UIManager>(registry, *gameStateSystem, *eventSystem, window);  
    uiManager->initialize();  
    
    std::cout << "所有系统初始化完成" << std::endl;  
}  

void Application::processInput() {  
    sf::Event event;  
    while (window.pollEvent(event)) {  
        // 处理窗口关闭事件  
        if (event.type == sf::Event::Closed) {  
            window.close();  
        }  
        
        // 处理键盘按键事件  
        if (event.type == sf::Event::KeyPressed) {  
            if (event.key.code == sf::Keyboard::Escape) {  
                // 根据游戏状态不同，ESC键可能有不同行为  
                GameState currentState = gameStateSystem->getCurrentState();  
                if (currentState == GameState::MAIN_MENU) {  
                    window.close();  // 在主菜单按ESC退出游戏  
                }  
                else if (currentState == GameState::COMBAT) {  
                    // 在战斗中可能显示暂停菜单  
                }  
                else {  
                    // 在其他状态可能返回上一状态  
                    gameStateSystem->returnToPreviousState();  
                }  
            }  
        }  
        
        // 将事件传递给UI管理器处理  
        bool eventHandledByUI = uiManager->handleEvent(event);  
        if (eventHandledByUI) {  
            continue;  // 如果UI处理了事件，不再传递给其他系统  
        }  
        
        // 根据游戏状态分发事件到相应系统  
        GameState currentState = gameStateSystem->getCurrentState();  
        if (currentState == GameState::COMBAT) {  
            // 传递给战斗系统处理  
            combatSystem->handleInput(event);  
        }  
        else if (currentState == GameState::MAP) {  
            // 传递给地图系统处理  
            mapSystem->handleInput(event);  
        }  
        // 根据需要添加其他状态的输入处理...  
    }  
}  

void Application::update(float deltaTime) {  
    // 获取当前游戏状态  
    GameState currentState = gameStateSystem->getCurrentState();  
    
    // 根据游戏状态更新相应系统  
    switch (currentState) {  
        case GameState::COMBAT:  
            combatSystem->update(deltaTime);  
            cardSystem->update(deltaTime);  
            enemyAISystem->update(deltaTime);  
            break;  
            
        case GameState::MAP:  
            mapSystem->update(deltaTime);  
            break;  
            
        case GameState::SHOP:  
        case GameState::REST:  
        case GameState::EVENT:  
        case GameState::REWARD:  
            // 更新相应系统  
            break;  
            
        default:  
            // 其他状态可能不需要特定的逻辑更新  
            break;  
    }  
    
    // 通用系统更新  
    relicSystem->update(deltaTime);  
    
    // 更新UI  
    uiManager->update(deltaTime);  
}  

void Application::render() {  
    // 清除窗口  
    window.clear(sf::Color(30, 30, 30));  
    
    // 获取当前游戏状态  
    GameState currentState = gameStateSystem->getCurrentState();  
    
    // 根据游戏状态渲染相应内容  
    switch (currentState) {  
        case GameState::COMBAT:  
            combatSystem->render(window);  
            break;  
            
        case GameState::MAP:  
            mapSystem->render(window);  
            break;  
            
        case GameState::SHOP:  
        case GameState::REST:  
        case GameState::EVENT:  
        case GameState::REWARD:  
            // 渲染相应画面  
            break;  
            
        default:  
            // 其他状态可能不需要特定的渲染  
            break;  
    }  
    
    // 渲染UI（始终在最上层）  
    uiManager->render();  
    
    // 刷新窗口  
    window.display();  
}  

void Application::run() {  
    std::cout << "游戏启动" << std::endl;  
    
    // 游戏主循环  
    while (window.isOpen()) {  
        // 计算帧间隔时间  
        float deltaTime = gameClock.restart().asSeconds();  
        
        // 处理输入  
        processInput();  
        
        // 更新游戏逻辑  
        update(deltaTime);  
        
        // 渲染游戏画面  
        render();  
    }  
    
    std::cout << "游戏结束" << std::endl;  
}