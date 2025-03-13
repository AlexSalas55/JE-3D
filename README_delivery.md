SNOWHILL MADNESS
================

Nombre: Alex Salas y Oriol Bech
NIA: 254338, 240128
Email: alex.salas01@estudiant.upf.edu , oriol.bech01@estudiant.upf.edu

Youtube: https://youtu.be/xjwspTa5_o0

¿De qué va el juego?
--------------------
SnowHill Madness es un juego de esquí en el que los jugadores deben descender la montaña
lo más rápido posible, esquivando obstáculos y compitiendo contra el tiempo.  
Se puede jugar en solitario o en pantalla dividida con otro jugador.

El juego cuenta con dos modos:  
- **Modo Normal**: Puedes jugar solo o en pantalla dividida contra otro jugador.
- **Modo Training**: Permite practicar y conocer mejor los controles del juego antes de competir.

¿Cómo se juega?
-------------
- **Jugador 1:** W A S D para moverse y controlar la velocidad.
- **Jugador 2:** ← → ↑ ↓ Flechas direccionales para moverse y controlar la velocidad.


Sobre el código:
--------------
- **World**: Clase central que gestiona todo el mundo del juego, incluyendo entidades, cámaras, iluminación y colisiones.

- **Player**: Controla el comportamiento del jugador, incluyendo movimiento, físicas, animaciones y colisiones. Implementa la lógica de deslizar sobre la nieve.

- **Stage**: Permite cambiar entre diferentes pantallas del juego:
  - **MenuStage**: Pantalla principal con botones para iniciar el juego, modo multijugador y entrenamiento.
  - **PlayStage**: Stage donde se desarrolla la carrera (Normal o multijugador)
  - **TrainingStage**: Modo de práctica con tutorial.

IA:
--------------
Se ha utilizado inteligencia artificial para ayudar en varias partes del desarrollo del juego:
- **UI del Cronómetro:** La IA nos ayudó a hacer la UI del cronometro para ponerlo en la pantalla.
- **Colisiones:** Ayudó en la optimización del sistema de detección de colisiones.
- **Físicas del juego:** Nos ayudó en la definición y ajuste de la simulación de movimiento y velocidad de los jugadores.
- **Cámara:** Nos ayudo a la hora de hacer ajustes automaticos con la pendiente en la camara.
- **Otros aspectos generales:** Consultas sobre estructura del código, eficiencia y buenas prácticas.

Comentarios adicionales:
--------------
Esperamos que disfruten jugando a **SnowHill Madness** tanto como nosotros hemos disfrutado desarrollándolo.  
¡Buena suerte en la pista y que gane el mejor esquiador! 
