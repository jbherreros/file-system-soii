#include "directorios.h"

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
	char ruta[1024];
    char buffer[5000];
    int detailed_ls=0; // Por norma hará un ls simplificado
    int numentradas;

    // Comprobamos los argumentos
    if(argc!=3 && argc!=4){ 
        fprintf(stderr, "Sintaxis: ./mi_ls <disco> </ruta_directorio> || ./mi_ls <disco> </ruta_directorio> -d\n"); // -d significa modo detallado (más información)
        return EXIT_FAILURE;
    }

    // Comprobamos el argumento -d
    if(argc==4){
        if(strcmp("-d", argv[3])==0){ // Si aparece la -d, hara un ls detallado
            detailed_ls=1;
        } else {
            fprintf(stderr, "Sintaxis: mi_ls <disco> </ruta_directorio> || mi_ls <disco> </ruta_directorio> -d\n"); // -d significa modo detallado (más información)
            return EXIT_FAILURE;
        }
    }

    strcpy(nombre_dispositivo,argv[1]);
    strcpy(ruta, argv[2]);

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    if((numentradas=mi_dir(ruta, buffer, detailed_ls))<0) return EXIT_FAILURE;

    // Directorio vacío
    if(numentradas==0){
        exit(EXIT_SUCCESS);
    } 

    // Imprimimos información
    if(detailed_ls || (ruta[strlen(ruta)-1]!='/' && numentradas==1)){
        printf("Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n");
        printf("--------------------------------------------------------------\n");
    }
    printf("%s", buffer);
    if(ruta[strlen(ruta)-1]=='/' && detailed_ls){
        printf("Total entradas: %d\n", numentradas);
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}