#include "directorios.h"

int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024], ruta_origen[1024], ruta_destino[1024];

    // Comprobamos los argumentos
    if (argc != 4){
        fprintf(stderr, "Sintaxis: ./mi_cp <disco> </ruta_directorio_origen/> </ruta_directorio_destino/>\n");
        return -1;
    }

    strcpy(nombre_dispositivo, argv[1]);
    strcpy(ruta_origen, argv[2]);
    strcpy(ruta_destino, argv[3]);

    // Comprobamos si la ruta_origen es un fichero
    if (ruta_origen[strlen(ruta_origen) - 1] != '/'){
        fprintf(stderr, "Error: Comando mi_cp sólo para copiar directorios\n");
        exit(EXIT_FAILURE);
    }

    // Comprobamos si la ruta_destino es un directorio
    if (ruta_destino[strlen(ruta_destino) - 1] != '/'){
        fprintf(stderr, "Error: La ruta destino tiene que ser un directorio\n");
        exit(EXIT_FAILURE);
    }

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    mi_cp(ruta_origen,ruta_destino);

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
}