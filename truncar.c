#include "ficheros.h"

int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024];
    int nbytes, ninodo;
    struct STAT datos;

    // Validación de sintaxis
    if (argc != 4){
        fprintf(stderr, "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo, argv[1]);
    nbytes=atoi(argv[3]);
    ninodo=atoi(argv[2]);

    if(bmount(nombre_dispositivo)==-1) return -1;

    if(nbytes==0){
        liberar_inodo(ninodo); 

    } else {
        mi_truncar_f(ninodo,nbytes);

    }

    if (mi_stat_f(ninodo, &datos) == -1) return -1;
    fprintf(stderr, "DATOS INODO %d\n", ninodo);
    datos_STAT(datos);

    if(bumount(argv[1])==-1) return -1;

}