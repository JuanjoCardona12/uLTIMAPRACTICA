#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QFont>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();

    connect(m_game, &GameWidget::stateChanged,  this, &MainWindow::onStateChanged);
    connect(m_game, &GameWidget::turnChanged,   this, &MainWindow::onTurnChanged);
    connect(m_game, &GameWidget::gameOver,      this, &MainWindow::onGameOver);
    connect(m_game, &GameWidget::obstacleHit,   this, &MainWindow::onObstacleHit);
    connect(m_launchBtn, &QPushButton::clicked, this, &MainWindow::onLaunch);
    connect(m_resetBtn,  &QPushButton::clicked, this, &MainWindow::onReset);

    connect(m_angleSlider, &QSlider::valueChanged, [this](int v) {
        m_angleLabel->setText(QString("Angulo: %1 grados").arg(v));
    });
    connect(m_speedSlider, &QSlider::valueChanged, [this](int v) {
        m_speedLabel->setText(QString("Velocidad: %1 px/s").arg(v));
    });

    updateTurnLabel();
    updateHealthBars();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    auto* central    = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // ── Juego ────────────────────────────────────────────────
    m_game = new GameWidget(this);
    mainLayout->addWidget(m_game);

    // ── Panel de control ─────────────────────────────────────
    auto* right = new QVBoxLayout();
    mainLayout->addLayout(right);

    auto* title = new QLabel("BattlePhysics");
    QFont tf; tf.setPixelSize(20); tf.setBold(true);
    title->setFont(tf);
    title->setAlignment(Qt::AlignCenter);
    right->addWidget(title);

    m_turnLabel = new QLabel();
    QFont tnf; tnf.setPixelSize(15); tnf.setBold(true);
    m_turnLabel->setFont(tnf);
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setStyleSheet("color:#e74c3c;padding:6px;background:#2c3e50;border-radius:6px;");
    right->addWidget(m_turnLabel);

    right->addSpacing(6);

    // Barras de resistencia (se inicializan con rango 0-1, se ajustan al crear GameWidget)
    auto* p1Box    = new QGroupBox("Jugador 1");
    auto* p1Layout = new QVBoxLayout(p1Box);
    m_p1Health     = new QProgressBar();
    m_p1Health->setStyleSheet("QProgressBar::chunk{background:#e74c3c;}");
    m_p1Health->setFormat("%v / %m");
    p1Layout->addWidget(m_p1Health);
    right->addWidget(p1Box);

    auto* p2Box    = new QGroupBox("Jugador 2");
    auto* p2Layout = new QVBoxLayout(p2Box);
    m_p2Health     = new QProgressBar();
    m_p2Health->setStyleSheet("QProgressBar::chunk{background:#3498db;}");
    m_p2Health->setFormat("%v / %m");
    p2Layout->addWidget(m_p2Health);
    right->addWidget(p2Box);

    right->addSpacing(6);

    // Control de disparo
    m_controlBox   = new QGroupBox("Control de disparo");
    auto* ctrlLayout = new QVBoxLayout(m_controlBox);

    m_angleLabel = new QLabel("Angulo: 45 grados");
    m_angleLabel->setAlignment(Qt::AlignCenter);
    ctrlLayout->addWidget(m_angleLabel);

    m_angleSlider = new QSlider(Qt::Horizontal);
    m_angleSlider->setRange(1, 89);
    m_angleSlider->setValue(45);
    m_angleSlider->setTickPosition(QSlider::TicksBelow);
    m_angleSlider->setTickInterval(15);
    ctrlLayout->addWidget(m_angleSlider);

    m_speedLabel = new QLabel("Velocidad: 400 px/s");
    m_speedLabel->setAlignment(Qt::AlignCenter);
    ctrlLayout->addWidget(m_speedLabel);

    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setRange(100, 800);
    m_speedSlider->setValue(400);
    m_speedSlider->setTickPosition(QSlider::TicksBelow);
    m_speedSlider->setTickInterval(100);
    ctrlLayout->addWidget(m_speedSlider);

    m_launchBtn = new QPushButton("Lanzar!");
    QFont bf; bf.setPixelSize(15); bf.setBold(true);
    m_launchBtn->setFont(bf);
    m_launchBtn->setMinimumHeight(44);
    m_launchBtn->setStyleSheet(
        "QPushButton{background:#27ae60;color:white;border-radius:8px;}"
        "QPushButton:hover{background:#2ecc71;}"
        "QPushButton:disabled{background:#7f8c8d;}"
        );
    ctrlLayout->addWidget(m_launchBtn);
    right->addWidget(m_controlBox);

    // Reiniciar
    m_resetBtn = new QPushButton("Nueva partida");
    m_resetBtn->setMinimumHeight(34);
    m_resetBtn->setStyleSheet(
        "QPushButton{background:#8e44ad;color:white;border-radius:6px;}"
        "QPushButton:hover{background:#9b59b6;}"
        );
    right->addWidget(m_resetBtn);

    // Instrucciones
    auto* instrBox    = new QGroupBox("Como jugar");
    auto* instrLayout = new QVBoxLayout(instrBox);
    auto* instrText   = new QLabel(
        "- Ajusta angulo y velocidad\n"
        "- Presiona Lanzar para disparar\n"
        "- El proyectil rebota en paredes\n"
        "- Daña obstaculos del rival\n"
        "- Destruye todos sus bloques\n"
        "  para ganar"
        );
    instrText->setWordWrap(true);
    instrLayout->addWidget(instrText);
    right->addWidget(instrBox);

    // Log de eventos
    m_eventLog = new QLabel("Esperando disparo...");
    m_eventLog->setWordWrap(true);
    m_eventLog->setStyleSheet(
        "background:#1a1a2e;color:#a8e6cf;padding:6px;"
        "border-radius:4px;font-family:monospace;font-size:11px;"
        );
    m_eventLog->setMinimumHeight(60);
    right->addWidget(m_eventLog);

    right->addStretch();
}

void MainWindow::onLaunch()
{
    double angle = m_angleSlider->value();
    double speed = m_speedSlider->value();
    m_game->launchProjectile(angle, speed);
    m_eventLog->setText(QString("Disparo: %1 grados | %2 px/s")
                            .arg(static_cast<int>(angle))
                            .arg(static_cast<int>(speed)));
}

void MainWindow::onReset()
{
    m_game->resetGame();
    m_eventLog->setText("Nueva partida iniciada.");
    updateTurnLabel();
    updateHealthBars();

    QString color = "#e74c3c";
    m_turnLabel->setStyleSheet(
        QString("color:%1;padding:6px;background:#2c3e50;border-radius:6px;").arg(color));
}

void MainWindow::onStateChanged(GameState state)
{
    bool canLaunch = (state == GameState::WAITING_INPUT);
    m_launchBtn->setEnabled(canLaunch);
    m_angleSlider->setEnabled(canLaunch);
    m_speedSlider->setEnabled(canLaunch);
    updateHealthBars();
}

void MainWindow::onTurnChanged(int player)
{
    updateTurnLabel();
    QString color = (player == 0) ? "#e74c3c" : "#3498db";
    m_turnLabel->setStyleSheet(
        QString("color:%1;padding:6px;background:#2c3e50;border-radius:6px;").arg(color));
}

void MainWindow::onGameOver(QString winner)
{
    m_turnLabel->setText(QString("%1 GANA!").arg(winner));
    m_turnLabel->setStyleSheet(
        "color:#f1c40f;padding:6px;background:#2c3e50;border-radius:6px;");
    m_launchBtn->setEnabled(false);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Fin del juego!");
    msgBox.setText(QString("%1 ha ganado la batalla!").arg(winner));
    msgBox.setInformativeText("Presiona 'Nueva partida' para volver a jugar.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::onObstacleHit(int owner, double damage, double remaining)
{
    QString pName = (owner == 0) ? "J1" : "J2";
    m_eventLog->setText(
        QString("Obstaculo de %1 golpeado\nDano: %2 | Resistencia: %3")
            .arg(pName)
            .arg(static_cast<int>(damage))
            .arg(static_cast<int>(remaining)));
    updateHealthBars();
}

void MainWindow::updateTurnLabel()
{
    int p = m_game->getCurrentPlayer();
    m_turnLabel->setText(QString("Turno: Jugador %1").arg(p + 1));
}

void MainWindow::updateHealthBars()
{
    Player* p1 = m_game->getPlayer(0);
    Player* p2 = m_game->getPlayer(1);

    // Ajustar el rango según la salud máxima real (suma de resistencias iniciales)
    m_p1Health->setRange(0, static_cast<int>(p1->getMaxHealth()));
    m_p1Health->setValue(static_cast<int>(p1->getHealth()));

    m_p2Health->setRange(0, static_cast<int>(p2->getMaxHealth()));
    m_p2Health->setValue(static_cast<int>(p2->getHealth()));
}
