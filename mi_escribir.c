#include "directorios.h"

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
	char ruta[1024];
    int offset, nbytes, escritos;

    // Comprobamos los argumentos
    if(argc!=5){
        fprintf(stderr, "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        exit(EXIT_FAILURE);
    }

    offset = atoi(argv[4]); 
    strcpy(nombre_dispositivo,argv[1]); 
    strcpy(ruta, argv[2]); 
    nbytes = strlen(argv[3]); 
	char buffer[nbytes];
	strcpy(buffer, argv[3]);

    /*fprintf(stderr,"peto aqui? string %s\n", argv[4]);
	offset = atoi(argv[4]); 
    fprintf(stderr,"atoi(offset): %i\n\n",offset);*/

    // Comprobamos si la ruta es un fichero
    if(ruta[strlen(ruta)-1]=='/'){
        fprintf(stderr, "Error: Comando mi_escribir sólo para escritura en ficheros\n");
        exit(EXIT_FAILURE);
    } 

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    if((escritos = mi_write(ruta, buffer, offset, nbytes))<0){
        //fprintf(stderr, "Error: mi_write()\n"); // mi_write ya muestra el error
	    exit(EXIT_FAILURE);
    }
    
    fprintf(stderr, "Longitud texto: %d\n", nbytes);
    fprintf(stderr, "Bytes escritos: %d\n", escritos);

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}