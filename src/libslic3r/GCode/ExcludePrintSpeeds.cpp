#include "ExcludePrintSpeeds.hpp"

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <regex>

namespace Slic3r {

// TODO - CHKA: Clean code; extract functions
ExcludePrintSpeeds::ExcludePrintSpeeds(const std::string &_forbidden_ranges_user_input,
    const ConfigOptionEnum<ExcludePrintSpeedsAdjustmentDirection> &_adjustment_direction)
{
    adjustment_direction                    = _adjustment_direction;
    std::string forbidden_ranges_user_input = _forbidden_ranges_user_input;

    forbidden_ranges_user_input.erase(std::remove_if(forbidden_ranges_user_input.begin(),
                                                     forbidden_ranges_user_input.end(), ::isspace),
                                      forbidden_ranges_user_input.end());

    std::vector<std::string> forbidden_ranges_strings;
    boost::split(forbidden_ranges_strings, forbidden_ranges_user_input, boost::is_any_of(","));

    // Parse input (check string regex and convert to numeric)
    auto numeric_range_regex = std::regex("^(\\d+)-(\\d+)$");
    for (const auto &elem : forbidden_ranges_strings) {
        std::smatch regex_match;
        if (!std::regex_match(elem, regex_match, numeric_range_regex)) {
            throw Slic3r::SlicingError("Invalid range " + elem +
                                       ". Range must have start and end values,"
                                       " separated by \"-\" (example: 30 - 50)");
        }

        constexpr size_t LOWER_BOUND_MATCH_INDEX = 1;
        constexpr size_t UPPER_BOUND_MATCH_INDEX = 2;
        auto             lower_bound             = std::stoi(regex_match[LOWER_BOUND_MATCH_INDEX]);
        auto             higher_bound            = std::stoi(regex_match[UPPER_BOUND_MATCH_INDEX]);
        if (lower_bound >= higher_bound) {
            throw Slic3r::SlicingError("Invalid range " + elem + ". Upper bound must be greater than lower bound.");
        }
        forbidden_ranges.emplace_back(lower_bound, higher_bound);
    }

    // Check range consistency (Simply check for overlap. User can enter them in non-ascending lower bound order)

    std::sort(forbidden_ranges.begin(), forbidden_ranges.end(),
              [](auto &left, auto &right) { return left.first < right.first; });

    for (size_t i = 1; i < forbidden_ranges.size(); i++) {
        if (forbidden_ranges[i].first < forbidden_ranges[i - 1].second) {
            throw Slic3r::SlicingError("Ranges " + forbidden_ranges_strings[i - 1] + " and " +
                                       forbidden_ranges_strings[i] + " overlap.");
        }
    }
}


double_t ExcludePrintSpeeds::adjust_speed_if_in_forbidden_range(double speed)
{
    for (auto range : forbidden_ranges) {
        if (speed > range.first && speed < range.second) {
            switch (adjustment_direction) {
            case epsdLowest:
                speed = range.first;
                break;
            case epsdHighest:
                speed = range.second;
                break;
            case epsdNearest:
                if ((speed - range.first) < (range.second - speed)) {
                    speed = range.first;
                } else {
                    speed = range.second;
                }
                break;
            default:
                return speed;
            }

            return speed;
        }
    }

    return speed;
}

}