#include "obstacle.h"
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QColor>

Obstacle::Obstacle(double x, double y, double w, double h, double resistance, int owner)
    : QGraphicsRectItem(x, y, w, h)
    , m_x(x), m_y(y), m_w(w), m_h(h)
    , m_resistance(resistance), m_maxResistance(resistance)
    , m_owner(owner), m_destroyed(false)
{
    // Jugador 1 = tonos naranjas, Jugador 2 = tonos verdes
    QColor fill = (owner == 0) ? QColor(255, 170, 80) : QColor(80, 200, 120);
    QColor border = fill.darker(160);
    setPen(QPen(border, 2));
    setBrush(QBrush(fill));

    // Etiqueta de resistencia encima del obstáculo
    m_label = new QGraphicsTextItem(this);  // hijo de este item
    QFont f;
    f.setPixelSize(13);
    f.setBold(true);
    m_label->setFont(f);
    m_label->setDefaultTextColor(Qt::black);

    refreshVisuals();
}

void Obstacle::applyDamage(double damage)
{
    if (m_destroyed) return;
    m_resistance -= damage;
    if (m_resistance <= 0.0) {
        m_resistance = 0.0;
        m_destroyed  = true;
        setVisible(false);       // desaparece del escenario
        m_label->setVisible(false);
    }
    refreshVisuals();
}

void Obstacle::refreshVisuals()
{
    // Actualiza etiqueta
    m_label->setPlainText(QString::number(static_cast<int>(m_resistance)));

    // Centra la etiqueta horizontalmente en el obstáculo
    double lw = m_label->boundingRect().width();
    m_label->setPos(m_x + (m_w - lw) / 2.0, m_y + 4);

    // Color degradado según resistencia restante (verde -> rojo)
    if (!m_destroyed) {
        double ratio = m_resistance / m_maxResistance;
        // Interpola entre rojo (ratio 0) y color original (ratio 1)
        QColor base = (m_owner == 0) ? QColor(255, 170, 80) : QColor(80, 200, 120);
        QColor damaged(
            static_cast<int>(base.red()   * ratio + 200 * (1 - ratio)),
            static_cast<int>(base.green() * ratio +  50 * (1 - ratio)),
            static_cast<int>(base.blue()  * ratio +  50 * (1 - ratio))
            );
        setBrush(QBrush(damaged));
    }
}
