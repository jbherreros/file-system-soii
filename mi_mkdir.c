#include "directorios.h"

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
	char ruta[1024];
	unsigned char permisos;

    // Comprobamos los argumentos
    if(argc!=4){
        fprintf(stderr, "Sintaxis: ./mi_mkdir <disco> <permisos> </ruta/>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo,argv[1]);
    permisos=atoi(argv[2]);
    strcpy(ruta, argv[3]); 

    // Comprobamos si la ruta es un directorio
    if(ruta[strlen(ruta)-1]!='/'){
        fprintf(stderr, "Error: Comando mi_mkdir sólo para creación de directorios\n");
        exit(EXIT_FAILURE);
    }

    // Comprobamos si los permisos están dentro del rango  
    if(permisos < 0 || permisos > 7){ 
        fprintf(stderr, "Error: Permisos no válidos (0-7)\n");
        exit(EXIT_FAILURE);
    }

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    if(mi_creat(ruta, permisos)<0){
        //fprintf(stderr, "Error: mi_creat()\n"); // mi_creat() ya muestra el error
        exit(EXIT_FAILURE);
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}