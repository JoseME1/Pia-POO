
#include "CombatSystem.h"
#include "Base/model.h" 
#include "Base/Utilities.h"
#include <sstream>
#include <cmath>
#include <algorithm>

CombatSystem::CombatSystem() : rng(std::random_device{}()),
playerModel(nullptr),
animationTimer(0.0),
currentAnimationIndex(3),
isPlayingActionAnimation(false),
enemyModel(nullptr),
enemyIsPlayingActionAnimation(false),
enemyAnimationTimer(0.0),
enemyAttackToggle(false),
enemyTurnPending(false),
enemyTurnTimer(0.0),
playerDodgeChance(0.0f),
playerCritBonus(0.0f)
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

void CombatSystem::setEnemyModel(Model* model) {
    this->enemyModel = model;
    if (this->enemyModel != nullptr) {
		this->enemyModel->setAnimation(1); // enemy combat idle
    }
}

void CombatSystem::setPlayerAnimation(int animIndex) {
    if (playerModel != nullptr) {
        playerModel->setAnimation(animIndex);
        currentAnimationIndex = animIndex;
    }
}

void CombatSystem::setEnemyAnimation(int animIndex) {
    if (enemyModel != nullptr) {
        enemyModel->setAnimation(animIndex);
    }
}

void CombatSystem::updateAnimations(double deltaTime) {
    // Player action animation timeout
    if (isPlayingActionAnimation) {
        animationTimer += deltaTime;
        if (animationTimer >= 2000.0) {
            setPlayerAnimation(3); // player combat idle
            isPlayingActionAnimation = false;
            animationTimer = 0.0;
        }
    }

    // Enemy action animation timeout
    if (enemyIsPlayingActionAnimation) {
        enemyAnimationTimer += deltaTime;
        if (enemyAnimationTimer >= enemyActionAnimDurationMs) {
            setEnemyAnimation(1); // enemy combat idle
            enemyIsPlayingActionAnimation = false;
            enemyAnimationTimer = 0.0;
        }
    }
}

void CombatSystem::startCombat() {
    playerStats.health = playerStats.maxHealth;
    enemyStats.health = enemyStats.maxHealth;
    combatActive = true;
    isPlayerTurn = true;
    lastActionLog = "¡Dense en la madre!";

    // initial combat state
    setPlayerAnimation(3); // Idle de combate (player)
    isPlayingActionAnimation = false;
    animationTimer = 0.0;

    // enemy base combat animation index 1
    setEnemyAnimation(1);
    enemyIsPlayingActionAnimation = false;
    enemyAnimationTimer = 0.0;
    enemyAttackToggle = false;

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
        // Alternate between attack animations for player
        static bool useAttack1 = true;
        int attackAnim = useAttack1 ? 4 : 5;
        setPlayerAnimation(attackAnim);
        useAttack1 = !useAttack1;

        isPlayingActionAnimation = true;
        animationTimer = 0.0;

        std::uniform_int_distribution<> dist(1, 100);
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
        setPlayerAnimation(3);
        break;
    }

    case CombatAction::TAUNT: {
        playerCritBonus = std::min(MAX_CRIT_BONUS, playerCritBonus + TAUNT_INCREMENT);
        log << "Ese wey voto por cheinbau! (Probabilidad de critico: " << (int)std::round(playerCritBonus * 100.0f) << "%)";
        setPlayerAnimation(3);
        break;
    }

    case CombatAction::DODGE: {
        playerDodgeChance = std::min(MAX_DODGE, playerDodgeChance + DODGE_INCREMENT);
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
        setPlayerAnimation(1);
        enemyTurnPending = false;
    }
    else {
        // schedule enemy turn after delay
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
        // Enemy attack animation: alternate between 2 and 3, then return to 1 after duration
        enemyAttackToggle = !enemyAttackToggle;
        int enemyAttackAnim = enemyAttackToggle ? 2 : 3;
        setEnemyAnimation(enemyAttackAnim);
        enemyIsPlayingActionAnimation = true;
        enemyAnimationTimer = 0.0;

        // Determine if player dodges using accumulated dodge chance
        if (playerDodgeChance > 0.0f) {
            std::uniform_real_distribution<float> roll(0.0f, 1.0f);
            float r = roll(rng);
            if (r <= playerDodgeChance) {
                log << "¡Te mueves tan sabroso que lo esquivaste!";
                //playerDodgeChance = 0.0f;
                lastActionLog = log.str();
                playerStats.isDodging = false;
                isPlayerTurn = true;
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
        // keep enemy at combat idle visually
        setEnemyAnimation(1);
        enemyIsPlayingActionAnimation = false;
        enemyAnimationTimer = 0.0;
        break;

    default:
        enemyStats.isDefending = true;
        log << "El enemigo no tiene tu tiempo...";
        setEnemyAnimation(1);
        enemyIsPlayingActionAnimation = false;
        enemyAnimationTimer = 0.0;
        break;
    }

    // consume player's dodge chance after enemy action attempt
    //playerDodgeChance = 0.0f;

    lastActionLog = log.str();
    playerStats.isDodging = false;

    if (playerStats.health <= 0) {
        lastActionLog += "\n!Si bueno... quien tiene hambre?!";
        combatActive = false;
        setPlayerAnimation(1);
        setEnemyAnimation(0);
    }
    else {
        isPlayerTurn = true;
        if (!isPlayingActionAnimation) {
            setPlayerAnimation(3);
        }
    }
}

void CombatSystem::update(double deltaTime) {
    // update animations timer (player + enemy)
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