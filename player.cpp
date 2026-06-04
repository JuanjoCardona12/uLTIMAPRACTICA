#include "player.h"

Player::Player(int id)
    : m_id(id), m_health(MAX_HEALTH)
{}

void Player::addObstacle(Obstacle* obs)
{
    m_obstacles.append(obs);
}

// Daño directo al jugador cuando un obstáculo es destruido completamente
void Player::receiveDamage(double damage)
{
    m_health -= damage;
    if (m_health < 0.0) m_health = 0.0;
}
