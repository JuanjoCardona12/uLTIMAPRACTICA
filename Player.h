#pragma once

#include <QList>
#include <QString>
#include "obstacle.h"

class Player {
public:
    explicit Player(int id);

    void addObstacle(Obstacle* obs);

    int     getId()  const { return m_id; }
    QString getName() const { return QString("Jugador %1").arg(m_id + 1); }

    // Salud = suma de resistencias restantes de todos los obstáculos
    double getHealth()    const;
    // Salud máxima = suma de resistencias iniciales
    double getMaxHealth() const;
    // Vivo si queda al menos un obstáculo sin destruir
    bool   isAlive()      const;

    QList<Obstacle*>&       obstacles()       { return m_obstacles; }
    const QList<Obstacle*>& obstacles() const { return m_obstacles; }

private:
    int              m_id;
    QList<Obstacle*> m_obstacles;
};
