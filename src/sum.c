#include "../include/sum.h"

float sum(float numbers[], int count) {
    float totalSum = 0.0f;
    for (int i = 0; i < count; i++) {
        totalSum += numbers[i];
    }
    return totalSum;
}