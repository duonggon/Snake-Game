#include "Game.h"
#include "Config.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>

namespace {
    sf::Vector2i directionOffset(Direction direction) {
        switch (direction) {
            case Direction::UP: return {0, -1};
            case Direction::DOWN: return {0, 1};
            case Direction::LEFT: return {-1, 0};
            case Direction::RIGHT: return {1, 0};
        }
        return {1, 0};
    }

    sf::Vector2f cellCenter(sf::Vector2i cell) {
        return {(cell.x + 0.5f) * config::TILE_SIZE,
                (cell.y + 0.5f) * config::TILE_SIZE};
    }

    void drawDisk(sf::RenderWindow& window, sf::Vector2f center,
                  float radius, sf::Color color) {
        sf::CircleShape shape(radius, 32);
        shape.setOrigin({radius, radius});
        shape.setPosition(center);
        shape.setFillColor(color);
        window.draw(shape);
    }

    // Than ran la cac doan noi lien mach tren luoi, khong phai anh ran cuon co dinh.
    void drawTube(sf::RenderWindow& window, const std::vector<sf::Vector2i>& cells,
                  float radius, sf::Vector2f offset, sf::Color color) {
        for (std::size_t i = 1; i < cells.size(); ++i) {
            const auto a = cellCenter(cells[i - 1]) + offset;
            const auto b = cellCenter(cells[i]) + offset;
            sf::RectangleShape link({std::abs(a.x - b.x) + (a.x == b.x ? radius * 2 : 0),
                                     std::abs(a.y - b.y) + (a.y == b.y ? radius * 2 : 0)});
            link.setPosition({std::min(a.x, b.x) - (a.x == b.x ? radius : 0),
                              std::min(a.y, b.y) - (a.y == b.y ? radius : 0)});
            link.setFillColor(color);
            window.draw(link);
        }
        for (const auto cell : cells) drawDisk(window, cellCenter(cell) + offset, radius, color);
    }

    const char* getStateName(GameState state) {
        switch (state) {
            case GameState::MAIN_MENU: return "MAIN MENU";
            case GameState::INSTRUCTIONS: return "INSTRUCTIONS";
            case GameState::COUNTDOWN: return "COUNTDOWN";
            case GameState::SURVIVAL: return "SURVIVAL";
            case GameState::BOSS_INCOMING: return "BOSS INCOMING";
            case GameState::BOSS_FIGHT: return "BOSS FIGHT";
            case GameState::PAUSED: return "PAUSED";
            case GameState::VICTORY: return "VICTORY";
            case GameState::GAME_OVER: return "GAME OVER";
        }
        return "UNKNOWN";
    }
}

Game::Game()
    : m_window(sf::VideoMode({config::WINDOW_WIDTH, config::WINDOW_HEIGHT}),
               "Snake Breaker - Core Demo", sf::Style::Titlebar | sf::Style::Close) {
    m_window.setFramerateLimit(config::FRAME_LIMIT);
    m_window.setKeyRepeatEnabled(false);
    m_hasDebugAppleTexture = m_debugAppleTexture.loadFromFile("assets/textures/apple.png");
#ifdef SNAKEBREAKER_ASSET_DIR
    if (!m_hasDebugAppleTexture)
        m_hasDebugAppleTexture = m_debugAppleTexture.loadFromFile(
            std::string(SNAKEBREAKER_ASSET_DIR) + "/textures/apple.png");
#endif
    if (m_hasDebugAppleTexture) m_debugAppleTexture.setSmooth(true);
    updateTitle();
}

void Game::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        const float realDeltaTime = clock.restart().asSeconds();
        handleInput();
        if (!m_window.isOpen()) break;
        update(realDeltaTime);
        render();
    }
}

void Game::handleInput() {
    // API sự kiện SFML 3: pollEvent trả về std::optional.
    while (const auto event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) m_window.close();
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            switch (key->code) {
                case sf::Keyboard::Key::Escape: m_window.close(); break;
                case sf::Keyboard::Key::P: togglePause(); break;
                case sf::Keyboard::Key::R: reset(); break;
                case sf::Keyboard::Key::W:
                case sf::Keyboard::Key::Up: setDebugDirection(Direction::UP); break;
                case sf::Keyboard::Key::S:
                case sf::Keyboard::Key::Down: setDebugDirection(Direction::DOWN); break;
                case sf::Keyboard::Key::A:
                case sf::Keyboard::Key::Left: setDebugDirection(Direction::LEFT); break;
                case sf::Keyboard::Key::D:
                case sf::Keyboard::Key::Right: setDebugDirection(Direction::RIGHT); break;
                // Phím N chỉ giả lập sự kiện ăn táo.
                case sf::Keyboard::Key::N: notifyAppleEaten(); break;
                default: break;
            }
        }
        // Chuột trái chỉ kiểm thử API xóa tường, không phải kỹ năng blast.
        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button == sf::Mouse::Button::Left && m_state == GameState::SURVIVAL &&
                mouse->position.x >= 0 && mouse->position.y >= 0) {
                m_map.destroyWall({mouse->position.x / config::TILE_SIZE,
                                   mouse->position.y / config::TILE_SIZE});
                updateTitle();
            }
        }
    }
}

void Game::update(float realDeltaTime) {
    const float gameDeltaTime = std::min(realDeltaTime, config::MAX_FRAME_DELTA) * m_gameTimeScale;
    if (m_state == GameState::SURVIVAL || m_state == GameState::BOSS_FIGHT)
        updateDebugSnake(gameDeltaTime);
    if (m_state == GameState::SURVIVAL) {
        const int previousCount = m_map.getDestructibleWallCount();
        m_map.update(gameDeltaTime, m_appleCount, m_debugSnakeCells, m_debugAppleCells);
        if (previousCount != m_map.getDestructibleWallCount()) updateTitle();
    } else if (m_state == GameState::BOSS_INCOMING) {
        m_bossIncomingTimer += realDeltaTime;
        if (m_bossIncomingTimer >= config::BOSS_INCOMING_DURATION) {
            m_state = GameState::BOSS_FIGHT;
            updateTitle();
            // TODO: Khởi tạo Boss, cấp đạn thông qua module Player khi tích hợp.
        }
    } else if (m_state == GameState::BOSS_FIGHT) {
        // TODO: Điều phối Player, Boss, Combat, UI khi có giao diện module.
        // Các module mô phỏng sử dụng gameDeltaTime, bộ đếm ngắm dùng realDeltaTime.
    }
}

void Game::render() {
    m_window.clear(sf::Color(33, 66, 29));
    m_map.render(m_window);
    renderDebugMarkers();
    // Khoảng trống 80px phía dưới dành cho HUD của thành viên 5.
    m_window.display();
}

void Game::setDebugDirection(Direction direction) {
    if (m_state != GameState::SURVIVAL && m_state != GameState::BOSS_FIGHT) return;
    const auto current = directionOffset(m_debugDirection);
    const auto requested = directionOffset(direction);
    // Kiem tra huong da di thuc su, tranh bam 2 phim trong cung mot buoc de quay nguoc.
    if (current.x + requested.x == 0 && current.y + requested.y == 0) return;
    m_debugNextDirection = direction;
    if (!m_debugMoving) m_debugMoveTimer = 0.0f;
    m_debugMoving = true;
}

void Game::updateDebugSnake(float gameDeltaTime) {
    if (!m_debugMoving) return;
    m_debugMoveTimer += gameDeltaTime;
    while (m_debugMoveTimer >= config::DEBUG_SNAKE_STEP_INTERVAL) {
        m_debugMoveTimer -= config::DEBUG_SNAKE_STEP_INTERVAL;
        const auto next = m_debugSnakeCells.front() + directionOffset(m_debugNextDirection);
        const bool eatsApple = std::find(m_debugAppleCells.begin(), m_debugAppleCells.end(), next)
                               != m_debugAppleCells.end();
        const bool grows = eatsApple && m_debugSnakeCells.size() < config::SNAKE_MAX_LENGTH;
        // Duoc buoc vao duoi neu duoi se roi di trong buoc nay.
        const auto bodyEnd = grows ? m_debugSnakeCells.end() : m_debugSnakeCells.end() - 1;
        if (m_map.isBlocked(next) || std::find(m_debugSnakeCells.begin(), bodyEnd, next) != bodyEnd) {
            // Core test: dung ran de co the tiep tuc quan sat tuong; khong xu ly mang song.
            m_debugMoving = false;
            m_debugMoveTimer = 0.0f;
            m_debugNextDirection = m_debugDirection;
            return;
        }
        m_debugDirection = m_debugNextDirection;
        if (!grows) m_debugSnakeCells.pop_back();
        m_debugSnakeCells.insert(m_debugSnakeCells.begin(), next);
        if (eatsApple) {
            m_debugAppleCells.clear();
            notifyAppleEaten();
            spawnDebugApple();
            if (m_state == GameState::BOSS_INCOMING) return;
        }
    }
}

void Game::spawnDebugApple() {
    std::vector<sf::Vector2i> candidates;
    const auto pending = m_map.getPendingWallPair();
    for (int y = 1; y < config::MAP_HEIGHT - 1; ++y) {
        for (int x = 1; x < config::MAP_WIDTH - 1; ++x) {
            const sf::Vector2i cell{x, y};
            if (m_map.isBlocked(cell)) continue;
            if (std::find(m_debugSnakeCells.begin(), m_debugSnakeCells.end(), cell)
                != m_debugSnakeCells.end()) continue;
            if (pending && std::find(pending->begin(), pending->end(), cell) != pending->end()) continue;
            candidates.push_back(cell);
        }
    }
    if (candidates.empty()) return;
    std::uniform_int_distribution<std::size_t> distribution(0, candidates.size() - 1);
    m_debugAppleCells = {candidates[distribution(m_debugRandom)]};
}

void Game::resetDebugPlayer() {
    m_debugSnakeCells = {{6, 6}, {5, 6}, {4, 6}};
    m_debugAppleCells = {{15, 10}};
    m_debugDirection = Direction::RIGHT;
    m_debugNextDirection = Direction::RIGHT;
    m_debugMoving = false;
    m_debugMoveTimer = 0.0f;
}

void Game::renderDebugMarkers() {
    // Than ran bong xanh voi vien dam, than sang va vet phan quang.
    drawTube(m_window, m_debugSnakeCells, 16.0f, {1, 3}, sf::Color(35, 92, 50, 105));
    drawTube(m_window, m_debugSnakeCells, 15.0f, {0, 0}, sf::Color(10, 64, 176));
    drawTube(m_window, m_debugSnakeCells, 13.5f, {0, -1}, sf::Color(14, 117, 236));
    drawTube(m_window, m_debugSnakeCells, 8.0f, {0, -4}, sf::Color(24, 155, 250));
    drawTube(m_window, m_debugSnakeCells, 3.0f, {0, -8}, sf::Color(94, 206, 255));

    const auto head = cellCenter(m_debugSnakeCells.front());
    const auto offset = directionOffset(m_debugDirection);
    const sf::Vector2f forward{float(offset.x), float(offset.y)};
    const sf::Vector2f side{-forward.y, forward.x};
    drawDisk(m_window, head, 18, sf::Color(8, 64, 173));
    drawDisk(m_window, head + sf::Vector2f{0, -1}, 16.5f, sf::Color(15, 137, 242));
    drawDisk(m_window, head + sf::Vector2f{-5, -8}, 5.5f, sf::Color(88, 199, 255));
    for (const float sign : {-1.0f, 1.0f}) {
        const auto eye = head + forward * 5.0f + side * (sign * 7.0f);
        drawDisk(m_window, eye, 6.8f, sf::Color(9, 79, 179));
        drawDisk(m_window, eye, 6.0f, sf::Color(250, 253, 255));
        const auto pupil = eye + forward * 2.0f;
        drawDisk(m_window, pupil, 3.5f, sf::Color(7, 35, 81));
        drawDisk(m_window, pupil + sf::Vector2f{-1, -1}, 1.2f, sf::Color::White);
        drawDisk(m_window, head + forward * 13.0f + side * (sign * 2.3f), 1.1f, sf::Color(5, 65, 145));
    }

    for (const auto cell : m_debugAppleCells) {
        const auto center = cellCenter(cell);
        drawDisk(m_window, center + sf::Vector2f{1, 3}, 15, sf::Color(40, 90, 35, 100));
        if (m_hasDebugAppleTexture) {
            sf::Sprite apple(m_debugAppleTexture);
            const auto size = m_debugAppleTexture.getSize();
            const float scale = 36.0f / float(std::max(size.x, size.y));
            apple.setOrigin({size.x / 2.0f, size.y / 2.0f});
            apple.setPosition(center);
            apple.setScale({scale, scale});
            m_window.draw(apple);
        } else {
            // Du phong khi chua copy assets.
            drawDisk(m_window, center + sf::Vector2f{0, 3}, 15, sf::Color(190, 15, 30));
            drawDisk(m_window, center + sf::Vector2f{-1, 1}, 13, sf::Color(248, 48, 45));
            drawDisk(m_window, center + sf::Vector2f{-5, -4}, 4, sf::Color(255, 160, 120));
            sf::RectangleShape stem({4, 8});
            stem.setPosition(center + sf::Vector2f{-2, -18});
            stem.setFillColor(sf::Color(108, 59, 26));
            m_window.draw(stem);
            sf::CircleShape leaf(5);
            leaf.setScale({1.5f, 0.65f});
            leaf.setPosition(center + sf::Vector2f{1, -17});
            leaf.setFillColor(sf::Color(55, 172, 36));
            m_window.draw(leaf);
        }
    }
}

void Game::notifyAppleEaten() {
    if (m_state != GameState::SURVIVAL && m_state != GameState::BOSS_FIGHT) return;
    ++m_appleCount;
    if (m_state == GameState::SURVIVAL && m_appleCount >= config::APPLES_TO_BOSS) enterBossIncoming();
    updateTitle();
}

void Game::enterBossIncoming() {
    m_map.stopSpawning();
    m_map.clearDestructibleWalls();
    m_gameTimeScale = 1.0f;
    m_bossIncomingTimer = 0.0f;
    m_state = GameState::BOSS_INCOMING;
}

void Game::togglePause() {
    if (m_state == GameState::PAUSED) {
        m_state = m_previousState;
    } else if (m_state == GameState::SURVIVAL || m_state == GameState::BOSS_INCOMING ||
               m_state == GameState::BOSS_FIGHT) {
        m_previousState = m_state;
        m_state = GameState::PAUSED;
    }
    updateTitle();
}

void Game::reset() {
    m_map.reset();
    resetDebugPlayer();
    m_appleCount = 0;
    m_gameTimeScale = 1.0f;
    m_bossIncomingTimer = 0.0f;
    m_state = GameState::SURVIVAL;
    m_previousState = GameState::SURVIVAL;
    updateTitle();
}

void Game::setSlowMotion(bool isActive) {
    m_gameTimeScale = isActive && m_state == GameState::BOSS_FIGHT ? config::SLOW_MOTION_SCALE : 1.0f;
}

void Game::requestVictory() {
    if (m_state != GameState::BOSS_FIGHT) return;
    m_state = GameState::VICTORY;
    m_gameTimeScale = 1.0f;
    updateTitle();
}

void Game::requestGameOver() {
    if (m_state != GameState::SURVIVAL && m_state != GameState::BOSS_FIGHT) return;
    m_map.stopSpawning();
    m_state = GameState::GAME_OVER;
    m_gameTimeScale = 1.0f;
    updateTitle();
}

GameState Game::getState() const { return m_state; }
int Game::getAppleCount() const { return m_appleCount; }
Map& Game::getMap() { return m_map; }
const Map& Game::getMap() const { return m_map; }

void Game::updateTitle() {
    m_window.setTitle(std::string("Core Demo | ") + getStateName(m_state) +
                      " | Apples: " + std::to_string(m_appleCount) +
                      " | Walls: " + std::to_string(m_map.getDestructibleWallCount()) +
                      " | WASD: move  N: test apple  P: pause  R: reset");
}
