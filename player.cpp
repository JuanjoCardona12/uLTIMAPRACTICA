#include "player.h"

Player::Player(int id)
    : m_id(id)
{}

void Player::addObstacle(Obstacle* obs)
{
    m_obstacles.append(obs);
}

double Player::getHealth() const
{
    double total = 0.0;
    for (const Obstacle* obs : m_obstacles)
        total += obs->getResistance();
    return total;
}

double Player::getMaxHealth() const
{
    double total = 0.0;
    for (const Obstacle* obs : m_obstacles)
        total += obs->getMaxResistance();
    return total;
}

bool Player::isAlive() const
{
    for (const Obstacle* obs : m_obstacles) {
        if (!obs->isDestroyed()) return true;
    }
    return false;
}
