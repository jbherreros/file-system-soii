#include "directorios.h"

int main(int argc, char const *argv[]){
	char nombre_dispositivo[1024];
	char ruta[1024];
	unsigned char permisos;
	int resultado;

    // Comprobamos los argumentos
	if (argc != 4){
		fprintf(stderr, "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n");
		exit(EXIT_FAILURE);
	}

    strcpy(nombre_dispositivo, argv[1]);
    strcpy(ruta, argv[3]);
    permisos = atoi(argv[2]);

    // Comprobamos si los permisos están dentro del rango 
    if (permisos < 0 || permisos > 7){
        fprintf(stderr, "Error: Permisos no válidos (0-7)\n");
		exit(EXIT_FAILURE);
	}

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

	if ((resultado = mi_chmod(ruta, permisos))<0){
		//fprintf(stderr, "Error: mi_chmod()\n"); // mi_creat ya muestra el error
		exit(EXIT_FAILURE);
	}

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}