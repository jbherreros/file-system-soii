#include "directorios.h"

int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024];
    char ruta[1024];
	struct STAT stat;

    // Comprobamos los argumentos
	if (argc != 3){
		fprintf(stderr, "Sintaxis: ./mi_stat <disco> </ruta>\n");
		exit(EXIT_FAILURE);
	}

	strcpy(nombre_dispositivo, argv[1]);
	strcpy(ruta, argv[2]);

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

	if (mi_stat(ruta, &stat) == -1){
		//fprintf(stderr, "Error: mi_stat()\n"); // mi_stat() ya muestra el error dentro de la función
		exit(EXIT_FAILURE);
	}

	/*unsigned int p_inodo_dir
	if ((error = buscar_entrada(ruta, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error);
        return -1;
    }*/

	// Imprimimos la información
	datos_STAT(stat);

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}