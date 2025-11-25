#pragma once
#include <string>

enum class CombatAction {
	ATTACK,
	DEFEND,
	TAUNT,
	DODGE
};

struct CombatStats {
	int health;
	int maxHealth;
	float critChance; //0.0 to 1.0
	float dodgeChance; //0.0 to 1.0
	float defenseActive; //0.0 to 1.0

	CombatStats() : health(100), maxHealth(100), critChance(0.1f), dodgeChance(0.1f), defenseActive(0.0f) {}
};

class CombatSystem {
private:
	CombatStats playerStats;
	CombatStats enemyStats;
	bool isPlayerTurn;
	std::string lastActionLog;
	bool combatActive;

	int calculateDamage(int baseDamage, float critChance, float defenseReduction);
	bool checkDodge(float dodgeChance);
	void applyPlayerAction(CombatAction action);
	void enemyTurn();
	void resetTemporaryEffects();

public:
	CombatSystem();

	//Iniciar/Terminar combate
	void startCombat();
	void endCombat();

	//Acciones del jugador
	void executePlayerAction(CombatAction action);

	//Getters
	bool isCombatActive() const { return combatActive; }
	bool isPlayerAlive() const { return playerStats.health>0; }
	bool isEnemyAlive() const { return enemyStats.health>0; }
	const CombatStats& getPlayerStats() const { return playerStats; }
	const CombatStats& getEnemyStats() const { return enemyStats; }
	std::string getLastActionLog() const { return lastActionLog; }
	bool getIsPlayerTurn() const { return isPlayerTurn; }


};