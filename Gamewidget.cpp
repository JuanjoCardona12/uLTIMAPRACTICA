#include "gamewidget.h"
#include <QPainter>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <cmath>

GameWidget::GameWidget(QWidget* parent)
    : QGraphicsView(parent)
    , m_state(GameState::WAITING_INPUT)
    , m_players{Player(0), Player(1)}
    , m_projectile(nullptr)
    , m_currentPlayer(0)
{
    m_scene = new QGraphicsScene(0, 0, SCENE_W, SCENE_H, this);
    setScene(m_scene);

    // Configurar vista
    setRenderHint(QPainter::Antialiasing);
    setBackgroundBrush(QBrush(QColor(30, 34, 42)));   // fondo oscuro estilo battlefield
    setFixedSize(static_cast<int>(SCENE_W) + 4, static_cast<int>(SCENE_H) + 4);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Timer de simulación: llama simulationStep() a 60 FPS
    m_timer = new QTimer(this);
    m_timer->setInterval(static_cast<int>(DT * 1000));
    connect(m_timer, &QTimer::timeout, this, &GameWidget::simulationStep);

    setupScene();
    setupObstacles();
}

GameWidget::~GameWidget()
{
    delete m_projectile;
}

// ─────────────────────────────────────────────────────────────
// Configura el escenario: suelo, cielo, figuras de jugadores
// ─────────────────────────────────────────────────────────────
void GameWidget::setupScene()
{
    // Cielo degradado (dos rectángulos)
    auto* sky = m_scene->addRect(0, 0, SCENE_W, SCENE_H * 0.75,
                                 Qt::NoPen, QBrush(QColor(44, 62, 80)));
    sky->setZValue(0);

    // Suelo
    auto* ground = m_scene->addRect(0, SCENE_H * 0.75, SCENE_W, SCENE_H * 0.25,
                                    Qt::NoPen, QBrush(QColor(34, 85, 34)));
    ground->setZValue(0);

    // Borde del escenario (la "caja")
    auto* border = m_scene->addRect(0, 0, SCENE_W, SCENE_H,
                                    QPen(QColor(200, 200, 200, 80), 2), Qt::NoBrush);
    border->setZValue(5);

    // Figuras de jugadores (figuras de palo simples)
    auto drawPlayer = [this](double cx, double cy, QColor color, int idx) {
        auto* group = new QGraphicsItemGroup();

        // Cabeza
        auto* head = new QGraphicsEllipseItem(cx - 10, cy - 50, 20, 20);
        head->setBrush(QBrush(color));
        head->setPen(QPen(color.darker(), 1.5));
        group->addToGroup(head);

        // Cuerpo
        auto* body = new QGraphicsLineItem(cx, cy - 30, cx, cy);
        body->setPen(QPen(color, 2.5));
        group->addToGroup(body);

        // Brazos
        auto* arms = new QGraphicsLineItem(cx - 15, cy - 20, cx + 15, cy - 20);
        arms->setPen(QPen(color, 2.5));
        group->addToGroup(arms);

        // Pierna izquierda
        auto* legL = new QGraphicsLineItem(cx, cy, cx - 12, cy + 20);
        legL->setPen(QPen(color, 2.5));
        group->addToGroup(legL);

        // Pierna derecha
        auto* legR = new QGraphicsLineItem(cx, cy, cx + 12, cy + 20);
        legR->setPen(QPen(color, 2.5));
        group->addToGroup(legR);

        // Etiqueta
        auto* label = new QGraphicsTextItem(QString("Jugador %1").arg(idx + 1));
        label->setDefaultTextColor(color);
        label->setFont(QFont("Arial", 9));
        label->setPos(cx - 30, cy + 20);
        group->addToGroup(label);

        group->setZValue(3);
        m_scene->addItem(group);
        m_playerGraphics[idx] = group;
    };

    drawPlayer(80,  SCENE_H * 0.75 - 20, QColor(220, 80, 80), 0);   // J1: rojo, izquierda
    drawPlayer(820, SCENE_H * 0.75 - 20, QColor(80, 160, 220), 1);  // J2: azul, derecha
}

// ─────────────────────────────────────────────────────────────
// Crea los obstáculos de cada jugador y los posiciona
// ─────────────────────────────────────────────────────────────
void GameWidget::setupObstacles()
{
    double groundY = SCENE_H * 0.75;
    double initRes = 200.0;

    // Jugador 1 (izquierda): 3 obstáculos
    struct ObsConfig { double x, y, w, h, res; };
    QList<ObsConfig> p1obs = {
        { 140, groundY - 80,  70, 80,  initRes },   // pared central
        {  50, groundY - 50,  60, 50,  initRes },   // pared izquierda
        { 140, groundY - 130, 70, 50,  initRes * 0.75 }  // techo
    };

    for (auto& cfg : p1obs) {
        auto* obs = new Obstacle(cfg.x, cfg.y, cfg.w, cfg.h, cfg.res, 0);
        obs->setZValue(2);
        m_scene->addItem(obs);
        m_players[0].addObstacle(obs);
        m_allObstacles.append(obs);
    }

    // Jugador 2 (derecha): 3 obstáculos (espejo)
    QList<ObsConfig> p2obs = {
        { SCENE_W - 210, groundY - 80,  70, 80,  initRes },
        { SCENE_W - 110, groundY - 50,  60, 50,  initRes },
        { SCENE_W - 210, groundY - 130, 70, 50,  initRes * 0.75 }
    };

    for (auto& cfg : p2obs) {
        auto* obs = new Obstacle(cfg.x, cfg.y, cfg.w, cfg.h, cfg.res, 1);
        obs->setZValue(2);
        m_scene->addItem(obs);
        m_players[1].addObstacle(obs);
        m_allObstacles.append(obs);
    }
}

// ─────────────────────────────────────────────────────────────
// Lanza el proyectil del jugador actual
// ─────────────────────────────────────────────────────────────
void GameWidget::launchProjectile(double angleDeg, double speed)
{
    if (m_state != GameState::WAITING_INPUT) return;

    // Eliminar proyectil anterior si existe
    if (m_projectile) {
        m_scene->removeItem(m_projectile);
        delete m_projectile;
        m_projectile = nullptr;
    }

    // Posición inicial según jugador
    double startX, startY;
    double angleRad;

    if (m_currentPlayer == 0) {
        startX   = 100.0;
        startY   = SCENE_H * 0.75 - 60;
        angleRad = qDegreesToRadians(angleDeg);       // dispara hacia la derecha
    } else {
        startX   = SCENE_W - 100.0;
        startY   = SCENE_H * 0.75 - 60;
        angleRad = qDegreesToRadians(180.0 - angleDeg); // dispara hacia la izquierda
    }

    double vx = speed * std::cos(angleRad);
    double vy = -speed * std::sin(angleRad); // negativo: arriba en coordenadas Qt

    m_projectile = new Projectile(startX, startY, vx, vy, 5.0, m_currentPlayer);
    m_scene->addItem(m_projectile);

    m_state = GameState::SIMULATING;
    emit stateChanged(m_state);
    m_timer->start();
}

// ─────────────────────────────────────────────────────────────
// Paso de simulación: se ejecuta 60 veces por segundo
// ─────────────────────────────────────────────────────────────
void GameWidget::simulationStep()
{
    if (!m_projectile || !m_projectile->isActive()) {
        endTurn();
        return;
    }

    // 1. Actualizar posición (ecuaciones de movimiento en tiempo discreto)
    m_projectile->update(DT);

    // 2. Verificar colisiones
    checkCollisions();
}

// ─────────────────────────────────────────────────────────────
// Detección y resolución de todas las colisiones
// ─────────────────────────────────────────────────────────────
void GameWidget::checkCollisions()
{
    if (!m_projectile) return;

    // --- Colisión 1: ELÁSTICA con paredes ---
    m_physics.elasticWallCollision(m_projectile, SCENE_W, SCENE_H);
    m_projectile->syncGraphics();

    // --- Colisión 2: INELÁSTICA con obstáculos del rival ---
    int rival = 1 - m_currentPlayer;
    for (Obstacle* obs : m_players[rival].obstacles()) {
        if (m_physics.inelasticObstacleCollision(m_projectile, obs)) {
            double remaining = obs->getResistance();
            double damage    = obs->getMaxResistance() - remaining; // aprox
            emit obstacleHit(rival, damage, remaining);

            // Si el obstáculo fue destruido, el jugador rival recibe daño extra
            if (obs->isDestroyed()) {
                m_players[rival].receiveDamage(30.0);
                checkWinCondition();
            }
            m_projectile->syncGraphics();
        }
    }

    // Si el proyectil cayó al suelo y rebotó muy despacio, terminarlo
    double speed = std::sqrt(m_projectile->getVx() * m_projectile->getVx()
                             + m_projectile->getVy() * m_projectile->getVy());
    if (speed < 15.0 && m_projectile->getY() > SCENE_H * 0.70) {
        m_projectile->setActive(false);
        endTurn();
    }
}

// ─────────────────────────────────────────────────────────────
// Cambia al siguiente turno
// ─────────────────────────────────────────────────────────────
void GameWidget::endTurn()
{
    m_timer->stop();
    if (m_projectile) {
        m_scene->removeItem(m_projectile);
        delete m_projectile;
        m_projectile = nullptr;
    }

    if (m_state == GameState::GAME_OVER) return;

    // Verificar condición de victoria antes de cambiar turno
    checkWinCondition();
    if (m_state == GameState::GAME_OVER) return;

    // Cambiar turno
    m_currentPlayer = 1 - m_currentPlayer;
    m_state = GameState::WAITING_INPUT;
    emit stateChanged(m_state);
    emit turnChanged(m_currentPlayer);
}

// ─────────────────────────────────────────────────────────────
// Verifica si algún jugador ganó (rival sin vida)
// ─────────────────────────────────────────────────────────────
void GameWidget::checkWinCondition()
{
    for (int i = 0; i < 2; i++) {
        if (!m_players[i].isAlive()) {
            m_state  = GameState::GAME_OVER;
            m_winner = m_players[1 - i].getName();
            m_timer->stop();
            emit stateChanged(m_state);
            emit gameOver(m_winner);
            return;
        }
    }

    // También gana si destruye todos los obstáculos del rival
    for (int i = 0; i < 2; i++) {
        bool allDestroyed = true;
        for (Obstacle* obs : m_players[i].obstacles()) {
            if (!obs->isDestroyed()) { allDestroyed = false; break; }
        }
        if (allDestroyed) {
            m_state  = GameState::GAME_OVER;
            m_winner = m_players[1 - i].getName();
            m_timer->stop();
            emit stateChanged(m_state);
            emit gameOver(m_winner);
            return;
        }
    }
}
