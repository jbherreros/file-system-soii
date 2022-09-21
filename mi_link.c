#include "directorios.h"

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
	char ruta_fichero_original[1024];
    char ruta_enlace[1024];

    // Comprobamos los argumentos
    if(argc!=4){
        fprintf(stderr, "Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo,argv[1]); 
    strcpy(ruta_fichero_original, argv[2]); 
    strcpy(ruta_enlace, argv[3]);

    // Comprobamos si ambas rutas son ficheros
    if((ruta_fichero_original[strlen(ruta_fichero_original)-1]=='/')||(ruta_enlace[strlen(ruta_enlace)-1]=='/')){
        fprintf(stderr, "Error: Comando mi_link sólo para enlazamiento de ficheros\n");
        exit(EXIT_FAILURE);
    }

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    // Enlazamos con mi_link()
	if(mi_link(ruta_fichero_original, ruta_enlace)<0){
        exit(EXIT_FAILURE);
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
