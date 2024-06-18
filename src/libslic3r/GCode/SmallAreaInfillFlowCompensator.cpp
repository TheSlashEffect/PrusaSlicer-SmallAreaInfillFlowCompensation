#include "SmallAreaInfillFlowCompensator.hpp"

#include "../libslic3r.h"

namespace Slic3r{
	
namespace GCode {

bool nearly_equal_floating_point(double a, double b) {
    return std::nextafter(a, std::numeric_limits<double>::lowest()) <= b &&
        std::nextafter(a, std::numeric_limits<double>::max()) >= b;
}

SmallAreaInfillFlowCompensator::SmallAreaInfillFlowCompensator(const Slic3r::GCodeConfig &config) {
    for (auto &line : config.small_area_infill_flow_compensation_model.values) {
        std::istringstream iss(line);
        std::string value_str;
        double extrusion_length = 0.0;

        if (std::getline(iss, value_str, ',')) {
            try {
                extrusion_length = std::stod(value_str);
                if (std::getline(iss, value_str, ',')) {
                    extrusionLengths.push_back(extrusion_length);
                    flowCompensationFactors.push_back(std::stod(value_str));
                }
            } catch (...) {
                std::stringstream ss;
                ss << "Error parsing data point in small area infill compensation model:" << line
                   << std::endl;

                throw Slic3r::InvalidArgument(ss.str());
            }
        }
    }

    for (int i = 0; i < extrusionLengths.size(); i++) {
        if (i == 0) {
            if (!nearly_equal_floating_point(extrusionLengths[i], 0.0)) {
                throw Slic3r::InvalidArgument(
                    "First extrusion length for small area infill compensation model must be 0"
                );
            }
        } else {
            if (nearly_equal_floating_point(extrusionLengths[i], 0.0)) {
                throw Slic3r::InvalidArgument("Only the first extrusion length for small area "
                                              "infill compensation model can be 0");
            }
            if (extrusionLengths[i] <= extrusionLengths[i - 1]) {
                throw Slic3r::InvalidArgument(
                    "Extrusion lengths for subsequent points must be increasing"
                );
            }
        }
    }

    if (!flowCompensationFactors.empty() && !nearly_equal_floating_point(flowCompensationFactors.back(), 1.0)) {
        throw Slic3r::InvalidArgument(
            "Final compensation factor for small area infill flow compensation model must be 1.0"
        );
    }
    flowModel.set_points(extrusionLengths, flowCompensationFactors);
}

double SmallAreaInfillFlowCompensator::flow_comp_model(const double line_length) {
    if (line_length == 0 || line_length > max_modified_length()) {
        return 1.0;
    }

    return flowModel(line_length);
}

double SmallAreaInfillFlowCompensator::modify_flow(
    const double line_length, const double dE, const ExtrusionRole role
) {
    if (role == ExtrusionRole::SolidInfill|| role == ExtrusionRole::TopSolidInfill) {
        return dE * flow_comp_model(line_length);
    }

    return dE;
}

} // namespace GCode

} // namespace Slic3r