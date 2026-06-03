#ifndef SIM_ENUMS_HPP
#define SIM_ENUMS_HPP

// Which spatial dimension the simulation runs in.
enum class SimDimension { DIM_2D, DIM_3D };

// Which physics domain an object belongs to.
enum class SimDomain { RIGID, FLUID, LIGHT };

#endif