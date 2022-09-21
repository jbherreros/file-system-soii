#include "directorios.h"

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
	char ruta_fichero[1024];

    // Comprobamos los argumentos
    if(argc!=3){
        fprintf(stderr, "Sintaxis: ./mi_rm <disco> </ruta_fichero>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo,argv[1]); 
    strcpy(ruta_fichero, argv[2]); 

    // Comprobamos si es un fichero
    if(ruta_fichero[strlen(ruta_fichero)-1]=='/'){
        fprintf(stderr, "Error: Comando mi_rm sólo para eliminar ficheros\n");
        exit(EXIT_FAILURE);
    }

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    // Desenlazamos con mi_unlink()
	if(mi_unlink(ruta_fichero)<0){
        exit(EXIT_FAILURE);
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
