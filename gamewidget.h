#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QList>
#include "gamestate.h"
#include "physicsengine.h"
#include "projectile.h"
#include "obstacle.h"
#include "player.h"

class GameWidget : public QGraphicsView {
    Q_OBJECT

public:
    static constexpr double SCENE_W = 900.0;
    static constexpr double SCENE_H = 550.0;
    static constexpr double DT      = 1.0 / 60.0;

    explicit GameWidget(QWidget* parent = nullptr);
    ~GameWidget();

    void launchProjectile(double angleDeg, double speed);

    GameState getState()          const { return m_state; }
    int       getCurrentPlayer() const { return m_currentPlayer; }
    Player*   getPlayer(int i)         { return &m_players[i]; }
    QString   getWinner()         const { return m_winner; }

signals:
    void stateChanged(GameState newState);
    void turnChanged(int player);
    void gameOver(QString winner);
    void obstacleHit(int obstacleOwner, double damage, double remaining);

private slots:
    void simulationStep();

private:
    void setupScene();
    void setupObstacles();
    void checkCollisions();
    void endTurn();
    void checkWinCondition();

    QGraphicsScene*  m_scene;
    QTimer*          m_timer;
    PhysicsEngine    m_physics;
    GameState        m_state;

    Player           m_players[2];
    Projectile*      m_projectile;
    int              m_currentPlayer;
    QString          m_winner;

    QGraphicsItem*   m_playerGraphics[2];
    QList<Obstacle*> m_allObstacles;
};
