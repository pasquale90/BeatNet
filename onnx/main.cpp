#include "BeatNet.h"
#include <iostream>

float randomFloatGenerator() {
    return static_cast<float>(rand()) / RAND_MAX;
}

int main() {
    
    BeatNet tracker;
    tracker.setup(44000, 512);

    // just for testing things out...
    tracker.setup(1,1);
    tracker.setup(0,0);
    tracker.setup(96000, 256);

    std::vector<float> raw_input(256);
    std::vector<float> output(3);

    for (int i = 0; i < 30; ++i) {
        std::generate(raw_input.begin(), raw_input.end(), randomFloatGenerator);

        if (tracker.process(raw_input, output)) {
            std::cout << "BeatNet Output: [";
            for (float val : output) {
                std::cout << val << " ";
            }
            std::cout << "]\n";
        }
    }
    return 0;
}
