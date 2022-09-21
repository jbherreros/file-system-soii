#include "ficheros.h" 
#define ERROR_CAMINO_INCORRECTO -1
#define ERROR_PERMISO_LECTURA -2
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA -3
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO -4
#define ERROR_PERMISO_ESCRITURA -5
#define ERROR_ENTRADA_YA_EXISTENTE -6
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO -7
#define tamnombre 60 //tamaño del nombre de directorio o fichero, en ext2 = 256
#define PROFUNDIDAD 32 //profundidad máxima del árbol de directorios

struct entrada { // 64 bytes: en un BLOCKSIZE cabran 60 entradas de directorio
  char nombre[tamnombre]; // 60 bytes
  unsigned int ninodo; // 4 bytes
};

struct UltimaEntrada{
  char camino [tamnombre*PROFUNDIDAD];
  unsigned int p_inodo;
};

struct ruta{
  char ruta_origen[tamnombre*PROFUNDIDAD];
  char ruta_destino[tamnombre*PROFUNDIDAD];
};

#define tamentrada sizeof(struct entrada)

// Nivel 7
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
void mostrar_error_buscar_entrada(int error, char *comments);
// Nivel 8
int mi_creat(const char *camino, unsigned char permisos);
int mi_dir(const char *camino, char *buffer, char tipo);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);
// Nivel 9
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);
// Nivel 10
int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);

// Opcional
int mi_rn(const char *camino, const char *nombre_nuevo);
int mi_mv(char *ruta_origen, char *destino);
int mi_cp_f(char *ruta_fichero, char *ruta_destino);
int mi_cp(char *ruta_origen, char *ruta_destino);
int mi_rm_r(const char *camino);

