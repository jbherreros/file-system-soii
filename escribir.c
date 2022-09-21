#include "directorios.h"
#define OFFSET1 9000
#define OFFSET2 209000
#define OFFSET3 30725000
#define OFFSET4 409605000
#define OFFSET5 480000000
#define NOFFSETS 5

int main(int argc, char const *argv[]){
    int diferentes_inodos, escritos, ninodo, nbytes;
    struct STAT datos_inodo;
    unsigned int offsets[] = {OFFSET1, OFFSET2, OFFSET3, OFFSET4, OFFSET5};
    if (argc != 4){
        fprintf(stderr, "Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        fprintf(stderr, "Offsets: 9000, 209000, 30725000, 409605000, 480000000\n");
        fprintf(stderr, "Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n");
        exit(EXIT_FAILURE);
    }

    nbytes = strlen(argv[2]); // Longitud del texto
    char buffer[nbytes];      // Buffer del texto
    strcpy(buffer, argv[2]);
    diferentes_inodos = atoi(argv[3]);
    fprintf(stderr, "longitud del texto: %d\n\n", nbytes);
    
    if (bmount(argv[1]) == -1) exit(EXIT_FAILURE);
    
    if (diferentes_inodos == 0){
        ninodo = reservar_inodo('f', 6);
        if (ninodo == -1) exit(EXIT_FAILURE);

        for (int i = 0; i < NOFFSETS; i++){
            fprintf(stderr, "Nº inodo reservado: %u\n", ninodo);
            fprintf(stderr, "Offset: %u\n", offsets[i]);
            escritos = mi_write_f(ninodo, buffer, offsets[i], nbytes);
            if (escritos == -1) exit(EXIT_FAILURE);
            fprintf(stderr, "Bytes escritos: %d\n", escritos);
            if (mi_stat_f(ninodo, &datos_inodo) == -1) exit(EXIT_FAILURE);
            //fprintf(stderr, "DATOS INODO %d:\n", ninodo);
            //datos_STAT(datos_inodo);
            fprintf(stderr, "tamEnBytesLog=%u\n", datos_inodo.tamEnBytesLog);
            fprintf(stderr, "numBloquesOcupados=%u\n\n", datos_inodo.numBloquesOcupados);
        }
    }else if (diferentes_inodos == 1){
        for (int i = 0; i < NOFFSETS; i++){
            ninodo = reservar_inodo('f', 6);
            if (ninodo == -1) exit(EXIT_FAILURE);

            fprintf(stderr, "Nº inodo reservado: %u\n", ninodo);
            fprintf(stderr, "Offset: %u\n", offsets[i]);
            escritos = mi_write_f(ninodo, buffer, offsets[i], nbytes);
            if (escritos == -1) exit(EXIT_FAILURE);
            fprintf(stderr, "Bytes escritos: %d\n", escritos);
            if (mi_stat_f(ninodo, &datos_inodo) == -1) exit(EXIT_FAILURE);
            //fprintf(stderr, "DATOS INODO %d:\n", ninodo);
            //datos_STAT(datos_inodo);
            fprintf(stderr, "tamEnBytesLog=%u\n", datos_inodo.tamEnBytesLog);
            fprintf(stderr, "numBloquesOcupados=%u\n\n", datos_inodo.numBloquesOcupados);
        }
    }else{
        fprintf(stderr, "Valor incorrecto para <diferentes_inodos>\n");
        exit(EXIT_FAILURE);
    }

    if (bumount(argv[1]) == -1) exit(EXIT_FAILURE);
    
    return 0;
}