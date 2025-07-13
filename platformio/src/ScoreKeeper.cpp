// Team Scores
#include "ScoreKeeper.h"
#include <Arduino.h>

void ScoreKeeper::pushGoal(unsigned int team, unsigned long gameTime) {
    // Record the goal
    Goal goal;
    goal.team = team;
    goal.time = gameTime; 
    this->goals[nextGoalIndex] = goal;

    // Increment pointer.
    nextGoalIndex++;
}

Goal ScoreKeeper::peekGoal() {
    // There have been no goals.
    if (this->nextGoalIndex <= 0) {
        Goal goal;
        goal.team = 2;
        goal.time = 0;
        return goal;
    } else {
        return goals[nextGoalIndex - 1];
    }
}

Goal ScoreKeeper::popGoal() {
    Goal goal = peekGoal();
    if (this->nextGoalIndex > 0) {
        nextGoalIndex--;
    }
    return goal;
}

ScoreKeeper::ScoreKeeper(unsigned int pointsToWin) : pointsToWin(pointsToWin), goals(new Goal[(pointsToWin * 2) - 1]), homeScore(0), awayScore(0), nextGoalIndex(0)  {
}

ScoreKeeper::~ScoreKeeper() {
    delete[] goals;
}

unsigned int ScoreKeeper::getHomeScore() {
    return homeScore;
}

unsigned int ScoreKeeper::getAwayScore() {
    return awayScore;
}

char* ScoreKeeper::getHomeScoreFormatted() {
    // Buffer to store the formatted string
    static char scoreStr[3]; // Sufficient size for an unsigned int and null-terminator

    // Convert the unsigned int to a string
    snprintf(scoreStr, sizeof(scoreStr), "%u", homeScore);

    // Return the pointer to the static buffer
    return scoreStr;
}

char* ScoreKeeper::getAwayScoreFormatted() {
    // Buffer to store the formatted string
    static char scoreStr[3]; // Sufficient size for an unsigned int and null-terminator

    // Convert the unsigned int to a string
    snprintf(scoreStr, sizeof(scoreStr), "%u", awayScore);

    // Return the pointer to the static buffer
    return scoreStr;
}

void ScoreKeeper::scoreHomeGoal(unsigned long gameTime) {
    if (!isGameOver()) {
        homeScore++;
        pushGoal(0, gameTime);
    }
}

void ScoreKeeper::scoreAwayGoal(unsigned long gameTime) {
    if (!isGameOver()) {
        awayScore++;
        pushGoal(1, gameTime);
    }
}

void ScoreKeeper::removeLastGoal() {
    Goal lastGoal = popGoal();
    switch (lastGoal.team) {
        case 0:
            homeScore--;
            break;
        case 1:
            awayScore--;
            break;
    }
}

bool ScoreKeeper::isGameOver() {
    return (homeScore >= pointsToWin) || (awayScore >= pointsToWin);
}

bool ScoreKeeper::isHomeWinning() {
    return homeScore > awayScore;
}

bool ScoreKeeper::isAwayWinning() {
    return awayScore > homeScore;
}

unsigned long ScoreKeeper::getTimeSinceLastGoal(unsigned long gameTime) {
    return gameTime - peekGoal().time;
}

unsigned int ScoreKeeper::getLastGoalTeam() {
    return peekGoal().team;
}


