#include "directorios.h"
#define tambuffer 3500

int main(int argc, char const *argv[]){
    char string[128];
    char buffer_texto[tambuffer];
    int ninodo, offset, leidos, total_leidos;
    struct STAT datos_inodo;

    if (argc != 3){
        fprintf(stderr, "Sintaxis: escribir <nombre_dispositivo> <ninodo>\n");
        exit(EXIT_FAILURE);
    }

    if (bmount(argv[1]) == -1) exit(EXIT_FAILURE);
    
    ninodo = atoi(argv[2]);
    offset = 0;
    leidos = 0;
    total_leidos = 0;
    memset(buffer_texto, 0, tambuffer);
    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    while (leidos  > 0){
        write(1, buffer_texto, leidos);
        memset(buffer_texto, 0, tambuffer);
        total_leidos += leidos;
        offset += tambuffer;
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    }

    sprintf(string, "\ntotal_leidos %d\n", total_leidos);
    write(2, string, strlen(string));
    if (mi_stat_f(ninodo, &datos_inodo) == -1) exit(EXIT_FAILURE);
    sprintf(string, "tamEnBytesLog %d\n", datos_inodo.tamEnBytesLog);
    write(2, string, strlen(string));
    if (bumount(argv[1]) == -1) exit(EXIT_FAILURE);

    return 0;
}