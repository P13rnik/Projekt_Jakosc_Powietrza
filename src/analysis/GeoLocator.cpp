#include "analysis/GeoLocator.hpp"
#include <cmath>
#include <algorithm>

double GeoLocator::haversineDistance(double lat1, double lon1,
                                      double lat2, double lon2) {
    // Wzór haversine - odległość na sferze (Ziemia)
    auto toRad = [](double deg) { return deg * M_PI / 180.0; };

    double dLat = toRad(lat2 - lat1);
    double dLon = toRad(lon2 - lon1);
    double a = std::sin(dLat / 2) * std::sin(dLat / 2)
             + std::cos(toRad(lat1)) * std::cos(toRad(lat2))
             * std::sin(dLon / 2)   * std::sin(dLon / 2);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return EARTH_RADIUS_KM * c;
}

QList<Station> GeoLocator::findNearest(const QList<Station>& stations,
                                        double lat, double lon,
                                        double maxDistanceKm) {
    // Para: (odległość, indeks w liście)
    QList<QPair<double, int>> distIndexPairs;
    for (int i = 0; i < stations.size(); ++i) {
        double dist = haversineDistance(lat, lon,
                                        stations[i].latitude(),
                                        stations[i].longitude());
        if (maxDistanceKm <= 0.0 || dist <= maxDistanceKm)
            distIndexPairs.append({dist, i});
    }

    // Sortuj rosnąco po odległości
    std::sort(distIndexPairs.begin(), distIndexPairs.end(),
              [](const auto& a, const auto& b){ return a.first < b.first; });

    QList<Station> result;
    for (const auto& pair : distIndexPairs)
        result.append(stations[pair.second]);
    return result;
}
