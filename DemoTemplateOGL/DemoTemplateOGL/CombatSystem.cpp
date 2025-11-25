
#include "CombatSystem.h"
#include "Base/model.h" 
#include "Base/Utilities.h"
#include <sstream>
#include <cmath>
#include <algorithm>

CombatSystem::CombatSystem() : rng(std::random_device{}()), playerModel(nullptr), animationTimer(0.0), currentAnimationIndex(3),
isPlayingActionAnimation(false), enemyTurnPending(false), enemyTurnTimer(0.0),
playerDodgeChance(0.0f), playerCritBonus(0.0f)
{
    playerStats = { 100, 100, 20, 10, false, false };
    enemyStats = { 100, 100, 20, 10, false, false };
    combatActive = false;
    isPlayerTurn = true;
    lastActionLog = "";
}

void CombatSystem::setPlayerModel(Model* model) {
    playerModel = model;
}

void CombatSystem::setPlayerAnimation(int animIndex) {
    if (playerModel != nullptr) {
        playerModel->setAnimation(animIndex);
        currentAnimationIndex = animIndex;
    }
}

void CombatSystem::updateAnimations(double deltaTime) {
    if (!isPlayingActionAnimation) {
        return;
    }

    animationTimer += deltaTime;

    // After 2 seconds (2000ms) return to combat idle
    if (animationTimer >= 2000.0) {
        setPlayerAnimation(3); // combat idle
        isPlayingActionAnimation = false;
        animationTimer = 0.0;
    }
}

void CombatSystem::startCombat() {
    playerStats.health = playerStats.maxHealth;
    enemyStats.health = enemyStats.maxHealth;
    combatActive = true;
    isPlayerTurn = true;
    lastActionLog = "¡Dense en la madre!";

    // initial combat state
    setPlayerAnimation(3); // Idle de combate
    isPlayingActionAnimation = false;
    animationTimer = 0.0;
    enemyTurnPending = false;
    enemyTurnTimer = 0.0;
    playerDodgeChance = 0.0f;
    playerCritBonus = 0.0f;
}

void CombatSystem::executePlayerAction(CombatAction action) {
    if (!combatActive || !isPlayerTurn) return;

    // reset per-turn booleans
    playerStats.isDefending = false;
    playerStats.isDodging = false;

    std::stringstream log;

    switch (action) {
    case CombatAction::ATTACK: {
        // Alternate between attack animations
        static bool useAttack1 = true;
        int attackAnim = useAttack1 ? 4 : 5;
        setPlayerAnimation(attackAnim);
        useAttack1 = !useAttack1;

        isPlayingActionAnimation = true;
        animationTimer = 0.0;

        std::uniform_int_distribution<> dist(1, 100);
        // compute effective crit chance (base 210% + accumulated bonus up to 100%)
        int effectiveCritPercent = std::min(100, 10 + (int)std::round(playerCritBonus * 100.0f));
        bool isCritical = dist(rng) <= effectiveCritPercent;

        int damage = calculateDamage(playerStats, enemyStats, isCritical);
        enemyStats.health -= damage;

        log << "Le diste un sape a ese wey! Enemigo recibe: " << damage << " de damage";
        if (isCritical) log << " ¡LE TIRAS UN DIENTE!";
        break;
    }

    case CombatAction::DEFEND: {
        playerStats.isDefending = true;
        log << "ME HAGO BOLITA";
        setPlayerAnimation(3); // no special defend animation here
        break;
    }

    case CombatAction::TAUNT: {
        // increase player's crit bonus by TAUNT_INCREMENT up to MAX_CRIT_BONUS
        playerCritBonus = std::min(MAX_CRIT_BONUS, playerCritBonus + TAUNT_INCREMENT);
        log << "Ese wey voto por cheinbau! (Probabilidad de critico: " << (int)std::round(playerCritBonus * 100.0f) << "%)";
        setPlayerAnimation(3);
        break;
    }

    case CombatAction::DODGE: {
        // increase dodge chance (accumulates up to MAX_DODGE)
        playerDodgeChance = std::min(MAX_DODGE, playerDodgeChance + DODGE_INCREMENT);
        // play dodge anim
        setPlayerAnimation(0);
        isPlayingActionAnimation = true;
        animationTimer = 0.0;
        log << "Sacas los prohibidos! (Probabilidad de esquivar: " << (int)std::round(playerDodgeChance * 100.0f) << "%)";
        break;
    }
    }

    lastActionLog = log.str();

    if (enemyStats.health <= 0) {
        lastActionLog += "\n¡Le diste en su madre!!";
        combatActive = false;
        // return to normal idle when combat ends
        setPlayerAnimation(1);
        enemyTurnPending = false;
    }
    else {
        // schedule enemy turn after delay; do NOT force idle here so action animation can play out
        isPlayerTurn = false;
        enemyTurnPending = true;
        enemyTurnTimer = 0.0;
        
    }
}

void CombatSystem::enemyTurn() {
    if (!combatActive || isPlayerTurn) return;

    // clear per-turn flags for enemy
    enemyStats.isDefending = false;
    enemyStats.isDodging = false;

    // Only allow ATTACK or DEFEND for enemy actions
    CombatAction action;
    {
        std::uniform_int_distribution<> dist(0, 1);
        action = dist(rng) == 0 ? CombatAction::ATTACK : CombatAction::DEFEND;
    }

    std::stringstream log;

    switch (action) {
    case CombatAction::ATTACK: {
        // Determine if player dodges using accumulated dodge chance (consumed once)
        if (playerDodgeChance > 0.0f) {
            std::uniform_real_distribution<float> roll(0.0f, 1.0f);
            float r = roll(rng);
            if (r <= playerDodgeChance) {
                log << "¡Te mueves tan sabroso que lo esquivaste!";
                // consume dodge chance after the enemy attempt (reset to 0)
                //playerDodgeChance = 0.0f;
                lastActionLog = log.str();
                playerStats.isDodging = false;
                // enemy action ends (no damage)
                isPlayerTurn = true;
                // ensure player returns to combat idle if not playing action anim
                if (!isPlayingActionAnimation) setPlayerAnimation(3);
                return;
            }
        }

        // If not dodged, compute attack
        std::uniform_int_distribution<> dist(1, 100);
        bool isCritical = dist(rng) <= 15; // enemy base crit chance unchanged

        int damage = calculateDamage(enemyStats, playerStats, isCritical);
        playerStats.health -= damage;

        log << "El enemigo te suelta un golpe. Recibes: " << damage << " de damage";
        if (isCritical) log << " ¡Te sento de un ver...!";
        break;
    }

    case CombatAction::DEFEND:
        enemyStats.isDefending = true;
        log << "El enemigo se hace bolita";
        break;

    default:
        enemyStats.isDefending = true;
        log << "El enemigo no tiene tu tiempo...";
        break;
    }

    // After resolving enemy action, consume player's dodge chance (it applied/was available for this enemy action)
   // playerDodgeChance = 0.0f;

    lastActionLog = log.str();
    playerStats.isDodging = false;

    if (playerStats.health <= 0) {
        lastActionLog += "\n!Si bueno... quien tiene hambre?!";
        combatActive = false;
        // return to normal idle
        setPlayerAnimation(1);
    }
    else {
        isPlayerTurn = true;
        // After enemy completes action, ensure player idle combat animation (unless playing own action)
        if (!isPlayingActionAnimation) {
            setPlayerAnimation(3);
        }
    }
}

void CombatSystem::update(double deltaTime) {
    // update animations timer
    updateAnimations(deltaTime);

    if (!combatActive) return;

    // If enemy turn is pending, wait the configured delay before executing
    if (enemyTurnPending) {
        enemyTurnTimer += deltaTime;
        if (enemyTurnTimer >= enemyTurnDelayMs) {
            enemyTurnPending = false;
            enemyTurnTimer = 0.0;
            // perform enemy action now
            enemyTurn();
        }
        // still waiting
        return;
    }

    // fallback: if it's enemy's turn and nothing pending, perform immediately
    if (!isPlayerTurn && combatActive) {
        enemyTurn();
    }
}

int CombatSystem::calculateDamage(const CombatStats& attacker, const CombatStats& defender, bool isCritical) {
    float baseDamage = static_cast<float>(attacker.attack);

    if (isCritical) {
        baseDamage *= 2.0f;
    }

    // If defender is defending, reduce damage by 40%
    float defendMultiplier = defender.isDefending ? 0.6f : 1.0f;

    // Apply defense scaling: damage = base * (100 / (100 + defense)) * defendMultiplier
    float scaled = baseDamage * (100.0f / (100.0f + static_cast<float>(defender.defense))) * defendMultiplier;
    int finalDamage = std::max(1, (int)std::round(scaled));
    return finalDamage;
}

CombatAction CombatSystem::getRandomEnemyAction() {
    std::uniform_int_distribution<> dist(0, 3);
    return static_cast<CombatAction>(dist(rng));
}

bool CombatSystem::isCombatActive() const { return combatActive; }
bool CombatSystem::isPlayerAlive() const { return playerStats.health > 0; }
bool CombatSystem::isEnemyAlive() const { return enemyStats.health > 0; }
bool CombatSystem::getIsPlayerTurn() const { return isPlayerTurn; }
CombatStats CombatSystem::getPlayerStats() const { return playerStats; }
CombatStats CombatSystem::getEnemyStats() const { return enemyStats; }
std::string CombatSystem::getLastActionLog() const { return lastActionLog; }