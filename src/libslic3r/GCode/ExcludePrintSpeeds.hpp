#ifndef slic3r_ExcludePrintSpeeds_hpp_
#define slic3r_ExcludePrintSpeeds_hpp_

#include "../Print.hpp"

#include <cmath>

namespace Slic3r {

class ExcludePrintSpeeds
{
private:
static constexpr size_t LOWER_BOUND_MATCH_INDEX = 1;
static constexpr size_t UPPER_BOUND_MATCH_INDEX = 2;

    std::vector<std::pair<int, int>> forbidden_ranges;
    ConfigOptionEnum<ExcludePrintSpeedsAdjustmentDirection> adjustment_direction;

public:
    ExcludePrintSpeeds(const std::string &_forbidden_ranges_user_input,
                       const ConfigOptionEnum<ExcludePrintSpeedsAdjustmentDirection> &_adjustment_direction);

    double_t adjust_speed_if_in_forbidden_range(double speed);
};

}

#endif // slic3r_ExcludePrintSpeeds_hpp_