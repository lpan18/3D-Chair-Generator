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
    return mixed;
}