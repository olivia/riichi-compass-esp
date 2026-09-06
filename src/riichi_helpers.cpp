int fuMultiplier[] = {20,25,30,40,50,60,70,80,90,100,110, 1};
int hanBase[] = {8, 16, 32, 64, 2000, 3000, 3000, 4000, 4000, 4000, 6000, 6000, 8000, 16000, 24000, 32000 };

bool isUpgradedMangan(int hanIndex, int fuIndex) {
  return (hanIndex == 3 && fuIndex >= 3 ) || (hanIndex == 2 && fuIndex >= 6);
}

bool isDealer(int index, int seatRotationNum) {
  return index == (seatRotationNum & 3);
}

int getHandScore(int hanIndex, int fuIndex, int rotationIndex, int seatRotationNum) {
  int multiplier = isDealer(rotationIndex, seatRotationNum) ? 6 : 4;
  if (hanIndex >= 4) {
    return multiplier * hanBase[hanIndex]; 
  } else if (isUpgradedMangan(hanIndex, fuIndex)){
    return multiplier * hanBase[4];
  } else {
    return multiplier * fuMultiplier[fuIndex] * hanBase[hanIndex]; 
  }
}

int roundScore(int score) {
  int scoreOffset = score % 100 ? 1: 0;
  int flooredScore = score/100;
  return 100 * (flooredScore + scoreOffset);
}