#include "physicsengine.h"
#include <cmath>
#include <algorithm>
#include <QRectF>

// =============================================================================
// COLISIÓN 1: PERFECTAMENTE ELÁSTICA con paredes (límites de la caja)
// Invierte la componente de velocidad perpendicular a la pared tocada y
// corrige la posición para que el proyectil quede estrictamente dentro
// de los límites (evita múltiples detecciones consecutivas en la misma pared).
// =============================================================================
bool PhysicsEngine::elasticWallCollision(Projectile* p, double sceneW, double sceneH)
{
    bool hit = false;
    double r = Projectile::RADIUS;

    // Pared izquierda
    if (p->getX() - r <= 0.0) {
        p->setX(r + 1.0);                      // corrección de posición
        p->setVx(std::abs(p->getVx()));         // dirección positiva
        hit = true;
    }
    // Pared derecha
    else if (p->getX() + r >= sceneW) {
        p->setX(sceneW - r - 1.0);
        p->setVx(-std::abs(p->getVx()));
        hit = true;
    }

    // Pared superior
    if (p->getY() - r <= 0.0) {
        p->setY(r + 1.0);
        p->setVy(std::abs(p->getVy()));
        hit = true;
    }
    // Pared inferior (suelo)
    else if (p->getY() + r >= sceneH) {
        p->setY(sceneH - r - 1.0);
        p->setVy(-std::abs(p->getVy()));
        hit = true;
    }

    return hit;
}

// =============================================================================
// COLISIÓN 2: INELÁSTICA con obstáculos
//
//   v'_perp = -ε · v_perp    (pérdida de energía controlada por ε)
//   v'_par  =  v_par         (componente tangencial sin cambio)
//
// Fórmula de daño:
//   daño = DAMAGE_FACTOR × masa_proyectil × |v_perp_antes_del_impacto|
//
// Corrección de posición: tras el rebote el proyectil se desplaza fuera
// del rect expandido en la dirección de la normal, eliminando el problema
// de múltiples detecciones por el mismo contacto (tunneling inverso).
//
// Retorna el daño aplicado, o 0.0 si no hubo colisión.
// =============================================================================
double PhysicsEngine::inelasticObstacleCollision(Projectile* p, Obstacle* obs)
{
    if (obs->isDestroyed() || !obs->isVisible()) return 0.0;

    double r = Projectile::RADIUS;
    QRectF obsBounds = obs->rect();

    // Rect expandido: el centro del proyectil debe quedar dentro para detectar contacto
    QRectF expanded(obsBounds.x()     - r,
                    obsBounds.y()     - r,
                    obsBounds.width() + 2.0 * r,
                    obsBounds.height()+ 2.0 * r);

    if (!expanded.contains(QPointF(p->getX(), p->getY()))) return 0.0;

    // Normal de la cara impactada
    QPointF normal = detectCollisionNormal(p, obs);
    if (normal.isNull()) return 0.0;

    double nx = normal.x();
    double ny = normal.y();

    double vx = p->getVx();
    double vy = p->getVy();

    // v · n: positivo → proyectil alejándose (ya rebotó), ignorar
    double vDotN = vx * nx + vy * ny;
    if (vDotN >= 0.0) return 0.0;

    // Descomposición en componentes perpendicular y paralela
    double vPerpX = vDotN * nx;
    double vPerpY = vDotN * ny;
    double vParX  = vx - vPerpX;
    double vParY  = vy - vPerpY;

    // Magnitud de la velocidad perpendicular (usada para el daño)
    double speedPerp = std::sqrt(vPerpX * vPerpX + vPerpY * vPerpY);

    // Nueva velocidad: v' = v_par - ε · v_perp
    p->setVx(vParX - EPSILON * vPerpX);
    p->setVy(vParY - EPSILON * vPerpY);

    // ── Corrección de posición ──────────────────────────────────────────────
    // Calculamos cuánto hay que desplazar el centro del proyectil para que
    // quede justo en el borde del rect expandido, en la dirección de la normal.
    // Esto evita que el frame siguiente vuelva a detectar el mismo contacto.
    double penetrationX = 0.0, penetrationY = 0.0;
    if (nx < 0.0) penetrationX = p->getX() - expanded.left();   // izquierda del rect
    if (nx > 0.0) penetrationX = p->getX() - expanded.right();  // derecha
    if (ny < 0.0) penetrationY = p->getY() - expanded.top();
    if (ny > 0.0) penetrationY = p->getY() - expanded.bottom();

    // Mover el proyectil 2 px más allá del borde (margen de seguridad)
    if (nx != 0.0) p->setX(p->getX() - penetrationX - nx * 2.0);
    if (ny != 0.0) p->setY(p->getY() - penetrationY - ny * 2.0);

    // Calcular y aplicar daño al obstáculo
    double damage = calculateDamage(p, speedPerp);
    obs->applyDamage(damage);

    return damage;
}

// =============================================================================
// Daño = DAMAGE_FACTOR × masa × |v_perp_impacto|
// =============================================================================
double PhysicsEngine::calculateDamage(Projectile* p, double speed) const
{
    return DAMAGE_FACTOR * p->getMass() * speed;
}

// =============================================================================
// Normal de la cara del obstáculo más cercana al proyectil.
// Se calcula la penetración en cada eje y se elige la menor (cara más próxima).
// =============================================================================
QPointF PhysicsEngine::detectCollisionNormal(Projectile* p, Obstacle* obs) const
{
    QRectF rect = obs->rect();
    double px = p->getX();
    double py = p->getY();

    double dLeft   = std::abs(px - rect.left());
    double dRight  = std::abs(px - rect.right());
    double dTop    = std::abs(py - rect.top());
    double dBottom = std::abs(py - rect.bottom());

    double minDist = std::min({dLeft, dRight, dTop, dBottom});

    if (minDist == dLeft)   return QPointF(-1.0,  0.0);
    if (minDist == dRight)  return QPointF( 1.0,  0.0);
    if (minDist == dTop)    return QPointF( 0.0, -1.0);
    return                         QPointF( 0.0,  1.0);
}
