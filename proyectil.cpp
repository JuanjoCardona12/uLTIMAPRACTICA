#include "projectile.h"
#include <cmath>
#include <QPen>
#include <QBrush>

Projectile::Projectile(double x, double y, double vx, double vy, double mass, int owner)
    : QGraphicsEllipseItem(-RADIUS, -RADIUS, RADIUS * 2, RADIUS * 2)
    , m_x(x), m_y(y), m_vx(vx), m_vy(vy)
    , m_mass(mass), m_owner(owner), m_active(true)
{
    QColor color = (owner == 0) ? QColor(220, 50, 47) : QColor(38, 139, 210);
    setPen(QPen(color.darker(150), 1.5));
    setBrush(QBrush(color));
    setPos(m_x, m_y);
    setZValue(10);
}

// Integración de Euler en tiempo discreto:
// vy(t+dt) = vy(t) + g * dt
// x(t+dt)  = x(t) + vx * dt
// y(t+dt)  = y(t) + vy * dt
void Projectile::update(double dt)
{
    if (!m_active) return;
    m_vy += GRAVITY * dt;   // gravedad: Y crece hacia abajo en Qt
    m_x  += m_vx * dt;
    m_y  += m_vy * dt;
    syncGraphics();
}

void Projectile::syncGraphics()
{
    setPos(m_x, m_y);
}

double Projectile::momentum() const
{
    double speed = std::sqrt(m_vx * m_vx + m_vy * m_vy);
    return m_mass * speed;
}
