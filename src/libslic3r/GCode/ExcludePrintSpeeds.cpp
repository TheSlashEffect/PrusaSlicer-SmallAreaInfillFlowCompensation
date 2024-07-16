#include "ExcludePrintSpeeds.hpp"

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <regex>

namespace Slic3r {

ExcludePrintSpeeds::ExcludePrintSpeeds(const std::string &_forbidden_ranges_user_input,
                                       bool               _move_to_lowest_available_speed)
{
    std::cout << "chka46: initializing ExcludePrintSpeeds" << std::endl;
    move_to_lowest_allowed_speed            = _move_to_lowest_available_speed;
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
            std::cout << "Range element " << elem << " does not match int-int format" << std::endl;
            // return speed;
            return; // TODO
        }

        constexpr size_t LOWER_BOUND_MATCH_INDEX = 1;
        constexpr size_t UPPER_BOUND_MATCH_INDEX = 2;
        auto             lower_bound             = std::stoi(regex_match[LOWER_BOUND_MATCH_INDEX]);
        auto             higher_bound            = std::stoi(regex_match[UPPER_BOUND_MATCH_INDEX]);
        if (lower_bound >= higher_bound) {
            std::cout << "Invalid range: " << elem << ". Upper bound must be greater than lower bound." << std::endl;
            return; // TODO
        }
        forbidden_ranges.emplace_back(lower_bound, higher_bound);
    }

    // Check range consistency (Simply check for overlap. User can enter them in non-ascending lower bound order)

    std::sort(forbidden_ranges.begin(), forbidden_ranges.end(),
              [](auto &left, auto &right) { return left.first < right.first; });

    for (size_t i = 1; i < forbidden_ranges.size(); i++) {
        int range_start_front = forbidden_ranges[i].first;
        int range_end_back    = forbidden_ranges[i - 1].second;
        if (range_start_front < range_end_back) {
            return; // TODO
        }
    }
}


double_t ExcludePrintSpeeds::adjust_speed_if_in_forbidden_range(double speed)
{
    for (auto range : forbidden_ranges) {
        if (speed > range.first && speed < range.second) {
            speed = (move_to_lowest_allowed_speed) ? range.first : range.second;
            return speed;
        }
    }

    return speed;
}

}