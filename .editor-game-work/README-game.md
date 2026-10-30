# Cuartos JSON

Los 14 diseños de `Cuartos Composicion.txt` tienen ahora un JSON individual en
esta carpeta. El texto original se conserva intacto. Los archivos JSON son las
definiciones usadas por la compilación; no se regeneran desde el texto al compilar.

| Archivo | Función |
| --- | --- |
| `inicio_menu.json` | Menú jugable; cuatro puertas, solo la superior conectada |
| `inicio.json` | Inicio de la mazmorra |
| `boss_1_ant.json` | Recompensas; puerta derecha, tres palancas, sin llave |
| `fogata.json` | Fogata, entrega de aventureros y primera palanca |
| `cuarto_final_escaleras.json` | Escaleras y segunda palanca |
| `cuarto_final_escaleras_2.json` | Variante de escaleras; solo puerta inferior |
| `cuarto_1.json` / `cuarto_1_2.json` | Cuartos de piedras con murciélagos / mushys |
| `cuarto_2.json` / `cuarto_2_2.json` | Segunda distribución con murciélagos / mushys |
| `cuarto_3.json` / `cuarto_4.json` | Cuartos con patrullas de esqueletos |
| `cuarto_5.json` / `cuarto_6.json` | Cuartos de plantas |
| `mi_primer_cuarto.json` | Diseño adicional importado desde el editor |

## Compilación automática

El Makefile ejecuta `python tools/import_room.py rooms include/imported_room.h`
antes de procesar C++. Valida los `.json` directamente dentro de `rooms/` y genera
datos constantes de ROM. La GBA no lee JSON durante la partida.

Los diseños de composición usan `schemaVersion: 2` y un `runtimeBinding` que
conserva su función y los identificadores de los guardados. No se agregan como
copias al grupo de cuartos aleatorios: siguen ocupando sus lugares habituales.
Las decisiones aleatorias se hacen en el juego después de definir la semilla.

Se importan posiciones, objetos, fondos posibles, puertas, puntos Q, recompensas,
candidatos de palancas, patrullas de esqueletos, aparición de enemigos, trampas y
colisiones invisibles. Los comportamientos de rescate, combate, palancas y
guardado siguen bajo control del juego.

Los perfiles existentes mantienen sus tipos y cantidades de candidatos enemigos,
con `runtimeSlot` estable para los guardados; la semilla elige cuántos aparecen.
Los murciélagos conservan su movimiento de rebote y los mushys su movimiento libre.
`authoredAxis` y `sourceSymbol` conservan además la notación original. Los
esqueletos tienen recorridos explícitos `patrol.from/to`, de ida y vuelta.

## Coordenadas y reglas

- Lienzo 256×256; `x/y` de objetos indican esquina superior izquierda. Las
  posiciones de marcadores y extremos de patrulla son puntos sobre ese lienzo.
- La cuadrícula visible comienza en (8,48), con 15×10 celdas de 16 px.
- Objetos de dos celdas se exportan una sola vez: columnas/libreros verticales y
  mesas horizontales. Puertas laterales también ocupan dos celdas.
- `solid: false` en los asientos. Los sprites de 16×16 usan por defecto la caja
  superior de 10×8: `{"x":3,"y":0,"width":10,"height":8}`.
- `collisionProfile: "legacy-base"` conserva la colisión existente de columnas y
  libreros. `explicit` usa la hitbox del objeto. El personaje no cambia de caja.
- Puertas `rule: "possible"` corresponden a r; `required` corresponde a R. El
  generador respeta los lados declarados. Los destinos se asignan con la semilla.
- Marcadores `random` equivalen a Q, `ability` a &, y `npc_delivery` a 1/2/3.
  El símbolo $ de la fogata genera dos marcadores en el mismo punto: Q y &.
- Las tres Q y el & del boss son obligatorios. La puerta derecha se revela al
  activar las tres palancas. El boss se coloca a la izquierda de otro cuarto.
- `collisions` contiene los rectángulos invisibles %, sin aplicarles el recorte
  de las colisiones de sprites. Se conservan también las filas 4 y 5 del boss.
- `source.grid` conserva la cuadrícula original como referencia; la geometría
  que usa el juego está en `objects`, `markers` y `collisions`.

Conserva los 14 perfiles y sus `runtimeBinding`; cambiarles el nombre de archivo
es válido. Un perfil faltante, duplicado o incompatible detiene la conversión con
un mensaje antes de compilar C++. No edites `include/imported_room.h` a mano.

El formato v1 del editor sigue aceptando sus assets y fondos anteriores. Se
añadieron `table_armor`, `box` y `player` como decoraciones; colocar el asset
`player` no cambia al personaje controlable ni su colisión.

También se aceptan exportaciones v2 sencillas del editor con `rules.roomType:
"standard"` y acceso normal. `allowedDoorSides` declara los lados candidatos:
si no se colocaron sprites de puerta, se generan las posiciones estándar para
esos lados y la semilla decide sus conexiones. `onlyTheseSides` limita las salidas.
`spawn` puede ser `null` u omitirse en cuartos adicionales. Al entrar, el juego
coloca al jugador junto a la puerta opuesta del cuarto destino, para los cuatro
lados. Las coordenadas `spawn` de JSON antiguos se aceptan por compatibilidad,
pero no sustituyen la entrada por puerta.
Se conservan las hitboxes explícitas de esos diseños. Los cuartos adicionales
admiten columnas, libreros, mesas, asientos, cajas, jarrones, piedras, carteles,
hachas de suelo y cofres; esqueletos, murciélagos, hongos y plantas; llaves y
pociones, pinchos, fogata, escalera, palanca y variantes de puerta. El proyectil
de planta se genera durante el juego, no se coloca como objeto independiente.
Los enemigos usan sus movimientos originales. Cada cuarto permite hasta cuatro
de cada tipo, cuatro recogibles, doce pinchos, una fogata, una escalera y una
palanca candidata. La palanca puede ser la tercera del piso; la semilla escoge
entre los cuartos candidatos cuando se activaron las otras dos. Las escaleras
colocadas en cuartos adicionales también permiten pasar al siguiente piso.

Los cuartos v2 genéricos admiten hasta 16 marcadores Q, cuatro marcadores & y
32 rectángulos invisibles %. Q alimenta las posiciones candidatas del generador
para llaves, pociones, NPC y la tercera palanca. Cada & es una recompensa de
habilidad: `required:true` aparece siempre; la opcional se decide con la semilla
y la habilidad concreta se elige al generar el piso. Su estado se guarda como
objeto del cuarto. Los enemigos `spawnMode:"seeded"` eligen cantidad y posiciones
entre los puntos dibujados; los fijos siempre aparecen. Murciélagos y hongos
necesitan posiciones únicas por tipo. Los recogibles colocados y & suman cuatro
como máximo. Recortes distintos de 256×256, comportamientos personalizados,
patrullas explícitas y salas especiales siguen requiriendo un perfil original.

Para crear una sala adicional v1, usa un ID propio. Los IDs y slots estables
conservan las referencias al añadir o renombrar archivos. Quitar un diseño usado
o cambiar sus candidatos puede impedir restaurar una partida anterior.

## Verificación sin compilar

`python -B tools/test_import_room.py`

`python -B tools/test_composition_room.py`

`python -B tools/test_editor_v2.py`

`python -B tools/test_extended_import.py`

La conversión original se puede reproducir con
`python -B tools/convert_composition.py "Cuartos Composicion.txt" carpeta_destino`.
Usa otra carpeta si ya editaste los JSON, pues el conversor escribe esos diseños.
