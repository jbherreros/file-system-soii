#include "directorios.h"

int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024], nombre_actual[1024], nombre_nuevo[1024];
    //unsigned int p_inodo_dir, p_inodo, p_entrada;
    //int error;

    // Comprobamos los argumentos
    if (argc != 4){
        fprintf(stderr, "Sintaxis: ./mi_rn <disco> </ruta/nombre_actual> <nombre_nuevo>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo, argv[1]);
    strcpy(nombre_actual, argv[2]);
    strcpy(nombre_nuevo, argv[3]);
    
    // Nombre nuevo no puede incluir ninguna barra ('/')
    if (strchr(nombre_nuevo,'/')!=NULL){
        fprintf(stderr, "Error: nombre_nuevo no puede contener ninguna barra '/'\n");
        exit(EXIT_FAILURE);
    }

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
    
    if(mi_rn(nombre_actual, nombre_nuevo)<0){
        exit(EXIT_FAILURE);
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
}