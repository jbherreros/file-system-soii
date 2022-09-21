#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;

/*
 * Monta el dispositivo virtual
 * Param: nombre del dispositivo (fichero)
 * Return: el descriptor del dispositivo virtual (fichero), -1 si error
 */
int bmount(const char *camino){
    if (descriptor > 0){
        close(descriptor);
    }

    umask(000);
    descriptor= open(camino, O_RDWR|O_CREAT, 0666); // O_RDWR: lectura/escritura 
                                                    // O_CREAT: si no existe el fichero lo crea
                                                    // 0666(octal): 110 --> rw- rw- rw- (permisos a usuario, grupos y otros)
    if(descriptor==-1){
      fprintf(stderr, "Error: bloques.c / bmount() / open()\n"); 
    } 

    if (!mutex) { // El semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem(); 
        if (mutex == SEM_FAILED) {
            return -1;
        }
    }

    return descriptor;
}

/*
 * Desmonta el dispositivo virtual
 * Return: 0 si se ha cerrado correctamente, -1 si ha habido un error
 */
int bumount(){
    descriptor=close(descriptor); // Devuelve 0, -1 si error

    if (descriptor==-1){
        fprintf(stderr, "Error: bloques.c / bmount() / open()\n"); 
    }

    deleteSem(); // Borramos el semáforo

    return descriptor;
}

/*
 * Escribe el contenido de un buffer de memoria apuntado por *buf (de tamaño BLOCKSIZE) 
 * en el bloque del dispositivo virtual especificado por el argumento nbloque.
 * Param: número de bloque al que acceder (nbloque) y puntero a la direccion del buffer(lo que hay que escribir)
 * Return: nbytes que se han escrito, -1 si ha habido un error
 */
int bwrite(unsigned int nbloque, const void *buf){
    int nbytes;

    if (lseek(descriptor, BLOCKSIZE*nbloque, SEEK_SET)<0){          // Desplazamiento para situarnos en la posición apuntada por nbloque
        fprintf(stderr, "Error: bloques.c / bwrite() / lseek()\n"); // SEEK_SET: punto de referencia (desde que posición se empieza,
        return -1;                                                  // en este caso el inicio del fichero)
    } 
    
    if ((nbytes=write(descriptor, buf, BLOCKSIZE))<0){
        fprintf(stderr, "Error: bloques.c / bwrite() / write()\n"); 
        return -1;
    }

    return nbytes;
}

/*
 * Lee del dispositivo virtual el bloque especificado por nbloque. Copia su contenido en 
 * un buffer de memoria apuntado por *buf.
 * Param: número de bloque al que acceder (nbloque) y puntero a la direccion del buffer(lo que se lee)
 * Return: nbytes que se han leído, -1 si ha habido un error
 */
int bread(unsigned int nbloque, void *buf){
    int nbytes;

    if (lseek(descriptor, BLOCKSIZE*nbloque, SEEK_SET)<0){ // Igual que en bwrite(), nos colocamos en el bloque a leer  
        fprintf(stderr, "Error: bloques.c / bread() / lseek()"); 
        return -1;                                                 
    } 
    
    if ((nbytes=read(descriptor, buf, BLOCKSIZE))<0){
        fprintf(stderr, "Error: bloques.c / bread() / read()"); 
        return -1;
    }

    return nbytes;
}

void mi_waitSem() {
    if (!inside_sc) { // inside_sc==0
        waitSem(mutex);
    }
    inside_sc++;
}

 
void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }
}

