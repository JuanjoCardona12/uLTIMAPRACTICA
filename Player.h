#pragma once

#include <QList>
#include <QString>
#include "obstacle.h"

class Player {
public:
    static constexpr double MAX_HEALTH = 300.0;

    explicit Player(int id);

    void addObstacle(Obstacle* obs);
    void receiveDamage(double damage);

    int     getId()     const { return m_id; }
    double  getHealth() const { return m_health; }
    bool    isAlive()   const { return m_health > 0.0; }
    QString getName()   const { return QString("Jugador %1").arg(m_id + 1); }

    QList<Obstacle*>& obstacles() { return m_obstacles; }

private:
    int    m_id;
    double m_health;
    QList<Obstacle*> m_obstacles;
};
