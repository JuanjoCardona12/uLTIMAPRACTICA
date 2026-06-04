#pragma once

#include <QGraphicsEllipseItem>

class Projectile : public QGraphicsEllipseItem {
public:
    static constexpr double RADIUS        = 10.0;
    static constexpr double GRAVITY       = 200.0;
    static constexpr double DAMAGE_FACTOR = 0.05;

    Projectile(double x, double y, double vx, double vy, double mass, int owner);

    void   update(double dt);
    void   syncGraphics();
    double momentum() const;

    double getX()     const { return m_x; }
    double getY()     const { return m_y; }
    double getVx()    const { return m_vx; }
    double getVy()    const { return m_vy; }
    double getMass()  const { return m_mass; }
    int    getOwner() const { return m_owner; }
    bool   isActive() const { return m_active; }

    void setVx(double vx)  { m_vx = vx; }
    void setVy(double vy)  { m_vy = vy; }
    void setActive(bool a) { m_active = a; }

private:
    double m_x, m_y;
    double m_vx, m_vy;
    double m_mass;
    int    m_owner;
    bool   m_active;
};
