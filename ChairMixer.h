#include <string>
#include <vector>
#include "ObjBuffer.h"

#ifndef CHAIRMIXER_H
#define CHAIRMIXER_H
using namespace std;

class ChairMixer {
public:
    vector<ChairBuffer> chairs;

    void readFolder(string path);
    ObjBuffer mix(ChairPartBuffer seat, ChairPartBuffer leg, ChairPartBuffer back, ChairPartBuffer arm);

    // Temp Test Method
    ObjBuffer tempTest() {

        ChairBuffer chair1 = chairs[rand() % chairs.size()];
        ChairBuffer chair2 = chairs[rand() % chairs.size()];
        ChairBuffer chair3 = chairs[rand() % chairs.size()];
        ChairBuffer chair4 = chairs[rand() % chairs.size()];

        return mix(chair1.seat, chair2.leg, chair3.back, chair4.arm);
    }
};
#endif // CHAIRMIXER_H