#ifndef slic3r_GCode_SmallAreaInfillFlowCompensator_hpp_
#define slic3r_GCode_SmallAreaInfillFlowCompensator_hpp_

#include "../libslic3r.h"
#include "../PrintConfig.hpp"
#include "../ExtrusionEntity.hpp"
#include "spline/spline.h"

namespace Slic3r {

namespace GCode {


class SmallAreaInfillFlowCompensator
{
private:
    // Model points
    std::vector<double> extrusionLengths;
    std::vector<double> flowCompensationFactors;

    tk::spline flowModel;
    
private:
    double flow_comp_model(const double line_length);

    double max_modified_length() { return extrusionLengths.back(); }

public:
    explicit SmallAreaInfillFlowCompensator(const Slic3r::GCodeConfig &config);

    double modify_flow(const double line_length, const double dE, const ExtrusionRole role);
};

} // namespace Slic3r

} // namespace GCode

#endif /* slic3r_GCode_SmallAreaInfillFlowCompensator_hpp_ */
