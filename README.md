# TP 2016 2C - Chamba

- [x] Funciones de osada completas
- [ ] Semáforos (Osada - ¿Mapa/Entrenador?)
- [ ] Corrección deadlock según tiempo de ingreso, etc.
- [ ] Memory Leaks
- [ ] Setear la fecha de acceso y modificación para los casos correspondientes (read, write, truncate, create)

## Para abrir un mapa (PuebloPaleta)
1. Buildear
2. Abrir una consola para el Mapa y posicionarse en "tp-2016-2c-Chamba/Mapa/Debug"
3. Escribir "./Mapa PuebloPaleta ../../PokedexConfig" y al dar enter se ejecuta el mapa correspondiente

## Para comenzar un entrenador (Red)
1. Buildear
2. Abrir una consola para el Entrenador y posicionarse en "tp-2016-2c-Chamba/Entrenador/Debug"
3. Escribir "./Entrenador Red ../../PokedexConfig" y al dar enter se conecta al primer mapa de su hoja de ruta.

TENER EN CUENTA CUAL ES EL PRIMER MAPA DE CADA ENTRENADOR YA QUE SINO ESTA LEVANTADO EL MAPA CORRESPONDIENTE, EXPLOTA.

## Para ejecutar el pokedexServer con el pokedexCliente
1. Buildear
2. Abrir una consola para server y otra para cliente
3. En la consola de PokedexServer ejecutar:
  * *export PUERTO=7666*
  * *./PokedexServer*

4. En la consola de PokedexCliente ejecutar:
  * *export PUERTO=7666*
  * *export IP_SERVER=127.0.0.1*
  * *./PokedexCliente carpetaDondeSeMonta*

5. Al terminar, posicionarse en PokedexConfig/Debug y desmontar con el comando:
  * *sudo umount carpetaDondeSeMonta*
  
## Links útiles
* [Foro de issues del TP](https://github.com/sisoputnfrba/foro/issues)
* [Repositorios de la cátedra](https://github.com/sisoputnfrba)
* [Tutorial de Valgrind para Memory Leaks](https://docs.google.com/document/d/1flOJ2P2g9UGVRiruuA4OCF6nucbN_BWVI0WDlYTJNf4/edit#)



