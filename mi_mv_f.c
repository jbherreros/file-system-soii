#include "directorios.h"
#define BUFFERSIZE 8000
int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024], ruta_fichero[1024], ruta_destino[1024], *buffer;
    char buffer_texto[BUFFERSIZE];
    int offset, leidos;
    struct STAT stat;

    // Comprobamos los argumentos
    if (argc != 4){
        fprintf(stderr, "Sintaxis: ./mi_mv_f <disco> </ruta_fichero_origen> </ruta_directorio_destino/>\n");
        return -1;
    }

    strcpy(nombre_dispositivo, argv[1]);
    strcpy(ruta_fichero, argv[2]);
    strcpy(ruta_destino, argv[3]);

    // Comprobamos si la ruta_fichero es un fichero
    if (ruta_fichero[strlen(ruta_fichero) - 1] == '/'){
        fprintf(stderr, "Error: Comando mi_mv_f solo para mover ficheros\n");
        exit(EXIT_FAILURE);
    }

    // Comprobamos si la ruta_destino es un directorio
    if (ruta_destino[strlen(ruta_destino) - 1] != '/'){
        fprintf(stderr, "Error: La ruta destino tiene que ser un directorio\n");
        exit(EXIT_FAILURE);
    }

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    // Creamos el fichero en el directorio destino: ruta_destino
    if (mi_stat(ruta_fichero, &stat) == -1) exit(EXIT_FAILURE); // Comprobamos permisos
    buffer= strrchr(ruta_fichero, '/') + 1;
    strcat(ruta_destino,buffer);
    if (mi_creat(ruta_destino, stat.permisos) == -1) exit(EXIT_FAILURE); // Se hace la copia con los mismos permisos

    // Volcamos el contenido del fichero original en su copia
    offset = 0;
    leidos = 0;
    memset(buffer_texto, 0, BUFFERSIZE); // Recorremos el fichero original leyendolo
    while ((leidos = mi_read(ruta_fichero, buffer_texto, offset, BUFFERSIZE)) > 0){
        if (mi_write(ruta_destino, buffer_texto, offset, leidos) == -1) exit(EXIT_FAILURE); // Volcamos en el nuevo
        memset(buffer_texto, 0, BUFFERSIZE);
        offset += BUFFERSIZE;

    }

    // Borramos el fichero original
	if(mi_unlink(ruta_fichero)<0){
        exit(EXIT_FAILURE);
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
}