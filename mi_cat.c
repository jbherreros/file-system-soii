#include "directorios.h"
#define tambuffer 4096  

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
	char camino[1024];
    char string[128];
    int offset, total_leidos, leidos;// error;
    char buffer[tambuffer];
    //struct STAT datos_inodo;
	//unsigned int p_inodo_dir, p_inodo, p_entrada;

    // Comprobamos los argumentos
    if(argc!=3){
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo,argv[1]); 
    strcpy(camino, argv[2]); 

    // Comprobamos si la ruta es un fichero
    if(camino[strlen(camino)-1]=='/'){
        fprintf(stderr, "Error: Comando mi_cat sólo para lectura de ficheros\n");
        exit(EXIT_FAILURE);
    }
    
    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    // Leemos
    offset = 0;
	leidos = 0;
	total_leidos = 0;
	memset(buffer, 0, tambuffer);

	while ((leidos = mi_read(camino, buffer, offset, tambuffer)) > 0){
		write(1, buffer, leidos);
		memset(buffer, 0, tambuffer);
		total_leidos += leidos;
		offset += tambuffer;
	}

    sprintf(string, "\ntotal_leidos %d\n", total_leidos);
	write(2, string, strlen(string));

    /*// Comprobamos si el tamEnBytesLog = total_leidos
	if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
		mostrar_error_buscar_entrada(error, NULL);
		return -1;
	}*/

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}