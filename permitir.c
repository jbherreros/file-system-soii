#include "directorios.h"

int main(int argc, char const *argv[])
{
    char nombre_dispositivo[1024];
    unsigned int ninodo;
    unsigned char permisos;
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis: escribir <nombre_dispositivo> <ninodo> <permisos>\n");
        exit(EXIT_FAILURE);
    }
    strcpy(nombre_dispositivo, argv[1]);
    if (access(nombre_dispositivo, F_OK) == -1)
    {
        fprintf(stderr, "ERROR: No existe el archivo\n");
        exit(EXIT_FAILURE);
    }
    ninodo = atoi(argv[2]);
    permisos = atoi(argv[3]);
    if (permisos > 7)
    {
        fprintf(stderr, "Valor incorrecto para <permisos>");
        exit(EXIT_FAILURE);
    }
    if (bmount(nombre_dispositivo) == -1)
        exit(EXIT_FAILURE);
    if (mi_chmod_f(ninodo, permisos) == -1)
        exit(EXIT_FAILURE);
    if (bumount(nombre_dispositivo) == -1)
        exit(EXIT_FAILURE);
    return 0;
}