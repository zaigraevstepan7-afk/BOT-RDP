#pragma once
#include "quaternion.h"

float normalizeAngle(float angle) {
    while (angle > 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;
    return angle;
}

Vector3 normalizeAngles(Vector3 angles) {
    angles.x = fmodf(angles.x + 180.0f, 360.0f) - 180.0f;
    angles.y = fmodf(angles.y + 180.0f, 360.0f) - 180.0f;
    angles.z = fmodf(angles.z + 180.0f, 360.0f) - 180.0f;
    return angles;
}

Vector3 lerpAngles(Vector3 angle, Vector3 bngle, float time) {
    Vector3 delta;
    delta.x = fmodf(bngle.x - angle.x + 180.0f, 360.0f) - 180.0f;
    delta.y = fmodf(bngle.y - angle.y + 180.0f, 360.0f) - 180.0f;
    delta.z = fmodf(bngle.z - angle.z + 180.0f, 360.0f) - 180.0f;
    Vector3 result;
    result.x = angle.x + delta.x * (1.0f - expf(-time * 10.0f));
    result.y = angle.y + delta.y * (1.0f - expf(-time * 10.0f));
    result.z = angle.z + delta.z * (1.0f - expf(-time * 10.0f));
    return normalizeAngles(result);
}

Vector3 toEulerRad(Quaternion quaternion) {
    float sqW = quaternion.w * quaternion.w;
    float sqX = quaternion.x * quaternion.x;
    float sqY = quaternion.y * quaternion.y;
    float sqZ = quaternion.z * quaternion.z;
    float unit = sqX + sqY + sqZ + sqW;
    float singCheck = quaternion.x * quaternion.w - quaternion.y * quaternion.z;
    if (singCheck > 0.4995f * unit) {
        return Vector3(90.0f, 2.0f * atan2f(quaternion.y, quaternion.x) * 57.29578f, 0.0f);
    }
    if (singCheck < -0.4995f * unit) {
        return Vector3(-90.0f, -2.0f * atan2f(quaternion.y, quaternion.x) * 57.29578f, 0.0f);
    }
    Quaternion q_normalized(quaternion.w, quaternion.z, quaternion.x, quaternion.y);
    Vector3 euler;
    euler.y = atan2f(2.0f * q_normalized.x * q_normalized.w + 2.0f * q_normalized.y * q_normalized.z, 1.0f - 2.0f * (q_normalized.z * q_normalized.z + q_normalized.w * q_normalized.w));
    euler.x = asinf(2.0f * (q_normalized.x * q_normalized.z - q_normalized.w * q_normalized.y));
    euler.z = atan2f(2.0f * q_normalized.x * q_normalized.y + 2.0f * q_normalized.z * q_normalized.w, 1.0f - 2.0f * (q_normalized.y * q_normalized.y + q_normalized.z * q_normalized.z));
    return normalizeAngles(euler * 57.29578f);
}

Vector3 calculateBestAngles(Vector3 enemyPosition, Vector3 localPosition) {
    Vector3 dir = enemyPosition - localPosition;
    Quaternion lookRotation = Quaternion::LookRotation(dir.normalized(), Vector3(0, 1, 0));
    return toEulerRad(lookRotation);
}
