# AirQuality – Monitor jakości powietrza GIOŚ

Aplikacja desktopowa C++/Qt 6 do pobierania i analizy danych z publicznego
REST API Głównego Inspektoratu Ochrony Środowiska (GIOŚ).

---

## Wymagania

| Narzędzie | Wersja minimalna |
|-----------|-----------------|
| Qt        | 6.4             |
| CMake     | 3.20            |
| Kompilator| GCC 11 / MSVC 2022 / Clang 14 |
| Google Test (opcjonalnie) | 1.12 |

---

## Budowanie projektu

### Qt Creator (zalecane)
1. Otwórz Qt Creator → **Open Project** → wybierz `CMakeLists.txt`
2. Wybierz kit (np. *Desktop Qt 6.x GCC 64bit*)
3. Kliknij **Build** (Ctrl+B)
4. Uruchom: **Run** (Ctrl+R)

### Wiersz poleceń (Linux/macOS)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
./AirQualityProject
```

### Wiersz poleceń (Windows – MSVC)
```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
Release\AirQualityProject.exe
```

---

## Uruchamianie testów
```bash
cd build
ctest --output-on-failure
# lub bezpośrednio:
./AirQualityTests
```

---

## Wzorce projektowe zastosowane w projekcie

| Wzorzec | Gdzie zastosowany | Cel |
|---------|------------------|-----|
| **Singleton** | `DatabaseManager` | Jeden punkt dostępu do lokalnej bazy JSON |
| **Strategy** | `ITrendStrategy`, `LinearTrendStrategy`, `MovingAverageTrendStrategy` | Wymienne algorytmy obliczania trendu |
| **Observer** | `AsyncFetcher` (sygnały Qt) | Powiadamianie GUI o zakończeniu wątku pobierania |

---

## Struktura projektu

```
AirQualityProject/
├── CMakeLists.txt          # System budowania
├── README.md               # Ten plik
├── Doxyfile                # Konfiguracja Doxygen
├── data/                   # Lokalna baza danych (JSON, tworzony auto)
│   └── measurements/
├── docs/                   # Wygenerowana dokumentacja HTML
├── include/                # Pliki nagłówkowe (.hpp)
│   ├── api/                # GiosClient, AsyncFetcher
│   ├── core/               # Station, Sensor, Measurement
│   ├── storage/            # DatabaseManager (Singleton)
│   ├── analysis/           # AirAnalyzer, ITrendStrategy, GeoLocator
│   ├── gui/                # MainWindow, ChartWidget, StationListWidget
│   └── utils/              # Logger, AppExceptions
├── src/                    # Implementacje (.cpp)
├── tests/                  # Testy jednostkowe (Google Test)
└── external/
    └── nlohmann/           # json.hpp (header-only)
```

---

## Generowanie dokumentacji Doxygen

```bash
# W katalogu głównym projektu:
doxygen Doxyfile
# Dokumentacja pojawi się w docs/html/index.html
```

---

## API GIOŚ – używane endpointy

Bazowy URL: `https://api.gios.gov.pl/pjp-api/v1/rest`

| Endpoint | Opis |
|----------|------|
| `GET /station/findAll` | Lista wszystkich stacji pomiarowych |
| `GET /station/sensors/{stationId}` | Czujniki danej stacji |
| `GET /data/getData/{sensorId}` | Historia pomiarów czujnika |

API jest publiczne, nie wymaga klucza dostępu.

---

## Tryb offline

Jeśli API GIOŚ jest niedostępne, aplikacja automatycznie wczytuje dane
z katalogu `data/` (ostatnio pobrane dane są tam zapisywane jako cache).
Status online/offline jest sygnalizowany kolorem paska stanu na dole okna.

---

