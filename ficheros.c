#include "ficheros.h"

/*
 * Escribe el contenido de un buffer de memoria de tamaño nbytes en un 
 * fichero/directorio a partir de la posición lógica indicada
 * Param: Fichero/directorio a escribir correspondiente al inodo (ninodo), contenido a escribir (buf_original),
 * posicion a escribir en el fichero (offset) y cantidad de bytes a escribir (nbytes)
 * Return: el número de bytes escritos (si ok será igual a nbytes)
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    int primerBL, ultimoBL, desp1, desp2, bfisico, escritos;
    char buf_bloque[BLOCKSIZE];
    struct inodo inodo;
    
    if(leer_inodo(ninodo,&inodo)==-1) return -1;

    if((inodo.permisos & 2)!=2){
        fprintf(stderr, BOLD RED "Error: Permiso de escritura denegado\n" RESET);
        return -1; // El inodo no tiene permisos de escritura
    } 

    primerBL=offset/BLOCKSIZE;
    ultimoBL=(offset+nbytes-1)/BLOCKSIZE;
    desp1=offset%BLOCKSIZE;
    desp2=(offset+nbytes-1)%BLOCKSIZE;

    if(primerBL==ultimoBL){ // Sólo un bloque involucrado
        mi_waitSem();
        bfisico=traducir_bloque_inodo(ninodo,primerBL,1);
        mi_signalSem();
        if(bread(bfisico, buf_bloque)==-1) return -1;
        memcpy(buf_bloque+desp1,buf_original,nbytes);
        if(bwrite(bfisico, buf_bloque)==-1) return -1;
        escritos=nbytes;

    } else { // Más de un bloque involucrado
        // Primer bloque lógico 
        mi_waitSem();
        bfisico=traducir_bloque_inodo(ninodo,primerBL,1);
        mi_signalSem();
        if(bread(bfisico, buf_bloque)==-1) return -1;
        memcpy(buf_bloque+desp1,buf_original,BLOCKSIZE-desp1);
        if(bwrite(bfisico, buf_bloque)==-1) return -1;
        escritos = BLOCKSIZE - desp1;

        // Bloques lógicos intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++){
            mi_waitSem();
            bfisico=traducir_bloque_inodo(ninodo,i,1);
            mi_signalSem();
            if(bwrite(bfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1){
                return -1;
            }
            escritos += BLOCKSIZE;
        }

        // Último bloque lógico
        mi_waitSem();
        bfisico=traducir_bloque_inodo(ninodo,ultimoBL,1);
        mi_signalSem();
        if(bread(bfisico, buf_bloque)==-1) return -1;
        memcpy(buf_bloque, buf_original+(nbytes-desp2-1), desp2+1);
        if(bwrite(bfisico, buf_bloque)==-1) return -1;
        escritos += desp2 + 1;

    }

    mi_waitSem();
    if(leer_inodo(ninodo, &inodo)==-1){
        mi_signalSem();
        return -1;
    }
    inodo.mtime=time(NULL);
    if ((offset+nbytes)>= inodo.tamEnBytesLog){
        inodo.ctime=time(NULL);
        inodo.tamEnBytesLog=offset+nbytes;
    }
    if (escribir_inodo(ninodo, inodo)==-1){
        mi_signalSem();
        return -1;
    } 

    mi_signalSem();
    return escritos;
}

/*
 * Lee contenido de un fichero/directorio correspondiente a un ninodo, y lo almacena en un buffer de memoria.
 * Param: Fichero/directorio a leer correspondiente al inodo (ninodo), donde se guarda la lectura(buf_original),
 * posicion a leer del fichero (offset) y cantidad de bytes a leer (nbytes)
 * Return: el número de bytes leídos (si ok será igual a nbytes)
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    unsigned int primerBL, ultimoBL, bfisico, desp1, desp2, leidos;
    char buf_bloque[BLOCKSIZE];
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == -1) return -1;

    if ((inodo.permisos & 4) != 4){
        fprintf(stderr, BOLD RED "Error: permiso de lectura denegado\n" RESET);
        return -1;
    }

    if (offset >= inodo.tamEnBytesLog){
        leidos = 0;
        return leidos; // No se lee nada
    }

    if ((offset + nbytes) >= inodo.tamEnBytesLog){
        nbytes = inodo.tamEnBytesLog - offset; // Lee hasta el EOF
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    if (primerBL == ultimoBL){ // Un solo bloque involucrado
        bfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (bfisico != -1){
            if (bread(bfisico, buf_bloque) == -1) return -1;
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        leidos = nbytes;

    }else{ // Más de un bloque involucrado
        // Primer bloque lógico
        bfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (bfisico != -1){
            if (bread(bfisico, buf_bloque) == -1) return -1;
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        leidos = BLOCKSIZE - desp1;

        // Bloques lógicos intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++){
            bfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (bfisico != -1){
                if (bread(bfisico, buf_bloque) == -1) return -1;
                memcpy(buf_original + (BLOCKSIZE - desp1)+(i-primerBL-1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            leidos += BLOCKSIZE;
        }

        // Último bloque lógico
        bfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (bfisico != -1){
            if (bread(bfisico, buf_bloque) == -1) return -1;
            memcpy(buf_original + (nbytes - desp2 -1 ), buf_bloque, desp2 + 1);
        }
        leidos += desp2 + 1;
    }

    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == -1){
        mi_signalSem();
        return -1;
    }
    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, inodo) == -1){
       mi_signalSem();
       return -1;
    }
   
    mi_signalSem();
    return leidos;
}

/*
 * Devuelve la metainformación de un fichero/directorio (correspondiente al ninodo).
 * Param: inodo correspondiente al fichero/directorio (ninodo), tipo STAT que es igual al inodo
 * pero sin punteros (p_stat)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1) return -1;
 
    //fprintf(stderr, "Nº inodo: %i\n", ninodo);
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    
    return 0;
}

/*
 * Cambia los permisos de un fichero/directorio (correspondiente a ninodo)
 * Param: inodo correspondiente al fichero/directorio (ninodo) y permisos nuevos (permisos)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    struct inodo inodo;

    mi_waitSem();
    if(leer_inodo(ninodo, &inodo)==-1){
        mi_signalSem();
        return -1;
    } 

    inodo.permisos=permisos;
    inodo.ctime=time(NULL);

    if(escribir_inodo(ninodo, inodo)==-1){
        mi_signalSem();
        return -1;
    }

    mi_signalSem();
    return 0;
}

/*
 * Trunca un fichero/directorio (correspondiente a ninodo)
 * Param: inodo correspondiente al fichero/directorio (ninodo) y número de bytes que se quiere truncar (nbytes)
 * Return: Devuelve la cantidad de bloques liberados, -1 si error
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    int primerBL, liberados;

    if(leer_inodo(ninodo, &inodo)==-1) return -1;

    if ((inodo.permisos & 2) != 2){
        fprintf(stderr, BOLD RED "Error: permiso de escritura denegado\n" RESET);
    }

    if(inodo.tamEnBytesLog < nbytes){
        fprintf(stderr, BOLD RED "Error: No se puede truncar más allá del final del EOF\n" RESET);
    }

    if (nbytes % BLOCKSIZE == 0){
        primerBL=nbytes/BLOCKSIZE;
    } else {
        primerBL=nbytes/BLOCKSIZE+1;
    }

    liberados=liberar_bloques_inodo(primerBL,&inodo);
    if (liberados == -1) return -1;

    inodo.mtime=time(NULL);
    inodo.ctime=time(NULL);
    inodo.tamEnBytesLog=nbytes;
    inodo.numBloquesOcupados-=liberados;

    if(escribir_inodo(ninodo, inodo)==-1) return -1;

    return liberados;
}


// Funciones auxiliares para imprimir por pantalla
void datos_inodo(struct inodo inodo){
   struct tm *ts;
   char atime[80];
   char mtime[80];
   char ctime[80];
   ts = localtime(&inodo.atime);
   strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
   ts = localtime(&inodo.mtime);
   strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
   ts = localtime(&inodo.ctime);
   strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
   printf("tipo: %c\n", inodo.tipo);
   printf("permisos: %d\n", inodo.permisos);
   printf("atime: %s\n", atime);
   printf("ctime: %s\n", ctime);
   printf("mtime: %s\n", mtime);
   printf("nlinks: %u\n", inodo.nlinks);
   printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);
   printf("numBloquesOcupados: %u\n", inodo.numBloquesOcupados);
}

void datos_STAT(struct STAT STAT){
   struct tm *ts;
   char atime[80];
   char mtime[80];
   char ctime[80];
   ts = localtime(&STAT.atime);
   strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
   ts = localtime(&STAT.mtime);
   strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
   ts = localtime(&STAT.ctime);
   strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
   fprintf(stderr, "tipo=%c\n", STAT.tipo);
   fprintf(stderr, "permisos=%d\n", STAT.permisos);
   fprintf(stderr, "atime: %s\n", atime);
   fprintf(stderr, "ctime: %s\n", ctime);
   fprintf(stderr, "mtime: %s\n", mtime);
   fprintf(stderr, "nlinks=%u\n", STAT.nlinks);
   fprintf(stderr, "tamEnBytesLog=%u\n", STAT.tamEnBytesLog);
   fprintf(stderr, "numBloquesOcupados=%u\n", STAT.numBloquesOcupados);
}