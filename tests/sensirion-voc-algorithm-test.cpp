#include "CppUTest/CommandLineTestRunner.h"
#include "sensirion_voc_algorithm.h"

TEST_GROUP (Sgp40VocIndexAlgorithmTest) {};

TEST (Sgp40VocIndexAlgorithmTest, returns_zero_during_blackout) {
    VocAlgorithmParams params;
    VocAlgorithm_init(&params);
    for (int i = 0; i < VocAlgorithm_INITIAL_BLACKOUT; ++i) {
        int32_t voc_index;
        VocAlgorithm_process(&params, 0, &voc_index);
        CHECK_EQUAL_TEXT(0, voc_index,
                         "VOC index should be 0 during initial blackout");
    }
}

TEST (Sgp40VocIndexAlgorithmTest, returns_average_after_blackout) {
    VocAlgorithmParams params;
    VocAlgorithm_init(&params);
    int32_t voc_index;
    for (int i = 0; i < VocAlgorithm_INITIAL_BLACKOUT + 2; ++i) {
        VocAlgorithm_process(&params, 0, &voc_index);
    }

    CHECK_EQUAL_TEXT(
        VocAlgorithm_VOC_INDEX_OFFSET_DEFAULT, voc_index,
        "VOC index should be the offset default after the the blackout period");
}

int main(int argc, char** argv) {
    return CommandLineTestRunner::RunAllTests(argc, argv);
}
