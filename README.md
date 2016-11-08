# TP 2016 2C - Chamba

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
3. En una consola posicionarse en  "tp-2016-2c-Chamba/PokedexServer/Debug" y ejecutar ./PokedexServer
4. En la otra consola posicionarse en "tp-2016-2c-Chamba/PokedexCliente/Debug" y tener lo siguiente en cuenta:
5. Dentro de ese directorio tienen que tener creada una carpeta vacia que se llame "tmp" (va a ser el punto de montaje)
6. Escribir los siguientes comandos para setear el puerto e ip por variable de entorno
7. PUERTO=7666
8. IP_SERVER=127.0.0.1
9. export PUERTO
10. export IP_SERVER
11. Ejecutar ./PokedexCliente tmp

## Links útiles
* [Foro de issues del TP](https://github.com/sisoputnfrba/foro/issues)
* [Repositorios de la cátedra](https://github.com/sisoputnfrba)
* [Tutorial de Valgrind para Memory Leaks](https://docs.google.com/document/d/1flOJ2P2g9UGVRiruuA4OCF6nucbN_BWVI0WDlYTJNf4/edit#)



