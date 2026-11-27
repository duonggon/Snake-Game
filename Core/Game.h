#pragma once
#include "GameState.h"
#include "Direction.h"
#include <random>
#include "Map.h"
#include <SFML/Graphics.hpp>

class Game {
public:
    Game();
    void run();
    void reset();
    void notifyAppleEaten();
    void setSlowMotion(bool isActive);
    void requestVictory();
    void requestGameOver();
    GameState getState() const;
    int getAppleCount() const;
    Map& getMap();
    const Map& getMap() const;

private:
    sf::RenderWindow m_window;
    Map m_map;
    GameState m_state = GameState::SURVIVAL;
    GameState m_previousState = GameState::SURVIVAL;
    int m_appleCount = 0;
    float m_gameTimeScale = 1.0f;
    float m_bossIncomingTimer = 0.0f;

    // Dữ liệu giả chỉ phục vụ kiểm thử Core, KHÔNG phải module Snake/Apple.
    std::vector<sf::Vector2i> m_debugSnakeCells{{6, 6}, {5, 6}, {4, 6}};
    std::vector<sf::Vector2i> m_debugAppleCells{{15, 10}};

    // Chi dung trong Core Demo, se bo khi tich hop module Snake/Apple.
    Direction m_debugDirection = Direction::RIGHT;
    Direction m_debugNextDirection = Direction::RIGHT;
    bool m_debugMoving = false;
    float m_debugMoveTimer = 0.0f;
    std::mt19937 m_debugRandom{std::random_device{}()};
    sf::Texture m_debugAppleTexture;
    bool m_hasDebugAppleTexture = false;

    void setDebugDirection(Direction direction);
    void updateDebugSnake(float gameDeltaTime);
    void spawnDebugApple();
    void resetDebugPlayer();
    void handleInput();
    void update(float realDeltaTime);
    void render();
    void togglePause();
    void enterBossIncoming();
    void updateTitle();
    void renderDebugMarkers();
};
