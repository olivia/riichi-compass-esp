// 1. Declare the arrays (do not assign values here)
extern int hanBase[];
extern int fuMultiplier[]; // Added this since it's used in your getHandScore logic

// 2. Declare the functions (prototypes only)
bool isUpgradedMangan(int hanIndex, int fuIndex);
bool isDealer(int index, int seatRotationNum);
int getHandScore(int hanIndex, int fuIndex, int rotationIndex, int seatRotationNum);
int roundScore(int score);
