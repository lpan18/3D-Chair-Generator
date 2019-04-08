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
    vector<ObjBuffer*> buffers;
    buffers.push_back(&seat);
    buffers.push_back(&leg);
    buffers.push_back(&back);
    buffers.push_back(&arm);

    ObjBuffer mixed = ObjBuffer::combineObjBuffers(buffers);

    transformLeg(seat, leg);
    transformBack(seat, back);
    transformArm(seat, arm);

    return mixed;
}

void ChairMixer::transformLeg(ChairPartBuffer& seat, ChairPartBuffer& leg) {
    float legScaleX = seat.origSeatFeatures.width / leg.origSeatFeatures.width;
    float legScaleY = seat.origSeatFeatures.depth / leg.origSeatFeatures.depth;
    float legScaleZ = (legScaleX + legScaleY) / 2;
    Matrix3f scale;
    scale << legScaleX, 0, 0,
            0, legScaleY, 0,
            0, 0, legScaleZ;

    for (int i = 0; i < leg.nVertices; i++) {
        leg.vertices[i] = ChairPartOrigSeatFeatures::transform(scale, leg.vertices[i], leg.origSeatFeatures.bottomCenter, seat.origSeatFeatures.bottomCenter);
    }

    leg.resetPartFeatures();
}

void ChairMixer::transformBack(ChairPartBuffer& seat, ChairPartBuffer& back) {
    float backScale = seat.origSeatFeatures.width / back.origSeatFeatures.width;
    Matrix3f scale;
    scale << backScale, 0, 0,
            0, backScale, 0,
            0, 0, backScale;
    
    for (int i = 0; i < back.nVertices; i++) {
        back.vertices[i] = ChairPartOrigSeatFeatures::transform(scale, back.vertices[i], back.origSeatFeatures.backTopCenter, seat.origSeatFeatures.backTopCenter);
    }

    back.resetPartFeatures();
}

void ChairMixer::transformArm(ChairPartBuffer& seat, ChairPartBuffer& arm) {
    float armScaleX = seat.origSeatFeatures.width / arm.origSeatFeatures.width;
    float armScaleY = seat.origSeatFeatures.depth / arm.origSeatFeatures.depth;
    float armScaleZ = (armScaleX + armScaleY) / 2;
    Matrix3f scale;
    scale << armScaleX, 0, 0,
            0, armScaleY, 0,
            0, 0, armScaleZ;
    
    for (int i = 0; i < arm.nVertices; i++) {
        arm.vertices[i] = ChairPartOrigSeatFeatures::transform(scale, arm.vertices[i], arm.origSeatFeatures.topCenter, seat.origSeatFeatures.topCenter);
    }

    arm.resetPartFeatures();
}