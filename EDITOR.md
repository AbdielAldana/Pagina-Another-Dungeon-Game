# Taller de cuartos de AnotherDungeon

El editor abre y vuelve a exportar los JSON de `rooms/` del juego, incluidos los
14 perfiles de composición y el cuarto original v1. No modifica el ROM ni
`Cuartos Composicion.txt` y no genera decisiones aleatorias.

## Abrir el editor

Abre `editor.html` desde Live Server o un servidor HTTP. `file://` no permite
cargar los módulos y BMP. Para actualizar el hosting, sube el contenido de
`editor-hosting.zip`, reemplazando los archivos anteriores. Incluye el nuevo
`editor-game-format.js`, los fondos y las plantillas. Recarga sin caché si usabas
una versión anterior.

## Editar un cuarto del juego

1. Usa **Mis cuartos → Importar JSON**, **Pegar código**, o elige una de las
   **Plantillas del juego → Abrir plantilla**.
2. Edita posiciones, frames, hitboxes, objetos y marcadores. Las coordenadas
   conservan su referencia al fondo completo de 256×256. La vista muestra la
   zona central de 240×160, con cuadrícula de 16 px desde (8,48).
3. En los perfiles aparecen sus **fondos posibles**, el **perfil de colisión**,
   los **slots enemigos** y los pinchos **siempre activos (D)**.
4. Exporta con **Archivo: Para el juego** y sustituye el JSON del mismo cuarto en
   `rooms/`. Los perfiles descargan el nombre de su `runtimeBinding`.

El perfil conserva el papel de la sala: menú, inicio, fogata, escaleras,
recompensas o cuarto normal. No se puede cambiar su identidad desde los
controles de reglas genéricas. Para otro papel, abre la plantilla correspondiente.
No dejes dos archivos con el mismo ID o `runtimeBinding` dentro de `rooms/`.

Se preservan IDs, nombres originales de assets, `runtimeBinding`, `roomKind`,
`backgroundOptions`, puertas r/R y acceso, `collisionProfile`, `runtimeSlot`,
`spawnMode`, patrullas, recompensas obligatorias, posiciones de entrega,
rectángulos %, `sourceCell`, `sourceSymbol` y la cuadrícula de referencia.

## Marcadores, enemigos y animación

- **Q** son posiciones candidatas. En Boss 1 las tres Q son obligatorias.
- **&** es la recompensa de habilidad. Boss 1 conserva su & obligatorio.
- **NPC 1/2/3** son posiciones de entrega en la fogata; se pueden mover y editar
  sus slots. No generan aventureros al exportar.
- En cada fogata y variante de escaleras se conservan dos posiciones candidatas
  de palanca. El juego elige una usando la semilla.
- Los esqueletos muestran la patrulla y permiten editar sus extremos. En disco
  son puntos centrales `patrol.from/to`; el inspector muestra coordenadas de la
  esquina superior izquierda del sprite. El cambio de referencia es reversible.
- Murciélagos y mushys conservan sus comportamientos del juego. Sus candidatos
  enemigos mantienen tipos, cantidades y slots para conservar los guardados.
- Cofres y palancas previsualizan manualmente sus tres frames. Sus estados
  durante la partida siguen bajo control del juego.

Boss 1 conserva únicamente la puerta derecha obligatoria, acceso por tres
palancas sin llave y sus cuatro recompensas. La plantilla incluye las colisiones
invisibles de todas las filas indicadas en el diseño original.

**Marcar como Boss** cambia el tipo del cuarto actual. Para exportar al
juego, debe tener una sola puerta y acceso `three_levers`. La semilla escoge
una sala boss por piso para el espacio de recompensa; sus JSON no se mezclan
con los cuartos normales. Los fondos `boss_1` y `boss_2` ya se integran al
juego. El perfil nativo `boss_1_ant` también participa en la selección por
semilla.

## Colisiones

Los sprites nuevos de 16×16 usan la caja superior `{x:3,y:0,width:10,height:8}`.
Los asientos no bloquean el paso. Al importar se conserva la hitbox existente;
**Aplicar colisión recomendada** cambia únicamente el objeto seleccionado.

**Conservar base de columna / librero** mantiene el perfil `legacy-base`.
**Usar la caja dibujada** exporta `explicit`. Los rectángulos % son independientes
de los sprites y conservan sus propios IDs. El personaje controlable no cambia.

## Archivos adicionales y borradores

Para crear un cuarto adicional usa **Nuevo**. El juego admite ahora las
decoraciones `table`, `table_armor`, `library_front`, `library_back`, `column`,
`seat`, `box`, `vase`, `rock`, `sign`, `axe`, `player` y `chest`; los enemigos
`skeleton`, `bat`, `mushy`, `plant`; los recogibles `key`, `energy`, `sprint`;
`spikes`, `label`, `campfire`, `stairs` y las cuatro variantes de `door`.
`player` sirve solo de referencia visual. `bullet` lo genera una planta al
disparar y no se coloca como objeto independiente. Los fondos son `land`, `room2`, `room3`, `room_campfire`, `boss_1` y
`boss_2`; ambos fondos de jefe funcionan en las salas boss integradas.

Los cuartos nuevos exportan `spawn: null`. El jugador entra junto a la puerta
de llegada, según su lado y posición. La referencia de inicio que se puede
dibujar en el editor es opcional y no reemplaza esa entrada. Los JSON antiguos
con `spawn` se siguen importando y conservan su dato al exportar.

Límites por cuarto adicional: 40 objetos, 32 decoraciones, cuatro enemigos de
cada tipo, hasta cuatro recogibles y recompensas & en total, 16 puntos Q,
cuatro recompensas &, 32 colisiones invisibles, doce pinchos, un cofre, una palanca candidata,
una fogata y una escalera. Los murciélagos rebotan, los hongos persiguen, las
plantas disparan y los esqueletos usan su patrulla corta original. La palanca
puede convertirse en la tercera del piso cuando las otras dos se activan; el
generador escoge el cuarto candidato con la semilla. La fogata cura una vez
por piso y la escalera avanza al siguiente piso. Puertas abiertas/cerradas
cambian de aspecto según el estado real de la conexión; la variante especial
conserva su estilo. `axe` es decoración de suelo en el juego.

Q ofrece posiciones candidatas para llaves, pociones, NPC y la tercera palanca;
la semilla elige si se usan. & crea una recompensa de habilidad: si es obligatoria
aparece siempre; si es opcional, aparece aproximadamente en la mitad de los pisos.
La habilidad se elige con la semilla al generar el piso y se conserva al guardar.
Las colisiones % bloquean al jugador y no muestran sprite. Los enemigos con
`enemy.spawnMode: "seeded"` eligen cantidad y posiciones entre los puntos dibujados;
los de aparición fija siempre están presentes. La exportación para el juego
conserva la apariencia, posición y función de estos sprites. Para patrullas explicitas o perfiles nativos del juego, usa las plantillas
originales. El editor informa de las combinaciones que el importador todavia no
admite, sin borrar datos.

Ejemplo editable: [marcadores.room.json](examples/marcadores.room.json).

**Archivo: Borrador del editor** permite descargar un diseño incompleto o con
funciones aún no admitidas por el juego. Se puede volver a importar para seguir
editándolo. El borrador no se debe colocar en `rooms/` hasta corregirlo.

El guardado automático, **Guardar**, **Mis cuartos**, deshacer/rehacer y los
controles móviles conservan también la información de compatibilidad. Son
diseños del navegador, separados de las partidas guardadas de la GBA.

## Pruebas

Ejecuta `npm test` con Node.js. Incluye pruebas de recursos BMP, geometría,
compatibilidad, reglas y controles con un adaptador DOM; no simula un dispositivo
físico. Los JSON de prueba completos están en `examples/game-rooms/`.

[Contrato de importación y exportación](EDITOR-FORMATO.md).
