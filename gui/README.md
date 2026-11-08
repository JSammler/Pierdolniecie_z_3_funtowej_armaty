# GUI - wizualizacja strzału z armaty (OpenGL)

Okno OpenGL (GLFW, tryb fixed-function) + panel Dear ImGui. Każdy strzał to osobny
`Simulation` z katalogu `physics/` (ten katalog nie jest modyfikowany).

## Budowanie

Wymagane: CMake, kompilator C++23, OpenGL. GLFW i Dear ImGui są pobierane automatycznie
(FetchContent) przy pierwszej konfiguracji, więc potrzebny jest internet.
GLFW z systemu (`find_package(glfw3)`) ma pierwszeństwo.

Linux (Debian/Ubuntu): `sudo apt install build-essential cmake libgl1-mesa-dev xorg-dev`

    cmake -S . -B build
    cmake --build build
    ./build/Pierdolniecie_z_3_funtowej_armaty

## Sterowanie

| Klawisz / mysz              | Akcja                                            |
|-----------------------------|--------------------------------------------------|
| Spacja                      | strzał                                           |
| R                           | nowy cel                                         |
| Góra / Dół                  | kąt ±0.5° (Shift 0.1°, Ctrl 5°)                  |
| Lewo / Prawo                | siła ±2% (Shift 0.5%)                            |
| Kółko myszy                 | zoom (wyłącza auto-kamerę)                       |
| Prawy / środkowy + ruch     | przesuwanie widoku                               |

Panel po lewej zawiera wszystkie pola `SimulationConfig` (siła, czas impulsu, masa, kąt,
grawitacja, położenie lufy, krok czasu, max czas), presety grawitacji (Księżyc, Mars,
Ziemia, Jowisz, Słońce...) i presety armat (3-funtowa, 12-funtowa, ...).

## Pliki

- `Game.*` - logika gry bez OpenGL: uruchamianie symulacji, cel, punkty, animacja
- `Camera.*` - kamera 2D z auto-dopasowaniem
- `Renderer.*` - rysowanie sceny w OpenGL (niebo, ziemia, armata, tarcza, tor, kula)
- `Menu.*` - panel ImGui
- `App.*` - okno GLFW i pętla główna
- `Presets.hpp` - stałe: grawitacje planet, presety armat
