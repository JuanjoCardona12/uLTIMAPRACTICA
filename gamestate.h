#pragma once

enum class GameState {
    WAITING_INPUT,
    SIMULATING,
    GAME_OVER
};

enum class CollisionType {
    NONE,
    WALL,
    OBSTACLE,
    PLAYER
};
