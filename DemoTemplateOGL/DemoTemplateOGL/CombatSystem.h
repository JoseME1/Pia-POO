
#pragma once
#include <string>
#include <random>

// Forward declaration
class Model;

enum class CombatAction {
    ATTACK,
    DEFEND,
    TAUNT,
    DODGE
};

struct CombatStats {
    int health;
    int maxHealth;
    int attack;
    int defense;
    bool isDefending;
    bool isDodging;
};

class CombatSystem {
private:
    CombatStats playerStats;
    CombatStats enemyStats;
    bool combatActive;
    bool isPlayerTurn;
    std::string lastActionLog;
    std::mt19937 rng;

    // ✅ NUEVO: Soporte para animaciones
    Model* playerModel;
    double animationTimer;
    int currentAnimationIndex;
    bool isPlayingActionAnimation;

    //Enemy animation support
    Model* enemyModel;
	bool enemyIsPlayingActionAnimation;
	double enemyAnimationTimer;
	bool enemyAttackToggle;

    // ✅ NUEVO: enemy turn delay
    bool enemyTurnPending;
    double enemyTurnTimer;
    static constexpr double enemyTurnDelayMs = 1500.0; // 1.5 seconds

    // ✅ NUEVO: accumulated mechanics
    float playerDodgeChance;       // accumulated dodge chance, consumed on next enemy action (max 40%)
    float playerCritBonus;         // accumulated crit bonus, persistent (max 100%)
    static constexpr float DODGE_INCREMENT = 0.15f;   // +20% per dodge action
    static constexpr float MAX_DODGE = 0.50f;         // cap 40%
    static constexpr float TAUNT_INCREMENT = 0.10f;  // +10% crit per taunt
    static constexpr float MAX_CRIT_BONUS = 1.0f;    // cap 100%

    //enemy animation duration
	static constexpr double enemyActionAnimDurationMs = 800.0; // milliseconds

    void enemyTurn();
    int calculateDamage(const CombatStats& attacker, const CombatStats& defender, bool isCritical);
    CombatAction getRandomEnemyAction();

    // ✅ NUEVO: Control de animaciones
    void setPlayerAnimation(int animIndex);
	void setEnemyAnimation(int animIndex);
    void updateAnimations(double deltaTime);

public:
    CombatSystem();
    void startCombat();
    void executePlayerAction(CombatAction action);
    void update(double deltaTime);

    bool isCombatActive() const;
    bool isPlayerAlive() const;
    bool isEnemyAlive() const;
    bool getIsPlayerTurn() const;

    CombatStats getPlayerStats() const;
    CombatStats getEnemyStats() const;
    std::string getLastActionLog() const;

    // ✅ NUEVO: Configurar el modelo del jugador
    void setPlayerModel(Model* model);

	// ✅ NUEVO: Configurar el modelo del enemigo
	void setEnemyModel(Model* model);
};