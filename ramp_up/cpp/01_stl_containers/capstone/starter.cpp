// DRILL BRIEF
// Concept: Triage robot telemetry through parsing, safe aggregation, and a deterministic summary.
// Scenario: Read stdin telemetry lines "<joint> <position> <temperature>" and select one report stage with argv[1]: stage 1 parses and counts lines, stage 2 clamps and groups positions, and stage 3 reports joint count and hottest means.
// Implement: the capstone program in main.
// Behavior: stage 1 prints "parsed=6 skipped=2"; stage 2 prints "arm_elbow max_pos=1.57 clamped=1", "arm_shoulder max_pos=1.57 clamped=1", "arm_wrist max_pos=0.10 clamped=0", then "base_yaw max_pos=0.30 clamped=0"; stage 3 prints "joints=4 hottest=head_pan,arm_elbow".
// Interview focus: Build a small pipeline whose parsing, safety bounds, and reports have an exact text contract.
// Tests: fixtures/stage{1,2,3}_input.txt are compared exactly with the matching stage{1,2,3}_expected.txt outputs; malformed lines are skipped, never fatal.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/capstone -q
// Done when: All three fixture stages pass; stage is selected by argv[1], and each succeeds with exit code 0 and its exact expected output.

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
