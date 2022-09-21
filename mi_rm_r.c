#include "directorios.h"

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
	char ruta_directorio[1024];

    // Comprobamos los argumentos
    if(argc!=3){
        fprintf(stderr, "Sintaxis: ./mi_rm_r <disco> </ruta_directorio/>");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo,argv[1]); 
    strcpy(ruta_directorio, argv[2]); 

    if((strlen(ruta_directorio)==1)&&(strcmp(ruta_directorio,"/")==0)){
        fprintf(stderr, "Error: No se puede eliminar el directorio raíz\n");
        exit(EXIT_FAILURE);
    }

    // Comprobamos si es un directorio
    if(ruta_directorio[strlen(ruta_directorio)-1]!='/'){
        fprintf(stderr, "Error: Comando mi_rm_r sólo para eliminar directorios\n");
        exit(EXIT_FAILURE);
    }

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    // Ejecutamos el algoritmo recursivo mi_rm_r()
	if(mi_rm_r(ruta_directorio)<0){
        exit(EXIT_FAILURE);
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
