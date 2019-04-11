#include <string>
#include <vector>
#include <set> 
#include "ObjBuffer.h"

#ifndef CHAIRMIXER_H
#define CHAIRMIXER_H
using namespace std;
using namespace Eigen;

class ChairMixer {
public:
    vector<ChairBuffer> chairs;
    MatrixXi record;
    vector<int> seeds;

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
    };

    // Evolve Method
    ObjBuffer initialize(int level, int idx) {
        cout << "evolve call " << idx << endl;
        
        int seat_id = rand() % chairs.size();
        int leg_id = rand() % chairs.size();
        int back_id = rand() % chairs.size();
        int arm_id = rand() % chairs.size();
        
        if (level == 0) { // init
            if (idx == 0) { // init seeds for this level
                seeds.clear();
                for (int i = 0; i < chairs.size(); i++) {
                    seeds.push_back(i);
                }
            }
            int seed = takeSeed();
            seat_id = seed;
            leg_id = seed;
            back_id = seed;
            arm_id = seed;
        } 

        ChairBuffer chair1 = chairs[seat_id];
        ChairBuffer chair2 = chairs[leg_id];
        ChairBuffer chair3 = chairs[back_id];
        ChairBuffer chair4 = chairs[arm_id];

        return mix(chair1.seat, chair2.leg, chair3.back, chair4.arm);
    }

    // Evolve Method
    ObjBuffer evolve(int level, int idx) {
        // cout << "evolve call " << idx << endl;
        
        // int seat_id = rand() % chairs.size();
        // int leg_id = rand() % chairs.size();
        // int back_id = rand() % chairs.size();
        // int arm_id = rand() % chairs.size();
        
        // if (level == 0) { // init
        //     if (idx == 0) { // init seeds for this level
        //         seeds.clear();
        //         for (int i = 0; i < chairs.size(); i++) {
        //             seeds.push_back(i);
        //         }
        //     }
        //     int seed = takeSeed();
        //     seat_id = seed;
        //     leg_id = seed;
        //     back_id = seed;
        //     arm_id = seed;
        // } else if (level == 1) { // swap leg
            
        // }

        // ChairBuffer chair1 = chairs[seat_id];
        // ChairBuffer chair2 = chairs[leg_id];
        // ChairBuffer chair3 = chairs[back_id];
        // ChairBuffer chair4 = chairs[arm_id];

        // return mix(chair1.seat, chair2.leg, chair3.back, chair4.arm);
    }

private:
    void transformLeg(ChairPartBuffer& seat, ChairPartBuffer& leg);
    void transformBack(ChairPartBuffer& seat, ChairPartBuffer& back);
    void transformArm(ChairPartBuffer& seat, ChairPartBuffer& back, ChairPartBuffer& arm);

    int takeSeed();
};
#endif // CHAIRMIXER_H