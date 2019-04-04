#include <iostream>
#include <string>
#include <dirent.h>
#include "ChairMixer.h"

using namespace std;

void ChairMixer::readFolder(string path) {
    char *cstr = &path[0u];
    if (auto dir = opendir(cstr)) {
        while (auto f = readdir(dir)) {
            if (!f->d_name || f->d_name[0] == '.') {
                continue; // Skip everything that starts with a dot
            }

            ChairBuffer chair = ChairBuffer::readObjFile(path + "/" + f->d_name);
            chairs.push_back(chair);
        }
        closedir(dir);
    }
}
// temp code start ===============
float getNeg() {
    float neg = rand() / (float)RAND_MAX * 0.1;
    if (neg > 0.05f) {
        return neg + 0.1f; // 0.15-0.2
    } else {
        return neg - 0.2f; //-0.15--0.2
    }
}

float getNegScale() {
    float negScale = rand() / (float)RAND_MAX * 0.2 + 0.9;  // map to 0.9-1.1
    return negScale;
}

Vector3f getNegOffset(){
    return Vector3f(getNeg(), getNeg(), getNeg());
}

// temp code end ============

ObjBuffer ChairMixer::mix(ChairPartBuffer seat, ChairPartBuffer leg, ChairPartBuffer back, ChairPartBuffer arm) {
    vector<ObjBuffer> buffers;
    buffers.push_back(seat);
    buffers.push_back(leg);
    buffers.push_back(back);
    buffers.push_back(arm);

    ObjBuffer mixed = ObjBuffer::combineObjBuffers(buffers);
    
    int legVI = seat.nVertices;
    int backVI = legVI + leg.nVertices;
    int armVI = backVI + back.nVertices;
    int endVI = armVI + arm.nVertices;

// temp code start ===============
    Vector3f negOffset1 = getNegOffset() * (seat.width + seat.depth)/2;
    Vector3f negOffset2 = getNegOffset() * (seat.width + seat.depth)/2;
    Vector3f negOffset3 = getNegOffset() * (seat.width + seat.depth)/2;

    float negScale1 = getNegScale();
    float negScale2 = getNegScale();
    float negScale3 = getNegScale();
    if(negOffset2.y() <= 0){
        negOffset2.y() -= 0.25;
    }
    if(negOffset2.z() <= 0){
        negOffset2.z() -= 0.25;
    }

// temp code end ============

    float legScaleX = seat.width / leg.width;
    float legScaleY = seat.depth / leg.depth;
    float legScaleZ = (legScaleX + legScaleY) / 2;
    for (int i = legVI; i < backVI; i++) {
        Vector3f offset = mixed.vertices[i] - leg.bottomCenter;
        // offset = offset * negScale1 + negOffset1; // temp code
        mixed.vertices[i] = Vector3f(offset.x() * legScaleX,
                                     offset.y() * legScaleY,
                                     offset.z() * legScaleZ)
                            + seat.bottomCenter;
    }

    float backScale = seat.width / back.width;
    for (int i = backVI; i < armVI; i++) {
        Vector3f offset = (mixed.vertices[i] - back.backCenter) * backScale;
        offset = offset * negScale2 + negOffset2; // temp code
        mixed.vertices[i] = offset + seat.backCenter;
    }

    float armScaleX = seat.width / arm.width;
    float armScaleY = seat.depth / arm.depth;
    float armScaleZ = (armScaleX + armScaleY) / 2;
    for (int i = armVI; i < endVI; i++) {
        Vector3f offset = mixed.vertices[i] - arm.topCenter;
        // offset = offset * negScale3 + negOffset3; // temp code
        mixed.vertices[i] = Vector3f(offset.x() * armScaleX,
                                     offset.y() * armScaleY,
                                     offset.z() * armScaleZ)
                            + seat.topCenter;
    }

    return mixed;
}