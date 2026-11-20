# PokéRando NX

Randomizer de ROMs de Pokémon de GBA y DS (Gen 3–5) que funciona en la propia Nintendo Switch (homebrew `.nro`, Atmosphère).

Es un port a C++ de [Universal Pokemon Randomizer FVX](https://github.com/upr-fvx/universal-pokemon-randomizer-fvx), basado a su vez en Universal Pokemon Randomizer (Dabomstew) y UPR ZX (Ajarmar). Interfaz hecha con [Borealis](https://github.com/xfangfang/borealis).

La app no incluye ni descarga ROMs: solo trabaja con las que tengas en tu SD.

## Estructura

- `core/`: núcleo del randomizer, en C++ puro y sin interfaz (romio, randomizers, settings, log, data)
- `cli/`: programa de Linux para probar el núcleo
- `app/`: interfaz Borealis (Switch y Linux)
- `tests/`: comparaciones con FVX

## Licencia

GPLv3, heredada de UPR FVX. Los datos de `core/data/` (offsets, tablas de texto y parches) provienen de FVX.
