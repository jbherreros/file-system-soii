#include "bloques.h"
#include <limits.h>
#include <time.h> 

#define posSB 0 // el superbloque se escribe en el primer bloque de nuestro FS
#define tamSB 1 // tamaño en bloques del superbloque
#define INODOSIZE 128 // tamaño en bytes de un inodo
#define NPUNTEROS (BLOCKSIZE / sizeof(unsigned int)) // 256
#define DIRECTOS 12
#define INDIRECTOS0 (NPUNTEROS + DIRECTOS)                            // 268
#define INDIRECTOS1 (NPUNTEROS * NPUNTEROS + INDIRECTOS0)             // 65.804
#define INDIRECTOS2 (NPUNTEROS * NPUNTEROS * NPUNTEROS + INDIRECTOS1) // 16.843.020

#define RED "\x1B[31m" // colores
#define BLU "\x1B[34m"
#define RESET "\x1B[0m"
#define BOLD "\e[1m"

struct superbloque {
    unsigned int posPrimerBloqueMB;                      // Posición del primer bloque del mapa de bits
    unsigned int posUltimoBloqueMB;                      // Posición del último bloque del mapa de bits
    unsigned int posPrimerBloqueAI;                      // Posición del primer bloque del array de inodos
    unsigned int posUltimoBloqueAI;                      // Posición del último bloque del array de inodos
    unsigned int posPrimerBloqueDatos;                   // Posición del primer bloque de datos
    unsigned int posUltimoBloqueDatos;                   // Posición del último bloque de datos
    unsigned int posInodoRaiz;                           // Posición del inodo del directorio raíz
    unsigned int posPrimerInodoLibre;                    // Posición del primer inodo libre
    unsigned int cantBloquesLibres;                      // Cantidad de bloques libres
    unsigned int cantInodosLibres;                       // Cantidad de inodos libres
    unsigned int totBloques;                             // Cantidad total de bloques
    unsigned int totInodos;                              // Cantidad total de inodos
    char padding[BLOCKSIZE - 12 * sizeof(unsigned int)]; // Relleno
};

struct inodo {     // comprobar que ocupa 128 bytes haciendo un sizeof(inodo)!!!
    unsigned char tipo;     // Tipo ('l':libre, 'd':directorio o 'f':fichero)
    unsigned char permisos; // Permisos (lectura y/o escritura y/o ejecución)
 
    /* Por cuestiones internas de alineación de estructuras, si se está utilizando
    un tamaño de palabra de 4 bytes (microprocesadores de 32 bits):
    unsigned char reservado_alineacion1 [2];
    en caso de que la palabra utilizada sea del tamaño de 8 bytes
    (microprocesadores de 64 bits): unsigned char reservado_alineacion1 [6]; */
    unsigned char reservado_alineacion1[6];
 
    time_t atime; // Fecha y hora del último acceso a datos: atime
    time_t mtime; // Fecha y hora de la última modificación de datos: mtime
    time_t ctime; // Fecha y hora de la última modificación del inodo: ctime
 
    /* comprobar el tamaño del tipo time_t para vuestra plataforma/compilador:
    printf ("sizeof time_t is: %d\n", sizeof(time_t)); */
 
    unsigned int nlinks;             // Cantidad de enlaces de entradas en directorio
    unsigned int tamEnBytesLog;      // Tamaño en bytes lógicos
    unsigned int numBloquesOcupados; // Cantidad de bloques ocupados zona de datos
 
    unsigned int punterosDirectos[12];  // 12 punteros a bloques directos
    unsigned int punterosIndirectos[3]; /* 3 punteros a bloques indirectos:
    1 indirecto simple, 1 indirecto doble, 1 indirecto triple */
 
    /* Utilizar una variable de alineación si es necesario  para vuestra plataforma/compilador   */
    char padding[INODOSIZE - 2 * sizeof(unsigned char) - 3 * sizeof(time_t) - 18 * sizeof(unsigned int) - 6 * sizeof(unsigned char)];
    // Hay que restar también lo que ocupen las variables de alineación utilizadas!!!
};

int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB(); 
int initAI();
int escribir_bit(unsigned int nbloque, unsigned int bit); // A partir de aquí nivel 3
char leer_bit(unsigned int nbloque);
int reservar_bloque();
int liberar_bloque(unsigned int nbloque);
int escribir_inodo(unsigned int ninodo, struct inodo inodo);
int leer_inodo(unsigned int ninodo, struct inodo *inodo);
int reservar_inodo(unsigned char tipo, unsigned char permisos);
int obtener_nrangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr); // A partir de aquí nivel 4
int obtener_indice(unsigned int nblogico, unsigned int nivel_punteros);
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar);
int liberar_inodo(unsigned int ninodo); // A partir de aquí nivel 6
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo);