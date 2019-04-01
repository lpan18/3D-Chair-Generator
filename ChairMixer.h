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
        ChairBuffer chair1 = chairs[3];
        ChairBuffer chair2 = chairs[4];

        return mix(chair1.seat, chair2.leg, chair1.back, chair2.arm);
    }
};
#endif // CHAIRMIXER_H