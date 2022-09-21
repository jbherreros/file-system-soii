#include "directorios.h"

int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024], ruta_fichero[1024], ruta_destino[1024];

    // Comprobamos los argumentos
    if (argc != 4){
        fprintf(stderr, "Sintaxis: ./mi_cp_f <disco> </ruta_fichero> </ruta_destino/>\n");
        return -1;
    }

    strcpy(nombre_dispositivo, argv[1]);
    strcpy(ruta_fichero, argv[2]);
    strcpy(ruta_destino, argv[3]);

    // Comprobamos si la ruta_fichero es un fichero
    if (ruta_fichero[strlen(ruta_fichero) - 1] == '/'){
        fprintf(stderr, "Error: Comando mi_cp_f sólo para copiar ficheros\n");
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

    mi_cp_f(ruta_fichero,ruta_destino);

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
}