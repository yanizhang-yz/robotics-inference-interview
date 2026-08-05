#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr double kPositionLimit = 1.57;

struct Sample {
    std::string joint;
    double position = 0.0;
    double temperature = 0.0;
};

struct ParsedStream {
    std::vector<Sample> samples;
    int skipped = 0;
};

ParsedStream parse_stream(std::istream& in) {
    ParsedStream result;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream fields(line);
        Sample sample;
        std::string extra;
        const bool three_fields =
            static_cast<bool>(fields >> sample.joint >> sample.position >>
                              sample.temperature);
        if (three_fields && !(fields >> extra)) {
            result.samples.push_back(sample);
        } else {
            ++result.skipped;
        }
    }
    return result;
}

void print_parse_report(const ParsedStream& stream) {
    std::cout << "parsed=" << stream.samples.size()
              << " skipped=" << stream.skipped << "\n";
}

void print_clamp_report(std::vector<Sample>& samples) {
    struct JointStats {
        double max_abs_position = 0.0;
        int clamped = 0;
    };
    std::map<std::string, JointStats> per_joint;
    for (Sample& sample : samples) {
        const double clamped_position =
            std::clamp(sample.position, -kPositionLimit, kPositionLimit);
        JointStats& stats = per_joint[sample.joint];
        if (clamped_position != sample.position) {
            ++stats.clamped;
        }
        sample.position = clamped_position;
        stats.max_abs_position =
            std::max(stats.max_abs_position, std::abs(sample.position));
    }
    for (const auto& entry : per_joint) {
        std::cout << entry.first << " max_pos=" << entry.second.max_abs_position
                  << " clamped=" << entry.second.clamped << "\n";
    }
}

void print_triage_report(const std::vector<Sample>& samples) {
    struct TempStats {
        double sum = 0.0;
        int count = 0;
    };
    std::map<std::string, TempStats> per_joint;
    std::set<std::string> joints;
    for (const Sample& sample : samples) {
        joints.insert(sample.joint);
        TempStats& stats = per_joint[sample.joint];
        stats.sum += sample.temperature;
        ++stats.count;
    }
    std::vector<std::pair<std::string, double>> means;
    means.reserve(per_joint.size());
    for (const auto& entry : per_joint) {
        means.emplace_back(entry.first, entry.second.sum / entry.second.count);
    }
    std::sort(means.begin(), means.end(),
              [](const auto& left, const auto& right) {
                  if (left.second != right.second) {
                      return left.second > right.second;
                  }
                  return left.first < right.first;
              });
    std::cout << "joints=" << joints.size() << " hottest=";
    const std::size_t hottest = std::min<std::size_t>(2, means.size());
    for (std::size_t i = 0; i < hottest; ++i) {
        std::cout << (i ? "," : "") << means[i].first;
    }
    std::cout << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: capstone <stage>\n";
        return 2;
    }
    const int stage = std::stoi(argv[1]);
    std::cout << std::fixed << std::setprecision(2);
    ParsedStream stream = parse_stream(std::cin);
    if (stage == 1) {
        print_parse_report(stream);
    } else if (stage == 2) {
        print_clamp_report(stream.samples);
    } else if (stage == 3) {
        print_triage_report(stream.samples);
    } else {
        std::cerr << "unknown stage " << stage << "\n";
        return 2;
    }
    return 0;
}
