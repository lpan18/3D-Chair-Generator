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
    float neg = rand() / (float)RAND_MAX;
    if (neg > 0.5f) {
        return neg - 0.4f;
    } else {
        return neg - 0.6f;
    }
}

float getNegScale() {
    float negScale = rand() / (float)RAND_MAX * 0.2 + 0.9;  // map to 0.9-1.1
    return 1; //negScale;
}

Vector3f getNegOffset(){
    return Vector3f(getNeg(), getNeg(), getNeg());
}

// temp code end ============

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
    transformArm(seat, back, arm);

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

    // temp code start ===============
    Vector3f negOffset1 = getNegOffset() * (seat.origSeatFeatures.width + seat.origSeatFeatures.depth)/2;
    float negScale1 = getNegScale();
    // temp code end ============

    for (int i = 0; i < leg.nVertices; i++) {
        // change code for neg
        leg.vertices[i] = ChairPartOrigSeatFeatures::transform(scale, negScale1, negOffset1, leg.vertices[i], leg.origSeatFeatures.bottomCenter, seat.origSeatFeatures.bottomCenter);
    }

    leg.resetBound();
    leg.resetPartFeatures();

    float legDistFrontBack = abs(leg.partFeatures.topRightBack.y() - leg.partFeatures.topRightFront.y());
    float legDistLeftRight = abs(leg.partFeatures.topRightBack.x() - leg.partFeatures.topLeftBack.x());

    if (legDistFrontBack / seat.origSeatFeatures.depth < LEG_DIST_THLD) {
        if (legDistLeftRight / seat.origSeatFeatures.width < LEG_DIST_THLD) {
            // In this case, the legs are considered as one whole entity.
            Vector3f pb(leg.bound.getCenter().x(), leg.bound.getCenter().y(), leg.bound.minZ);
            Vector3f p0 = leg.partFeatures.topRightBack;
            Vector3f p1 = seat.getClosestPointTo(p0);
            leg.transformSingle(pb, p0, p1);
        } else {
            // In this case, apply symmetrical transformation on x axis.
            Vector3f pb(leg.bound.getCenter().x(), leg.bound.getCenter().y(), leg.bound.minZ);
            Vector3f p0 = leg.partFeatures.topRightBack;
            Vector3f p1 = seat.getClosestPointTo(p0);
            leg.transformSingleXSym(pb, p0, p1);
        }

        leg.resetBound();
        leg.resetPartFeatures();

        leg.align(seat.origSeatFeatures.bottomCenter);

    } else {
        // In this case, we classify legs as back legs and front legs
        Vector3f pb(leg.bound.getCenter().x(), leg.bound.getCenter().y(), leg.bound.minZ);
        Vector3f p0 = leg.partFeatures.topRightBack;
        Vector3f p1 = seat.getClosestPointTo(p0);
        Vector3f q0 = leg.partFeatures.topRightFront;
        Vector3f q1 = seat.getClosestPointTo(q0);
        leg.transformDouleXSym(pb, p0, p1, q0, q1);
    }
    
    leg.resetBound();
    leg.resetPartFeatures();
}

void ChairMixer::transformBack(ChairPartBuffer& seat, ChairPartBuffer& back) {
    float backScale = seat.origSeatFeatures.width / back.origSeatFeatures.width;
    Matrix3f scale;
    scale << backScale, 0, 0,
            0, backScale, 0,
            0, 0, backScale;

    // temp code start ===============
    Vector3f negOffset2 = getNegOffset() * (seat.origSeatFeatures.width + seat.origSeatFeatures.depth)/2;
    float negScale2 = getNegScale();
    // if(negOffset2.y() <= 0){
    //     negOffset2.y() -= 0.25;
    // }
    // if(negOffset2.z() <= 0){
    //     negOffset2.z() -= 0.25;
    // }
    // temp code end ============

    for (int i = 0; i < back.nVertices; i++) {
        // change code for neg
        back.vertices[i] = ChairPartOrigSeatFeatures::transform(scale, negScale2, negOffset2, back.vertices[i], back.origSeatFeatures.backTopCenter, seat.origSeatFeatures.backTopCenter);
    }

    back.resetBound();
    back.resetPartFeatures();

    Vector3f pb(back.bound.getCenter().x(), back.bound.getCenter().y(), back.bound.getCenter().z());
    Vector3f p0 = back.partFeatures.bottomRightBack;
    Vector3f p1 = seat.getClosestPointTo(p0);
    back.transformSingleXSym(pb, p0, p1);
}

void ChairMixer::transformArm(ChairPartBuffer& seat, ChairPartBuffer& back, ChairPartBuffer& arm) {
    float armScaleX = seat.origSeatFeatures.width / arm.origSeatFeatures.width;
    float armScaleY = seat.origSeatFeatures.depth / arm.origSeatFeatures.depth;
    float armScaleZ = (armScaleX + armScaleY) / 2;
    Matrix3f scale;
    scale << armScaleX, 0, 0,
            0, armScaleY, 0,
            0, 0, armScaleZ;
    
    // temp code start ===============
    Vector3f negOffset3 = getNegOffset() * (seat.origSeatFeatures.width + seat.origSeatFeatures.depth)/2;
    float negScale3 = getNegScale();
    // temp code end ============

    for (int i = 0; i < arm.nVertices; i++) {
        // change code for neg
        arm.vertices[i] = ChairPartOrigSeatFeatures::transform(scale, negScale3, negOffset3, arm.vertices[i], arm.origSeatFeatures.topCenter, seat.origSeatFeatures.topCenter);
    }

    arm.resetBound();
    arm.resetPartFeatures();

    Vector3f pb(arm.bound.getCenter().x(), arm.bound.getCenter().y(), arm.bound.getCenter().z());
    Vector3f p0 = arm.partFeatures.topRightBack;
    Vector3f p1 = back.getClosestPointTo(p0);
    Vector3f q0 = arm.partFeatures.topRightFront;
    Vector3f q1 = seat.getClosestPointTo(q0);
    arm.transformDouleXSym(pb, p0, p1, q0, q1, false);

    arm.resetBound();
    arm.resetPartFeatures();

}