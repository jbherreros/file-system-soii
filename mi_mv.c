#include "directorios.h"

// No funciona todavía
int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024], ruta_origen[1024], destino[1024];

    if (argc != 4){
        fprintf(stderr, "Sintaxis: ./mi_mv <disco> </origen/nombre> </destino/>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo, argv[1]);
    strcpy(ruta_origen, argv[2]);
    strcpy(destino, argv[3]);
    
    // Comprobamos si la ruta destino es un directorio 
    if(destino[strlen(destino)-1]!='/'){
        fprintf(stderr, "Error: El destino tiene que ser un directorio\n");
        exit(EXIT_FAILURE);
    } 

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
    
    mi_mv(ruta_origen, destino);

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
}