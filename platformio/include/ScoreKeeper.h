// Team Scores
#ifndef SCOREKEEPER_H
#define SCOREKEEPER_H

// Team Scores

struct Goal {
    unsigned long time;
    unsigned int team; // 0 = Home, 1 = Away
};

class ScoreKeeper {


private:

    unsigned int homeScore;
    unsigned int awayScore;
    unsigned int pointsToWin;
    unsigned int nextGoalIndex;
    Goal* goals;

    void pushGoal(unsigned int team, unsigned long gameTime);

    Goal peekGoal();

    Goal popGoal();
    
public:

    ScoreKeeper(unsigned int pointsToWin);

    ~ScoreKeeper();

    unsigned int getHomeScore();

    unsigned int getAwayScore();

    char* getHomeScoreFormatted();

    char* getAwayScoreFormatted();

    void scoreHomeGoal(unsigned long gameTime);

    void scoreAwayGoal(unsigned long gameTime);

    void removeLastGoal();

    bool isGameOver();

    bool isHomeWinning();

    bool isAwayWinning();

    unsigned long getTimeSinceLastGoal(unsigned long gameTime);

    unsigned int getLastGoalTeam();

};


#endif