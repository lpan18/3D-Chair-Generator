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
    Vector3f transformBySeatFeatures(Matrix3f scale, Vector3f v, Vector3f oldBase, Vector3f newBase);
    void transformLeg(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& leg, int legVI);
    void transformBack(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& back, int backVI);
    void transformArm(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& arm, int armVI);
};
#endif // CHAIRMIXER_H