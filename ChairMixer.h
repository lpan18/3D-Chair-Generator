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

    // Initialize Method
    ObjBuffer initialize(int idx, int n_to_show) {
        record = MatrixXi::Zero(n_to_show, 4);
        ChairBuffer chair1 = chairs[idx];
        ChairBuffer chair2 = chairs[idx];
        ChairBuffer chair3 = chairs[idx];
        ChairBuffer chair4 = chairs[idx];

        return mix(chair1.seat, chair2.leg, chair3.back, chair4.arm);
    }

    // Evolve Method
    ObjBuffer evolve(int level, int idx) {
        int which_record = int(idx / chairs.size()); 
        int which_origin = idx % chairs.size(); 
        
        //cout << idx << " | changed seed - " << which_origin << " | fixed seed idx - " << which_record << " | fixed seed - " << record(which_record, 0) << endl;
        int seat_id = rand() % chairs.size();
        int leg_id = rand() % chairs.size();
        int back_id = rand() % chairs.size();
        int arm_id = rand() % chairs.size();

        if (level == 1) { // leg
            seat_id = record(which_record, 0);
            leg_id = which_origin;
            arm_id = record(which_record, 0);
            back_id = record(which_record, 0);
        } else if (level == 2){ // arm
            seat_id = record(which_record, 0);
            leg_id = record(which_record, 1);
            arm_id = which_origin;
            back_id = record(which_record, 0);
        } else if (level == 3) {
            seat_id = record(which_record, 0);
            leg_id = record(which_record, 1);
            arm_id = record(which_record, 2);
            back_id = which_origin;
        } 

        //cout << "seat - " << seat_id << " | leg - " << leg_id << " | arm - " << arm_id <<" | back - " << back_id << endl;

        ChairBuffer chair1 = chairs[seat_id];
        ChairBuffer chair2 = chairs[leg_id];
        ChairBuffer chair3 = chairs[back_id];
        ChairBuffer chair4 = chairs[arm_id];

        return mix(chair1.seat, chair2.leg, chair3.back, chair4.arm);
    }

    // Record Method
    void updateRecord(int level, vector<int> selected_idx) {
        MatrixXi temp = record;
        
        for (int i = 0; i < selected_idx.size(); i++) {
            int which_record = int(selected_idx[i] / chairs.size()); 
            int which_origin = selected_idx[i] % chairs.size(); 

            for (int j = 0; j < level; j++) {
                record(i, j) = temp(which_record, j);
            }

            record(i, level) = which_origin;
        }

        cout << "=== Record ===" << endl;
        cout << record << endl;
    }

    void printIntVector(vector<int> v, string name) {
        cout << name << " -";
        for (auto i : v) {
            cout << " " << i;
        }
        cout << endl;
    }

private:
    void transformLeg(ChairPartBuffer& seat, ChairPartBuffer& leg);
    void transformBack(ChairPartBuffer& seat, ChairPartBuffer& back);
    void transformArm(ChairPartBuffer& seat, ChairPartBuffer& back, ChairPartBuffer& arm);
};
#endif // CHAIRMIXER_H