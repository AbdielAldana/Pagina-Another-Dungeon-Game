# Formatos compatibles con el juego

Las exportaciones **Para el juego** corresponden al importador actual de
`C:/GBA/butano/my-rpg/tools/import_room.py`. La página solo describe diseños: no
elige semillas, conexiones, cantidades aleatorias, recompensas ni estados de una
partida.

## Perfiles de composición, schemaVersion 2

Se cargan directamente los 14 JSON de `rooms/` con `runtimeBinding`:

`inicio_menu`, `inicio`, `boss_1_ant`, `fogata`, `cuarto_final_escaleras`,
`cuarto_final_escaleras_2`, `cuarto_1`, `cuarto_1_2`, `cuarto_2`, `cuarto_2_2`,
`cuarto_3`, `cuarto_4`, `cuarto_5`, `cuarto_6`.

El documento conserva sus propiedades, aunque algunas se representen de forma
distinta dentro del editor. Importar y exportar sin cambios preserva el contenido
completo del JSON. No se exportan campos internos del editor.

| Campo | Tratamiento |
| --- | --- |
| `id`, IDs de objetos, marcadores y colisiones | Se mantienen estables |
| `runtimeBinding`, `roomKind`, `doorPolicy` | Conservan función y perfil del juego |
| `background`, `backgroundOptions` | Fondo de referencia y lista ordenada de posibles fondos |
| `spawn`, objetos `x/y` | Coordenadas del lienzo completo 256×256; `spawn` se conserva en perfiles antiguos |
| `hitbox`, `collisionProfile` | Caja local y perfil explícito o base existente |
| Puertas `side`, `rule`, `access`, `hiddenUntil` | Se conservan sin generar destinos concretos |
| `runtimeSlot`, `spawnMode`, `behavior`, `authoredAxis` | Mantienen candidatos y comportamiento del perfil |
| `patrol.axis/from/to/mode` | Extremos como puntos centrales en disco; modo de ida y vuelta |
| `animation` | Metadatos originales del cofre; no se crea una animación continua |
| Marcadores `random`, `ability`, `npc_delivery` | Se muestran como Q, &, NPC con slot |
| `required` | Se conserva en Q y &; Boss 1 nativo tiene recompensas obligatorias |
| `collisions` | Rectángulos independientes con ID; no generan sprites |
| `source`, `sourceCell`, `sourceSymbol` | Referencia original; no se recalculan al mover objetos |
| `placement`, `access.requiresKey` | Se conserva la orientación del boss y el acceso sin llave |

### Alias de catálogo

| JSON del juego | Catálogo del editor |
| --- | --- |
| `chair` | `seat` |
| `weapon_table` | `table_armor` |
| `stone` | `rock` |
| `lever` | `label` |

Se restituyen los nombres originales al exportar. La fogata se interpreta como
frames de **16×32**, cuatro frames, y se incluye el fondo `menu_inicio`.

Las restricciones del juego se comprueban al exportar: cantidades y tipos de
candidatos por perfil, slots consecutivos, dos posiciones candidatas de palanca
en fogata y escaleras, escalera única, límites de 26 piedras/12 trampas, asientos
sin colisión y cajas superiores de 10×8 para enemigos de 16×16.

Boss 1 exige una sola puerta derecha R, tres Q obligatorias y un & obligatorio,
tres palancas y `requiresKey: false`. Escaleras 2 conserva su única puerta inferior R.
La fogata conserva el fuego, la recompensa compartida y los tres slots de entrega.

## Cuartos v1

Se importan los JSON originales como `mi_primer_cuarto.json` sin perder frames,
hitboxes, coordenadas ni propiedades. Los lados de sus puertas se deducen de la
geometría para mostrarlos en el inspector, como hace el importador del juego.
La exportación conserva schemaVersion 1 y los campos del archivo original; no
añade las estructuras de edición del formato v2.

## Cuartos v2 adicionales

Usan `rules.roomType: "standard"` o `"boss"`, `onlyTheseSides` y
`allowedDoorSides`. Las conexiones se resuelven al generar el piso. Una sala
boss debe declarar un solo lado y usar `rules.access: "three_levers"`.

La semilla del piso escoge exactamente una sala boss para el espacio de
recompensa. Los JSON boss se excluyen de la seleccion de cuartos normales. La
puerta puede quedar en cualquier lado declarado; `side` define la conexion y
`rotation` solo gira el sprite. Los fondos fijos `boss_1` y `boss_2` funcionan
en el juego. El perfil nativo `boss_1_ant` participa en la selección por
semilla. Si un diseño no cabe, el generador prueba las otras variantes.

En un boss v2 adicional, cada marcador Q genera una recompensa y el marcador
& genera una habilidad. Si `markers` está vacío, la sala no tiene recompensas.

`spawn` es opcional (`null` o ausente). Un cuarto recién creado usa `null`.
Cuando el jugador cruza una puerta, el juego calcula la entrada desde la puerta
opuesta del cuarto destino, incluida la entrada por abajo al venir desde arriba.
Un `spawn` numérico de un archivo anterior se conserva por compatibilidad, pero
no cambia la posición de entrada a un cuarto conectado.

El importador acepta los sprites de cuarto listados en [EDITOR.md](EDITOR.md).
Los roles `enemy`, `hazard`, `item`, `lever`, `interactive`, `stairs` y `door`
identifican funciones reales del juego; `decoration` no activa enemigos ni
recogibles. `bat`, `mushy` y `plant` usan los comportamientos existentes; el
`frame` inicial no define su movimiento. `spikes.alwaysActive` es booleano:
`true` mantiene la trampa activa; `false` usa su ciclo. `label` es candidato
para la tercera palanca del piso, sin elegir el cuarto al exportar. `key`,
`energy` y `sprint` se recogen y se guardan mediante los objetos del juego.
`campfire` cura una vez por piso y `stairs` avanza al siguiente. El cofre usa
sus estados por cercanía y la palanca permanece visible al activarse.

Las variantes de puerta usan `side` y `rule`; el generador decide conexiones y
estado abierto/cerrado. Las variantes especiales conservan su estilo. El asset
`bullet` es generado por `plant` durante la partida. Para añadirlo suelto o
para usar patrullas explícitas y salas especiales hace falta un perfil
original. Un diseño incompatible puede guardarse como **Borrador del editor**.

En cuartos adicionales, `markers` admite `{id,type:"Q",x,y,required:false}`
y `{id,type:"&",x,y,required:boolean}`. Cada ID es estable y único entre objetos
y marcadores. Q es una posición candidata que el generador usa con la semilla
para llaves, pociones, NPC y la tercera palanca. & es un lugar de recompensa de
habilidad. `required:true` la hace aparecer siempre; `false` permite que la
semilla decida si aparece. El JSON no predetermina qué habilidad será.
`collisions` admite rectángulos `{x,y,width,height}` en píxeles desde la esquina
superior izquierda; bloquean al jugador y no generan sprites. Los enemigos
admiten `enemy:{spawnMode:"fixed"|"seeded",patrol:null}`. En modo `seeded`, la
semilla elige cantidad y posiciones entre los sprites dibujados, y el movimiento
durante la partida sigue siendo el original del enemigo. Se admiten hasta 16 Q,
cuatro &, 32 rectángulos % y cuatro recogibles y & sumados por cuarto. Los
murciélagos y hongos de un mismo cuarto necesitan posiciones distintas.

Ejemplo de cuarto adicional: [murcielago.room.json](examples/murcielago.room.json).
Ejemplo con Q, &, %, y aparición por semilla: [marcadores.room.json](examples/marcadores.room.json).

## Conservación y actualización

Los perfiles descargan `runtimeBinding.json`. Sustituye el archivo correspondiente
en `rooms/`; añadir otra copia con el mismo ID o perfil produce un error del
importador. Para otro cuarto adicional usa un ID distinto. Cambiar posiciones o
candidatos puede afectar a partidas activas que almacenaron el estado anterior.

El editor no lee ni escribe `Cuartos Composicion.txt`, no compila y no modifica
guardados. `source.grid` es una referencia histórica; el diseño que usa el juego
está en objetos, marcadores y colisiones.
