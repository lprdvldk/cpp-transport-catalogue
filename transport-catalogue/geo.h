#pragma oncea

namespace geo {

struct Coordinates {
    double lat; // Широта
    double lng; // Долгота
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo