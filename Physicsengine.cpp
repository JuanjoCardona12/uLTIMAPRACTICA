#include "physicsengine.h"
#include <cmath>
#include <algorithm>
#include <QRectF>

// =============================================================================
// COLISIÓN 1: PERFECTAMENTE ELÁSTICA con paredes
// Cuando el proyectil toca una pared se invierte la componente perpendicular
// a esa pared, conservando la energía cinética total.
// =============================================================================
bool PhysicsEngine::elasticWallCollision(Projectile* p, double sceneW, double sceneH)
{
    bool hit = false;
    double r = Projectile::RADIUS;

    // Pared izquierda
    if (p->getX() - r <= 0) {
        p->setVx(std::abs(p->getVx()));   // forzar dirección positiva
        hit = true;
    }
    // Pared derecha
    else if (p->getX() + r >= sceneW) {
        p->setVx(-std::abs(p->getVx()));
        hit = true;
    }

    // Pared superior
    if (p->getY() - r <= 0) {
        p->setVy(std::abs(p->getVy()));
        hit = true;
    }
    // Pared inferior (suelo)
    else if (p->getY() + r >= sceneH) {
        p->setVy(-std::abs(p->getVy()));
        hit = true;
    }

    return hit;
}

// =============================================================================
// COLISIÓN 2: INELÁSTICA con obstáculos
// Se aplica:  v'_perp = -epsilon * v_perp   (pérdida de energía)
//             v'_par  = v_par               (componente paralela intacta)
//
// El daño se calcula con la velocidad perpendicular al impacto:
//   Daño = DAMAGE_FACTOR * masa * |v_perp_antes|
// =============================================================================
bool PhysicsEngine::inelasticObstacleCollision(Projectile* p, Obstacle* obs)
{
    if (obs->isDestroyed() || !obs->isVisible()) return false;

    double r = Projectile::RADIUS;
    QRectF obsBounds = obs->rect();

    // Expandir el rect del obstáculo por el radio del proyectil para detección
    QRectF expanded(obsBounds.x()      - r,
                    obsBounds.y()      - r,
                    obsBounds.width()  + 2 * r,
                    obsBounds.height() + 2 * r);

    if (!expanded.contains(QPointF(p->getX(), p->getY()))) return false;

    // Determinar la normal de la cara impactada
    QPointF normal = detectCollisionNormal(p, obs);
    if (normal.isNull()) return false;

    // nx, ny = componentes del vector normal unitario
    double nx = normal.x();
    double ny = normal.y();

    // Velocidad actual
    double vx = p->getVx();
    double vy = p->getVy();

    // Componente perpendicular: v_perp = (v · n) * n
    double vDotN = vx * nx + vy * ny;

    // Solo procesar si el proyectil se acerca al obstáculo (v·n < 0)
    if (vDotN >= 0) return false;

    double vPerpX = vDotN * nx;
    double vPerpY = vDotN * ny;

    // Componente paralela: v_par = v - v_perp
    double vParX = vx - vPerpX;
    double vParY = vy - vPerpY;

    // Velocidad perpendicular de impacto (módulo)
    double speedPerp = std::sqrt(vPerpX * vPerpX + vPerpY * vPerpY);

    // Aplicar coeficiente de restitución a componente perpendicular
    // v'_perp = -epsilon * v_perp
    double newVx = vParX + (-EPSILON * vPerpX);
    double newVy = vParY + (-EPSILON * vPerpY);

    p->setVx(newVx);
    p->setVy(newVy);

    // Calcular y aplicar daño al obstáculo
    double damage = calculateDamage(p, speedPerp);
    obs->applyDamage(damage);

    return true;
}

// =============================================================================
// Calcula el daño al obstáculo
// Daño = DAMAGE_FACTOR * masa_proyectil * |v_perp_impacto|
// =============================================================================
double PhysicsEngine::calculateDamage(Projectile* p, double speed) const
{
    return DAMAGE_FACTOR * p->getMass() * speed;
}

// =============================================================================
// Determina la normal de la cara del obstáculo más cercana al proyectil.
// Se analiza qué cara (top, bottom, left, right) es la más probable de impacto
// basándose en la posición relativa del proyectil.
// =============================================================================
QPointF PhysicsEngine::detectCollisionNormal(Projectile* p, Obstacle* obs) const
{
    QRectF r = obs->rect();
    double px = p->getX();
    double py = p->getY();

    // Distancias a cada cara
    double dLeft   = std::abs(px - r.left());
    double dRight  = std::abs(px - r.right());
    double dTop    = std::abs(py - r.top());
    double dBottom = std::abs(py - r.bottom());

    double minDist = std::min({dLeft, dRight, dTop, dBottom});

    // La cara más cercana determina la normal
    if (minDist == dLeft)   return QPointF(-1,  0);  // pared izquierda → normal hacia izquierda
    if (minDist == dRight)  return QPointF( 1,  0);  // pared derecha
    if (minDist == dTop)    return QPointF( 0, -1);  // pared superior
    return QPointF( 0,  1);  // pared inferior
}
