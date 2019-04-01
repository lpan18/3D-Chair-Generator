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
};
#endif // CHAIRMIXER_H