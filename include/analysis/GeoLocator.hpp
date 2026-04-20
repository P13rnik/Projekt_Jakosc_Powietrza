#pragma once
#include <QList>
#include "core/Station.hpp"

/**
 * @brief Narzędzie do obliczeń geograficznych.
 *
 * Oblicza odległości między współrzędnymi geograficznymi
 * z użyciem wzoru haversine (uwzględnia krzywiznę Ziemi).
 */
class GeoLocator {
public:
    /**
     * @brief Oblicza odległość między dwoma punktami geograficznymi (wzór haversine).
     * @param lat1 Szerokość geograficzna punktu 1 (stopnie).
     * @param lon1 Długość geograficzna punktu 1 (stopnie).
     * @param lat2 Szerokość geograficzna punktu 2 (stopnie).
     * @param lon2 Długość geograficzna punktu 2 (stopnie).
     * @return Odległość w kilometrach.
     */
    static double haversineDistance(double lat1, double lon1,
                                    double lat2, double lon2);

    /**
     * @brief Sortuje stacje według odległości od podanej lokalizacji.
     * @param stations Lista wszystkich stacji.
     * @param lat Szerokość geograficzna punktu odniesienia.
     * @param lon Długość geograficzna punktu odniesienia.
     * @param maxDistanceKm Maksymalna odległość w km (0 = bez limitu).
     * @return Lista stacji posortowana wg odległości (rosnąco).
     */
    static QList<Station> findNearest(const QList<Station>& stations,
                                      double lat, double lon,
                                      double maxDistanceKm = 0.0);

private:
    static constexpr double EARTH_RADIUS_KM = 6371.0;
};
