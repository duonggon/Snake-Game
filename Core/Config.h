#pragma once

namespace config {
    inline constexpr int MAP_WIDTH = 24;
    inline constexpr int MAP_HEIGHT = 18;
    inline constexpr int TILE_SIZE = 40;
    inline constexpr int HUD_HEIGHT = 80;
    inline constexpr unsigned int WINDOW_WIDTH = MAP_WIDTH * TILE_SIZE;
    inline constexpr unsigned int WINDOW_HEIGHT = MAP_HEIGHT * TILE_SIZE + HUD_HEIGHT;
    inline constexpr unsigned int FRAME_LIMIT = 60;
    inline constexpr float MAX_FRAME_DELTA = 0.1f;

    inline constexpr float DEBUG_SNAKE_STEP_INTERVAL = 0.16f;
    inline constexpr int WALL_PAIR_SIZE = 2;
    inline constexpr int SNAKE_INITIAL_LENGTH = 3;
    inline constexpr int SNAKE_MAX_LENGTH = 12;
    inline constexpr int SNAKE_MAX_LIVES = 3;
    inline constexpr float INVINCIBILITY_DURATION = 1.5f;
    inline constexpr int APPLES_TO_BOSS = 10;
    inline constexpr float HEALTH_APPLE_CHANCE = 0.05f;
    inline constexpr float ENERGY_APPLE_CHANCE = 0.10f;

    inline constexpr int MAX_WALL_COUNT = 48;
    inline constexpr int WALL_SAFE_RADIUS = 2;
    inline constexpr float WALL_WARNING_DURATION = 0.4f;
    inline constexpr float WALL_INTERVAL_EARLY = 2.0f;
    inline constexpr float WALL_INTERVAL_MID = 1.5f;
    inline constexpr float WALL_INTERVAL_LATE = 1.0f;
    inline constexpr float WALL_INTERVAL_FINAL = 0.7f;
    inline constexpr float BOSS_INCOMING_DURATION = 1.5f;

    inline constexpr int BLAST_RADIUS = 1;
    inline constexpr int MAX_ENERGY = 100;
    inline constexpr int NORMAL_APPLE_ENERGY = 20;
    inline constexpr int ENERGY_APPLE_ENERGY = 60;
    inline constexpr int BOSS_MAX_HEALTH = 100;
    inline constexpr int BOSS_SIZE = 2;
    inline constexpr int BOSS_TELEPORT_SAFE_RADIUS = 3;
    inline constexpr float BOSS_WARNING_DURATION = 0.8f;
    inline constexpr float BOSS_ENRAGED_WARNING_DURATION = 0.6f;
    inline constexpr float BOSS_SPEED = 4.0f;
    inline constexpr float BOSS_ENRAGED_SPEED = 5.0f;
    inline constexpr int BOSS_ROLL_DISTANCE = 6;
    inline constexpr int BOSS_ENRAGED_THRESHOLD = 50;
    inline constexpr int INITIAL_AMMO = 3;
    inline constexpr int MAX_AMMO = 5;
    inline constexpr int NORMAL_APPLE_AMMO = 1;
    inline constexpr int ENERGY_APPLE_AMMO = 3;
    inline constexpr int SHOT_DAMAGE = 15;
    inline constexpr float AIM_RADIUS = 5.0f;
    inline constexpr float AIM_DURATION = 3.0f;
    inline constexpr float SLOW_MOTION_SCALE = 0.3f;
}
