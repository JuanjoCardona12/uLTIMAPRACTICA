#pragma once

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

class Obstacle : public QGraphicsRectItem {
public:
    Obstacle(double x, double y, double w, double h, double resistance, int owner);

    void applyDamage(double damage);
    void refreshVisuals();

    double getX()             const { return m_x; }
    double getY()             const { return m_y; }
    double getW()             const { return m_w; }
    double getH()             const { return m_h; }
    double getResistance()    const { return m_resistance; }
    double getMaxResistance() const { return m_maxResistance; }
    int    getOwner()         const { return m_owner; }
    bool   isDestroyed()      const { return m_destroyed; }

private:
    double m_x, m_y, m_w, m_h;
    double m_resistance;
    double m_maxResistance;
    int    m_owner;
    bool   m_destroyed;

    QGraphicsTextItem* m_label;
};
