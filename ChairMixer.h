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
    void free();
    
    ObjBuffer mix(ChairPartBuffer seat, ChairPartBuffer leg, ChairPartBuffer back, ChairPartBuffer arm);

    // Temp Test Method
    ObjBuffer tempTest() {
        ChairBuffer chair1 = chairs[rand() % chairs.size()];
        ChairBuffer chair2 = chairs[rand() % chairs.size()];
        ChairBuffer chair3 = chairs[rand() % chairs.size()];
        ChairBuffer chair4 = chairs[rand() % chairs.size()];

        return mix(chair1.seat, chair2.leg, chair3.back, chair4.arm);
    }

private:
    void transformLeg(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& leg);
    void transformBack(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& back);
    void transformArm(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& arm);
};
#endif // CHAIRMIXER_H