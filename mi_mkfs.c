#include "directorios.h"

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
    unsigned char buf[BLOCKSIZE];
    int nbloques; // atoi(argv[2]);
    int ninodos;

    // Comprobación de parámetros
    if (argc!=3){
        fprintf(stderr, "Error: Sintaxis ./mi_mkfs <nombre_dispositivo> <n_bloques>\n");
        exit(EXIT_FAILURE);
    }

    // Inicialización de variables: nbloques, ninodos, nombre_dispositivo  
    strcpy(nombre_dispositivo,argv[1]);
    nbloques=atoi(argv[2]);
    ninodos=nbloques/4; // Por convención en nuestro SF
    
    if(nbloques<=0){
        fprintf(stderr, "Error: n_bloques incorrecto\n");
        exit(EXIT_FAILURE);
    }

    // Montamos el disco (primer parámetro es el nombre del disco)
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    } 

    // Inicialización a 0 (borrar basura)
    memset(buf,0,BLOCKSIZE);
    for (int i = 0; i < nbloques; i++){
        if(bwrite(i,buf)==-1){
            fprintf(stderr, "Error: Inicialización dispositivo virtual %s\n", nombre_dispositivo);
            exit(EXIT_FAILURE);
        }

    }

    // Inicialización metadatos e inodo raíz
    if(initSB(nbloques, ninodos)<0){
        fprintf(stderr, "Error: Inicialización superbloque\n");
        exit(EXIT_FAILURE);
    }
    
    if(initMB()<0){
        fprintf(stderr, "Error: Inicialización mapa de bits\n");
        exit(EXIT_FAILURE);
    }

    if(initAI()<0){
        fprintf(stderr, "Error: Inicialización array de inodos\n");
        exit(EXIT_FAILURE);
    }

    int ninodo;
    if((ninodo=reservar_inodo('d',7))<0){
        fprintf(stderr, "Error: Reservar inodo raíz\n");
        exit(EXIT_FAILURE);
    }
    
    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount() de %s\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}