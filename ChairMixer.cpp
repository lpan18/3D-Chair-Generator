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

void ChairMixer::free() {
    for (auto c : chairs) {
        c.free();
    }
}

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

    transformLeg(mixed, seat, leg, legVI);
    transformBack(mixed, seat, back, backVI);
    transformArm(mixed, seat, arm, armVI);

    return mixed;
}

Vector3f ChairMixer::transformBySeatFeatures(Matrix3f scale, Vector3f v, Vector3f oldBase, Vector3f newBase) {
    Vector3f offset = v - oldBase;
    return scale * offset + newBase;
}

void ChairMixer::transformLeg(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& leg, int legVI) {
    float legScaleX = seat.origSeatFeatures.width / leg.origSeatFeatures.width;
    float legScaleY = seat.origSeatFeatures.depth / leg.origSeatFeatures.depth;
    float legScaleZ = (legScaleX + legScaleY) / 2;
    Matrix3f scale;
    scale << legScaleX, 0, 0,
            0, legScaleY, 0,
            0, 0, legScaleZ;

    for (int i = legVI; i < legVI + leg.nVertices; i++) {
        mixed.vertices[i] = transformBySeatFeatures(scale, mixed.vertices[i], leg.origSeatFeatures.bottomCenter, seat.origSeatFeatures.bottomCenter);
    }
}

void ChairMixer::transformBack(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& back, int backVI) {
    float backScale = seat.origSeatFeatures.width / back.origSeatFeatures.width;
    Matrix3f scale;
    scale << backScale, 0, 0,
            0, backScale, 0,
            0, 0, backScale;
    
    for (int i = backVI; i < backVI + back.nVertices; i++) {
        mixed.vertices[i] = transformBySeatFeatures(scale, mixed.vertices[i], back.origSeatFeatures.backTopCenter, seat.origSeatFeatures.backTopCenter);
    }
}

void ChairMixer::transformArm(ObjBuffer& mixed, ChairPartBuffer& seat, ChairPartBuffer& arm, int armVI) {
    float armScaleX = seat.origSeatFeatures.width / arm.origSeatFeatures.width;
    float armScaleY = seat.origSeatFeatures.depth / arm.origSeatFeatures.depth;
    float armScaleZ = (armScaleX + armScaleY) / 2;
    Matrix3f scale;
    scale << armScaleX, 0, 0,
            0, armScaleY, 0,
            0, 0, armScaleZ;
    
    for (int i = armVI; i < armVI + arm.nVertices; i++) {
        mixed.vertices[i] = transformBySeatFeatures(scale, mixed.vertices[i], arm.origSeatFeatures.topCenter, seat.origSeatFeatures.topCenter);
    }
}