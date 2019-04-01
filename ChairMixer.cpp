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

    float legScaleX = seat.width / leg.width;
    float legScaleY = seat.depth / leg.depth;
    float legScaleZ = (legScaleX + legScaleY) / 2;
    for (int i = legVI; i < backVI; i++) {
        Vector3f offset = mixed.vertices[i] - leg.bottomCenter;
        mixed.vertices[i] = Vector3f(offset.x() * legScaleX,
                                     offset.y() * legScaleY,
                                     offset.z() * legScaleZ)
                            + seat.bottomCenter;
    }

    float backScale = seat.width / back.width;
    for (int i = backVI; i < armVI; i++) {
        Vector3f offset = (mixed.vertices[i] - back.backCenter) * backScale;
        mixed.vertices[i] = offset + seat.backCenter;
    }

    float armScaleX = seat.width / arm.width;
    float armScaleY = seat.depth / arm.depth;
    float armScaleZ = (armScaleX + armScaleY) / 2;
    for (int i = armVI; i < endVI; i++) {
        Vector3f offset = mixed.vertices[i] - arm.topCenter;
        mixed.vertices[i] = Vector3f(offset.x() * armScaleX,
                                     offset.y() * armScaleY,
                                     offset.z() * armScaleZ)
                            + seat.topCenter;
    }

    return mixed;
}