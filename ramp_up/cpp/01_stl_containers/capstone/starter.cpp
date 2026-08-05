#include <iostream>
#include <string>

// Telemetry triage — the module 01 capstone. Spec: README.md
//
// Build it stage by stage:
//   stage 1: parse stdin lines "<joint> <position> <temperature>"
//            -> print "parsed=N skipped=M"
//   stage 2: clamp positions to [-1.57, 1.57], group by joint
//            -> per joint: "<joint> max_pos=X.XX clamped=K"
//   stage 3: distinct joints + two hottest by mean temperature
//            -> "joints=N hottest=a,b"
//
// Run one stage by hand:  ./capstone 1 < fixtures/stage1_input.txt

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: capstone <stage>\n";
        return 2;
    }
    const int stage = std::stoi(argv[1]);
    (void)stage;
    // Your program starts here.
    return 0;
}
