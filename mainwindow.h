#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QProgressBar>
#include <QGroupBox>
#include "gamewidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLaunch();
    void onReset();
    void onStateChanged(GameState state);
    void onTurnChanged(int player);
    void onGameOver(QString winner);
    void onObstacleHit(int owner, double damage, double remaining);

private:
    void setupUI();
    void updateTurnLabel();
    void updateHealthBars();

    GameWidget*   m_game;
    QGroupBox*    m_controlBox;
    QLabel*       m_turnLabel;
    QLabel*       m_angleLabel;
    QSlider*      m_angleSlider;
    QLabel*       m_speedLabel;
    QSlider*      m_speedSlider;
    QPushButton*  m_launchBtn;
    QPushButton*  m_resetBtn;
    QProgressBar* m_p1Health;
    QProgressBar* m_p2Health;
    QLabel*       m_eventLog;
};
