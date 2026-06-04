#pragma once

#include <QPointF>
#include "projectile.h"
#include "obstacle.h"

class PhysicsEngine {
public:
    static constexpr double EPSILON       = 0.6;
    static constexpr double DAMAGE_FACTOR = 0.05;

    PhysicsEngine() = default;

    bool   elasticWallCollision(Projectile* p, double sceneW, double sceneH, double groundY);
    // Retorna el daño aplicado en este golpe (0 si no hubo colisión)
    double inelasticObstacleCollision(Projectile* p, Obstacle* obs);
    double calculateDamage(Projectile* p, double speed) const;

private:
    QPointF detectCollisionNormal(Projectile* p, Obstacle* obs) const;
};
