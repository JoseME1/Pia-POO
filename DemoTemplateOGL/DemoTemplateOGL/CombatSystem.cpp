#include "CombatSystem.h"
#include <cstdlib>
#include <sstream>
#include <algorithm>

CombatSystem::CombatSystem()
	: isPlayerTurn(true), combatActive(false) {
}

void CombatSystem::startCombat() {
//Reset stats
	playerStats = CombatStats();
	enemyStats = CombatStats();

	isPlayerTurn = true;
	combatActive = true;
	lastActionLog = "Dense en la madre!!!!!!!!!111";
}

void CombatSystem::endCombat() {
	combatActive = false;
	lastActionLog = "";
}

int CombatSystem::calculateDamage(int baseDamage, float critChance, float defenseReduction) {
//Check critical chance
	bool isCrit = (rand() % 100) < (critChance * 100);
	int damage = isCrit ? baseDamage * 2 : baseDamage;

	//Apply defense reduction
	damage = static_cast<int>(damage * (1.0f - defenseReduction));

	return std::max(10, damage); //10 base damage
}

bool CombatSystem::checkDodge(float dodgeChance) {
	return (rand() % 100) < (dodgeChance * 100);
}

void CombatSystem::resetTemporaryEffects() {
	//Defense stays active only 1 turn
	playerStats.defenseActive = 0.0f;
	enemyStats.defenseActive = 0.0f;
}

void CombatSystem::applyPlayerAction(CombatAction action) {
	std::stringstream log;

	switch (action) {
		case CombatAction::ATTACK:{
			if (checkDodge(enemyStats.dodgeChance)) {
				log << "Fallaste wey!";
			}
			else {
				int damage = calculateDamage(10, playerStats.critChance, enemyStats.defenseActive);
				enemyStats.health -= damage;
				log << "Le diste! Cajero recibe: " << damage << " de daño.";
				if (damage == 20) log << "CRITICO!!!!!11";
			}
			break;
		}

		case CombatAction::DEFEND: {
			playerStats.defenseActive = 0.10f; //reduce incoming damage by 10%
			log << "Te pusiste duro! (reducción de daño 10%)";
			break;
		}

		case CombatAction::TAUNT: {
			playerStats.critChance = std::min(1.0f, playerStats.critChance + 0.10f);
			log << "Estas flexeando esos musculos! (aunque eres puro hueso...)";
			log << "(+10% crítico, total: " << static_cast<int>(playerStats.critChance * 100) << "%)";
			break;
		}

		case CombatAction::DODGE: {
			playerStats.dodgeChance = std::min(0.40f, playerStats.dodgeChance + 0.10f);
			log << "Sacaste los prohibidos! (+10% esquiva, total: "
				<< static_cast<int>(playerStats.dodgeChance * 100) << "%)";
			break;
		}
	}
	lastActionLog = log.str()+"\n";
}

void CombatSystem::enemyTurn() {
	std::stringstream log;

	//Simple AI (random attack/defense if hp is low)
	CombatAction enemyAction = (enemyStats.health < 30 && rand() % 100 < 50)
		? CombatAction::DEFEND
		: CombatAction::ATTACK;

	if (enemyAction == CombatAction::ATTACK) {
		if (checkDodge(playerStats.dodgeChance)) {
			log << "Los prohibidos funcionaron! Que sabroso te mueves.";
		}
		else {
			int damage = calculateDamage(8, enemyStats.critChance, playerStats.defenseActive);
			playerStats.health -= damage;
			log << "Te dieron! Recibes: " << damage << " de daño";
		}
	}
	else {
		enemyStats.defenseActive = 0.10f;
		log << "Se puso duro el del Oxxo";
	}

	lastActionLog += "\n\n" + log.str();
}

void CombatSystem::executePlayerAction(CombatAction action) {
	if (!combatActive || !isPlayerTurn) return;

	//Player's turn
	applyPlayerAction(action);

	//Is enemy dead?
	if (enemyStats.health <= 0) {
		lastActionLog += "\n Le diste en su madre!!!!!!!!!111111";
		combatActive = false;
		return;
	}

	//Enemy´s turn
	isPlayerTurn = false;
	enemyTurn();
	resetTemporaryEffects();

	//is player dead?
	if (playerStats.health <= 0) {
		lastActionLog += "\n No pues, ni pedo XD";
		combatActive = false;
		return;
	}
	isPlayerTurn = true;
}