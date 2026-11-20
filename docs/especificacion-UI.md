# PokéRando NX — Especificación gráfica y de interfaz

Documento de referencia para implementar la interfaz de la app homebrew **PokéRando NX** (Nintendo Switch, `.nro`, Atmosphère), un randomizer de ROMs de Pokémon de GBA y DS que funciona en la propia consola. La lógica se porta de **Universal Pokemon Randomizer FVX** (GPLv3).


---

## 1. Principios generales

- **Resolución de diseño:** 1280×720 px, que es la nativa en modo portátil. En dock se escala a 1920×1080 manteniendo las proporciones. Todas las medidas de este documento están en px a 1280×720.
- **Idioma:** español en toda la interfaz. Deja los textos en un archivo de cadenas aparte (`romfs/i18n/es.json`) para poder traducir la app más adelante.
- **Tema:** oscuro, plano y sin degradados. Un solo color de acento.
- **Control:** todo se puede hacer solo con los mandos. La pantalla táctil también funciona en todos los elementos, con zonas táctiles de al menos 44 px de alto.
- **Librería de UI recomendada:** Borealis (`natinusala/borealis`, rama actual). Si limita demasiado el estilo, usa SDL2 + renderizado propio, pero mantén este diseño.
- **Nunca se modifica la ROM original.** La ROM randomizada siempre se escribe como un archivo nuevo.

---

## 2. Colores (tokens)

| Token | Hex | Uso |
|---|---|---|
| `bg` | `#1F2023` | Fondo general |
| `surface` | `#2A2B2F` | Tarjetas, filas, botones, paneles |
| `sidebar` | `#18191B` | Barra lateral de pestañas |
| `border` | `#3A3B40` | Líneas separadoras de cabecera y pie (1 px) |
| `track-off` | `#4A4B52` | Interruptor apagado, pista del slider, borde de botón secundario |
| `text` | `#EDEDED` | Texto principal |
| `text-muted` | `#A3A6AD` | Texto secundario, etiquetas, rutas |
| `accent` | `#3DDBC4` | Foco, selección, interruptor encendido, botón principal, estado «Hecho» |
| `accent-hover` | `#7FEBDA` | Variante clara del acento (pulsación) |
| `warn` | `#F2B35B` | Etiqueta «NDS», carpetas, etiqueta «Exclusivo de esta app» |
| `on-accent` | `#1F2023` | Texto o icono sobre fondo de acento o `warn` |
| `error` | `#FF5A5F` | Mensajes de error (ver §8) |

El acento se puede cambiar en **Ajustes** entre `#3DDBC4` (turquesa, por defecto), `#FF5A5F` (rojo), `#5B8CFF` (azul) y `#F2B35B` (ámbar). Guárdalo en la configuración.

---

## 3. Tipografía

Incluye las fuentes en `romfs/fonts/`. Ambas son de licencia OFL, así que se pueden distribuir.

- **Títulos (H1):** Space Grotesk Bold (700), 28–30 px.
- **Resto:** Inter. Regular 400, Medium 500, SemiBold 600 y Bold 700.
- **Rutas, semillas y nombres de archivo:** una monoespaciada, por ejemplo JetBrains Mono o la mono del sistema.

| Estilo | Fuente | Tamaño | Peso |
|---|---|---|---|
| H1 de pantalla | Space Grotesk | 28 px (30 en Inicio) | 700 |
| H2 de sección | Inter | 26 px | 700 |
| Botón grande del menú | Inter | 22 px | 600 si tiene el foco, 500 si no |
| Fila de lista o archivo | Inter | 19 px | 500 (600 si tiene el foco) |
| Pestaña lateral | Inter | 18 px | 700 si está activa, 500 si no |
| Interruptor, modo o cuerpo | Inter | 16–17 px | 500 |
| Etiqueta de sección (MAYÚSCULAS) | Inter | 14–15 px | 600, `letter-spacing: 0.06em`, color `text-muted` |
| Texto secundario | Inter | 15–16 px | 400, color `text-muted` |
| Insignia (GBA/NDS) | Inter | 14 px | 600–700 |

---

## 4. Estructura común de todas las pantallas

```
┌──────────────────────────────────────────────── 1280 ─┐
│ CABECERA  88 px   (padding horizontal 48)             │
├───────────────────────────────────────────────────────┤ ← línea 1 px `border`
│                                                       │
│ CONTENIDO  568 px                                     │
│                                                       │
├───────────────────────────────────────────────────────┤ ← línea 1 px `border`
│ PIE (ayuda de botones)  64 px                         │
└───────────────────────────────────────────────────────┘
```

- **Cabecera (88 px):** título H1 a la izquierda e información contextual a la derecha (`text-muted`, 16 px). Centrado en vertical.
- **Pie (64 px):** ayudas de botones alineadas a la derecha, con 32 px entre cada una. Cada ayuda es un círculo de 28 px con la letra del botón (fondo `text`, letra `on-accent`, Inter Bold 15 px) seguido de la acción (17 px). Las ayudas de dos botones, como «L / R», son una píldora de 28 px de alto con 10 px de padding horizontal. La ayuda de la acción principal de la pantalla (por ejemplo «+ Randomizar») usa el círculo de color `accent`.
- **Márgenes del contenido:** 48 px a los lados. Arriba y abajo, entre 28 y 40 px según la pantalla.
- **Radios de esquina:** 8 px en filas de lista y pasos, 10 px en botones e interruptores, 12–14 px en paneles grandes, y 50 % en círculos.

---

## 5. Componentes

### 5.1 Botón de menú grande (Inicio)
- 520 × 72 px, radio 10, fondo `surface`.
- Icono de línea de 28 px a la izquierda (trazo 2 px, `currentColor`) y texto de 22 px. 24 px de padding y 18 px de separación entre icono y texto.
- **Con foco:** borde de 3 px `accent` y texto en peso 600. **Sin foco:** borde de 3 px transparente, para que el contenido no salte al mover el foco.
- 12 px de separación vertical entre botones.

### 5.2 Fila de archivo o carpeta (Explorador)
- 680 × 60 px, radio 8, 20 px de padding y 16 px entre icono y texto. 6 px de separación entre filas.
- Icono de 24 px:
  - carpeta con trazo `warn`
  - archivo compatible con trazo `text`
  - archivo no compatible: icono y texto en `text-muted`, con el texto «No compatible» a la derecha
  - «.. (subir)»: flecha hacia la izquierda en `text-muted`
- Tamaño del archivo alineado a la derecha (15 px, `text-muted`).
- **Con foco:** fondo `surface`, borde de 3 px `accent`, icono de color `accent` y texto en peso 600. Resta 3 px al padding para que no se desplace el contenido.

### 5.3 Pestaña lateral (Opciones)
- Barra lateral de 300 px de ancho, fondo `sidebar` y 16 px de padding vertical.
- Cada pestaña mide 48 px de alto, texto de 18 px y 32 px de padding a la izquierda.
- **Activa:** barra de 5 px `accent` en el borde izquierdo, fondo `surface`, texto `accent` en peso 700 y padding izquierdo de 27 px.

### 5.4 Selector de modo (segmentado)
- Fila de 2 a 5 botones que se reparten el ancho por igual (`flex: 1`), con 10 px de separación. Cada botón mide 56 px de alto, radio 10, fondo `surface` y texto de 16 px.
- Encima, una etiqueta de sección en mayúsculas (§3) con 10 px de separación.
- **Seleccionado:** borde de 3 px `accent` y texto en peso 700. Selección única, como un botón de radio.

### 5.5 Interruptor (toggle)
- Toda la fila es el botón: 52 px de alto, radio 10, fondo `surface`, 18 px de padding y texto de 17 px a la izquierda.
- Se colocan en una rejilla de 2 columnas con 8 px de separación.
- Pista de 52 × 28 px, radio 14 y 3 px de padding interior. Pomo de 22 px.
  - **Encendido:** pista `accent`, pomo `on-accent` a la derecha.
  - **Apagado:** pista `track-off`, pomo `text` a la izquierda.
- **Con foco:** borde de 3 px `accent` alrededor de la fila.
- Si hay más interruptores de los que caben, el área de contenido hace scroll vertical y la barra lateral se queda fija.

### 5.6 Slider (porcentaje de nivel)
- Fila de 64 px de alto, fondo `surface`, radio 10 y 20 px de padding.
- Etiqueta de 230 px de ancho, pista flexible de 8 px de alto (fondo `track-off`, parte rellena `accent`) y pomo circular de 28 px en `text`.
- Valor a la derecha (64 px de ancho, Bold), con el formato «+20 %» o «−10 %».
- Rango de −50 % a +50 % en pasos de 5. Se ajusta con ◀/▶ en la cruceta o con el stick, y también arrastrando con el dedo.

### 5.7 Fichas de elección (probabilidad de shiny)
- Fila de 6 fichas iguales (`flex: 1`) de 52 px de alto, radio 10, texto Bold de 17 px y 10 px de separación.
- **Sin seleccionar:** fondo `bg` dentro del panel `surface`. **Seleccionada:** fondo y borde `accent`, texto `on-accent`.

### 5.8 Botón principal y secundario
- **Principal:** 64 px de alto, radio 10, fondo `accent`, texto `on-accent` en Bold de 19–20 px.
- **Secundario:** 64 px de alto, radio 10, borde de 2 px `track-off`, fondo transparente, texto `text` en SemiBold de 19 px.

### 5.9 Insignia de plataforma
- Píldora con radio 6 y padding de 4 × 10 px. Texto de 14 px en peso 600 y color `on-accent`.
- «GBA» con fondo `accent`. «NDS» con fondo `warn`.

### 5.10 Etiqueta «Exclusivo de esta app»
- Píldora con radio 6 y padding de 3 × 9 px, fondo `warn`, texto `on-accent` en Bold de 14 px. Va junto al H2 de la pestaña Extras.

### 5.11 Logotipo
- Círculo de 44 px con un anillo de 4 px `accent` y un punto central de 12 px `accent`, seguido del nombre «PokéRando NX» en el estilo H1 y la versión («v0.1», 15 px, `text-muted`).
- No uses la Poké Ball oficial ni ningún logotipo o arte de Nintendo, Game Freak o The Pokémon Company.

---

## 6. Pantallas

### 6.1 Inicio
- **Cabecera:** logotipo a la izquierda. A la derecha, «SD: XXX GB libres» con el espacio libre real de la tarjeta SD.
- **Contenido** (padding de 40 × 48 px), en dos columnas con 40 px de separación:
  - **Izquierda:** columna de 520 px con 4 botones de menú (§5.1):
    1. **Abrir ROM** (icono de carpeta). Tiene el foco al arrancar.
    2. **Presets guardados** (icono de lista).
    3. **ROMs generadas** (icono de reloj).
    4. **Ajustes** (icono de engranaje).
  - **Derecha:** panel flexible con fondo `surface`, radio 14 y 32 px de padding.
    - Título «Últimas generadas» (20 px, 600, `text-muted`).
    - Hasta 3 tarjetas con fondo `bg`, radio 10 y padding de 18 × 20 px. Cada una muestra el nombre del juego seguido de «· Random» (19 px, 600), «Preset «X» · semilla N» (15 px, `text-muted`) y la insignia de plataforma a la derecha.
    - Abajo del todo, la nota: «Las ROMs nuevas se guardan junto a la original. La original nunca se modifica.» (15 px, `text-muted`).
    - Si todavía no se ha generado nada, en lugar de las tarjetas aparece «Aún no has randomizado nada. Empieza con Abrir ROM.».
- **Pie:** A Seleccionar · + Salir.
- **Navegación:** ▲/▼ mueve el foco entre los botones. ▶ pasa a las tarjetas de la derecha, y A sobre una tarjeta abre su pantalla de Resultado.

### 6.2 Elegir ROM (explorador)
- **Cabecera:** H1 «Elegir ROM». A la derecha, la ruta actual en monoespaciada, por ejemplo `sdmc:/roms/gba/pokemon/`.
- **Contenido** (padding de 28 × 48 px):
  - **Izquierda:** lista de 680 px (§5.2), con scroll si hace falta. Orden: «.. (subir)», después las carpetas y después los archivos, todo en orden alfabético. Solo se muestran `.gba` y `.nds` como compatibles. El resto aparece atenuado, o se oculta si así se elige en Ajustes.
  - **Derecha:** panel «JUEGO DETECTADO» (fondo `surface`, radio 14, padding 28), que se actualiza al mover el foco sobre una ROM:
    - Etiqueta «JUEGO DETECTADO» (15 px, 600, `accent`, en mayúsculas).
    - Nombre del juego (28 px, Bold).
    - Lista de datos en dos columnas (120 px y el resto, 12 px entre filas, 17 px): Plataforma, Región (con el código del juego, por ejemplo «España (BPES)»), Generación e Integridad.
    - Integridad: «ROM original verificada» en `accent` si el CRC/SHA-1 coincide con una ROM conocida. Si no coincide, «ROM modificada o hack» en `warn`, con un aviso de que puede no funcionar bien.
    - Abajo, el botón principal «Continuar a opciones».
    - Si la ROM no es compatible: el panel muestra «Juego no compatible», el código detectado y el botón desactivado (opacidad 40 %).
- **Pie:** A Abrir · B Atrás · Y Favoritos (marca la carpeta como favorita para abrirla directamente la próxima vez).
- **Carpeta inicial:** la última que se usó. Si es la primera vez, `sdmc:/roms/`, y si no existe, `sdmc:/`.

### 6.3 Opciones del randomizer
- **Cabecera:** H1 «Opciones · {nombre del juego}». A la derecha, «Preset: {nombre}» y «Semilla: {número}» (16 px, valor en `text` y la semilla en monoespaciada).
  - El preset se muestra como «Personalizado» en cuanto se cambia cualquier opción de un preset cargado.
  - La semilla es un número aleatorio. Al pulsar sobre ella se puede escribir con el teclado del sistema (swkbd).
- **Contenido:** barra lateral de pestañas (§5.3) y el área de la pestaña activa (padding de 24 × 44 px y 18 px entre bloques), con este orden de arriba abajo:
  1. H2 con el nombre de la pestaña, más la etiqueta «Exclusivo de esta app» si corresponde.
  2. Selector de modo (§5.4), si la pestaña lo tiene.
  3. Panel de shiny (§7.10), solo en Extras.
  4. Rejilla de interruptores (§5.5).
  5. Slider (§5.6), si la pestaña lo tiene.
- **Opciones dependientes:** cuando el modo es «Sin cambios», los interruptores que dependen de él se muestran desactivados (opacidad 40 %) y el foco se los salta. Por ejemplo, en Salvajes con modo «Sin cambios», «Fuerza similar» no tiene efecto.
- **Opciones de otras generaciones:** las que no existen en el juego cargado no se muestran (§7).
- **Pie:** «L / R Cambiar pestaña» a la izquierda. A la derecha: A Cambiar · X Guardar preset · **+ Randomizar** (con el círculo en `accent`).
- **Navegación:**
  - L/R pasa a la pestaña anterior o siguiente y es cíclico.
  - ◀ en la primera columna del contenido mueve el foco a la barra lateral, y ▶ lo devuelve.
  - X abre el teclado del sistema para poner nombre al preset.
  - B vuelve al explorador. Si hay cambios sin guardar, pide confirmación (§8).
  - − (Minus) abre un menú rápido: Cargar preset · Restablecer pestaña · Restablecer todo · Pegar settings string (compatible con las settings strings de UPR FVX).

### 6.4 Randomizar y resultado
Esta pantalla tiene dos estados.

**a) En curso**
- Cabecera con el H1 «Randomizar».
- Columna izquierda (560 px) con la lista de pasos. Cada paso es una fila con fondo `surface`, radio 8, padding de 14 × 18 px y texto de 17 px. A la derecha de cada fila, su estado: «Pendiente» (`text-muted`), «En curso…» (`text` más un spinner de 18 px) o «Hecho» (`accent`, peso 600).
- Pasos: Pokémon base y tipos · Iniciales · Salvajes · Entrenadores y líderes · Movimientos y MT · Objetos · Extras · Guardar ROM y log. Solo aparecen los pasos que tienen alguna opción activa.
- No hay botones ni se puede cancelar a mitad de un paso, para evitar dejar una ROM corrupta.

**b) Terminado**
- Arriba de la columna izquierda, un círculo `accent` de 56 px con un ✓ de trazo 3 px en `on-accent`. A su lado, «¡ROM lista!» (26 px, Bold) y «Completado en X,X s» (16 px, `text-muted`).
- Panel derecho (fondo `surface`, radio 14, padding 28):
  - «GUARDADO EN» y la ruta completa en monoespaciada de 17 px, con salto de línea en cualquier carácter.
  - «TUS INICIALES» y una rejilla de 3 tarjetas (92 px de alto, fondo `bg`, radio 10) con los nombres de las iniciales resultantes, y su sprite si está disponible. Si en el log están activados los spoilers, esta sección se puede ocultar con un ajuste.
  - Abajo, dos botones en fila con 12 px de separación: **Jugar en mGBA** (principal) y **Ver log de spoilers** (secundario). Para NDS, el principal dice «Jugar en melonDS» o el emulador configurado.
  - «Jugar» lanza el emulador con la ROM a través de `envSetNextLoad`. Si el emulador no está instalado, el botón queda desactivado con la nota «Emulador no encontrado».
- **Pie:** A Aceptar · Y Otra semilla (repite con las mismas opciones y una semilla nueva) · B Volver al inicio.
- **Archivo de salida:** `{carpeta de la ROM}/{JuegoCorto}_{semilla}.{gba|nds}` y el log en `{...}_{semilla}.log.txt`, junto a la ROM original. Por defecto se borran las randomizadas anteriores de ese juego en esa carpeta (Ajustes › Conservar ROMs randomizadas para guardarlas). El Resultado no muestra las iniciales.

### 6.5 Pantallas sin maqueta, con el mismo estilo
- **Presets guardados:** lista (§5.2) con los presets de `sdmc:/switch/pokerandonx/presets/*.json`. Cada fila muestra el nombre, el juego o generación para el que se creó y la fecha. A Cargar · X Renombrar · Y Duplicar · − Borrar (con confirmación). Incluye presets de serie: «Caos total», «Solo salvajes», «Nuzlocke justo» y «Kaizo».
- **ROMs generadas:** historial completo, con el mismo formato que las tarjetas de Inicio. A abre su Resultado y − la borra.
- **Ajustes:** color de acento (4 fichas §5.7) · carpeta inicial del explorador · ocultar archivos no compatibles · emulador para GBA (mGBA / RetroArch) · emulador para DS · ocultar iniciales en Resultado · idioma · Acerca de (créditos a UPR, ZX y FVX y licencia GPLv3).

---

## 7. Contenido de las pestañas de Opciones

Para cada pestaña: modo (selección única), interruptores, slider y su equivalencia aproximada con UPR FVX. **Tienes que comprobar en el código de FVX el nombre exacto de cada opción** y si se aplica a cada generación, y ocultar las que no se apliquen. Los textos son los que ve el usuario.

### 7.1 Pokémon base
- **Modo «ESTADÍSTICAS BASE»:** Sin cambios · Mezclar · Aleatorias (Base Stats: Unchanged / Shuffle / Random).
- **Interruptores:** Seguir evoluciones (Follow Evolutions) · Actualizar stats a Gen moderna (Update Base Stats) · Tipos aleatorios (Pokemon Types) · Habilidades aleatorias (Abilities, Gen 3+) · Evoluciones aleatorias (Evolutions) · Evolucionar siempre a más fuerte (Force Growth) · Sin evoluciones convergentes (No Convergence) · Curvas de EXP iguales (Standardize EXP Curves).

### 7.2 Iniciales y estáticos
- **Modo «INICIALES»:** Sin cambios · Elegir a mano · Aleatorios · Cualquier básico.
  - «Elegir a mano» abre una lista con buscador para elegir los 3 Pokémon.
- **Interruptores:** Fuego / Agua / Planta · Triángulo de tipos · Sin tipos repetidos · Sin doble tipo (restricciones de tipo de las iniciales de FVX) · Legendarios y regalos aleatorios (Static Pokémon) · Intercambios aleatorios (In-game Trades).

### 7.3 Movimientos
- **Modo «MOVIMIENTOS POR NIVEL»:** Sin cambios · Aleatorios · Del mismo tipo · Mezcla.
- **Interruptores:** Potencia, precisión y PP aleatorios · Tipo de los movimientos aleatorio · Movimientos huevo aleatorios · 4 movimientos desde nivel 1 · Prohibir movimientos rompejuegos · Nombres de movimientos aleatorios (en FVX está en Graphics, pero aquí va en Movimientos).

### 7.4 Entrenadores
- **Modo «EQUIPOS RIVALES»:** Sin cambios · Aleatorios · Mantener tipo del gimnasio (Preserve Type Themes) · Tema o tipo primario (Preserve Themes Or Primary).
- **Interruptores:** Usar solo Pokémon locales · Liga con Pokémon únicos · Añadir Pokémon extra · Combates dobles (Battle Style; triples y rotatorios solo en Gen 5+) · Forzar evolución completa · Objetos equipados a jefes · Nombres de entrenadores aleatorios · Rival con tus iniciales.
- **Slider:** Nivel de rivales (Percentage Level Modifier).

### 7.5 Pokémon salvajes
- **Modo «MODO»:** Sin cambios · Por zona · Zona 1 a 1 · Global 1 a 1 · Familia a familia.
  - En la maqueta el valor por defecto es «Por zona».
- **Interruptores:** Atrápalos a todos (Catch-Em-All) · Fuerza similar (Similar Strength) · Mantener tipo primario · Mantener tema de la zona · Evitar legendarios · Objetos equipados aleatorios.
  - «Atrápalos a todos» y «Fuerza similar» se pueden activar a la vez.
- **Slider:** Nivel de salvajes.

### 7.6 MT, MO y tutores
- **Modo «MOVIMIENTOS DE LAS MT»:** Sin cambios · Aleatorios.
- **Interruptores:** Compatibilidad de MT aleatoria · Tutores aleatorios · Compatibilidad total · MT reutilizables (antes de Gen 5) · MO olvidables (Gen 1-5) · MO compatibles con todos.

### 7.7 Objetos
- **Modo «OBJETOS DEL SUELO»:** Sin cambios · Mezclar · Aleatorios.
- **Interruptores:** Tiendas aleatorias · Caramelos raros a 10 ₽ (Add Cheap Rare Candies) · Objetos de Recogida aleatorios (Pickup Items) · Prohibir objetos inútiles (Ban Bad Items).

### 7.8 Tipos
- **Modo «TABLA DE TIPOS»:** Sin cambios · Aleatoria · Equilibrada · Mantener identidad · Inversa.
- **Interruptor:** Actualizar efectividades a Gen moderna (Update Type Effectiveness).

### 7.9 Gráficos
- **Modo «COLORES DE LOS POKÉMON»:** Sin cambios · Por tipo · Por evolución · Totalmente aleatorios (Palette randomization). Es completo en Gen 1-3 y parcial en Gen 4 y 5.
- **Interruptor:** Sprite de jugador personalizado (Custom Player Graphics, solo Gen 1-3). Al activarlo se elige el sprite en una lista.

### 7.10 Extras (exclusivo de esta app)
- **Panel «Probabilidad de shiny»:** fondo `surface`, radio 12 y padding de 18 × 20 px.
  - Título (19 px, 600) y, a la derecha, «Original: 1 entre 8192» en `text-muted`.
  - Fichas (§5.7): **1/8192 · 1/4096 · 1/1024 · 1/256 · 1/64 · 1/8**. Por defecto, 1/8192.
  - Nota debajo: «Afecta a salvajes, huevos y regalos. Los entrenadores rivales no cambian.»
  - **Implementación:** en Gen 3, 4 y 5 un Pokémon es shiny si `(TID ^ SID ^ PID_alto ^ PID_bajo) < 8`, es decir, 8 de 65536 = 1/8192. Hay que parchear ese umbral en el código del juego: 16 → 1/4096, 64 → 1/1024, 256 → 1/256, 1024 → 1/64 y 8192 → 1/8. **Verifica en la decomp** (`pokeemerald`, `pokefirered`, `pokeruby`) y en las ROMs de DS **dónde está la comparación y cómo está codificada en cada juego y región** antes de parchearla. Si un juego o región no es compatible, el panel muestra «No disponible para este juego» y las fichas aparecen desactivadas.
- **Interruptores:** Texto rápido (Fast Text) · Correr en interiores (Running Shoes Indoors, si aplica) · Evoluciones imposibles por nivel (Change Impossible Evolutions) · Generar log de spoilers.

---

## 8. Diálogos, avisos y estados

- **Diálogo modal:** se oscurece el fondo con negro al 60 %. Tarjeta centrada de 640 px de ancho, fondo `surface`, radio 14 y padding 32, con título de 22 px Bold, cuerpo de 17 px y dos botones (§5.8) alineados a la derecha. A confirma y B cancela.
  - Ejemplo: «¿Salir sin guardar el preset?» con los botones Salir / Cancelar.
- **Error:** el mismo diálogo con un icono de 28 px en `error` junto al título. Casos:
  - ROM no compatible
  - espacio insuficiente en la SD (hace falta el tamaño de la ROM + 1 MB)
  - error al escribir
  - opciones incompatibles entre sí (indicando cuáles son)
- **Toast:** aviso abajo en el centro, 48 px de alto, fondo `sidebar` con borde de 1 px `border` y radio 10. Dura 2,5 s. Ejemplos: «Preset guardado», «Settings string pegada».
- **Foco:** siempre es visible y siempre es el borde de 3 px `accent` (o la barra lateral de 5 px en las pestañas). Nunca se representa solo con un cambio de color del texto.
- **Animaciones:** cortas, de 120–150 ms con `ease-out`, para el movimiento del foco, el cambio de pestaña (fundido del contenido) y el pomo del interruptor. Nada de animaciones largas.
- **Sonido:** opcional, los clics del sistema si Borealis los ofrece.

---

## 9. Rutas y archivos de la app

| Qué | Ruta |
|---|---|
| App | `sdmc:/switch/pokerandonx/pokerandonx.nro` |
| Configuración | `sdmc:/switch/pokerandonx/config.json` (acento, última carpeta, emuladores, favoritos) |
| Presets | `sdmc:/switch/pokerandonx/presets/*.json` |
| Historial | `sdmc:/switch/pokerandonx/history.json` |
| Salida | `{carpeta de la ROM}/` (junto a la original) |

---

## 10. Aspectos legales

- La app no incluye, no descarga y no enlaza ROMs. Solo trabaja con las ROMs que el usuario tiene en su SD.
- No usa logotipos, arte ni la tipografía oficial de Pokémon. Si se muestran sprites, se leen de la propia ROM del usuario.
- La licencia es GPLv3, heredada de UPR FVX, con créditos visibles en Ajustes › Acerca de.
