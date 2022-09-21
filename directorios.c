#include "directorios.h"
#define tamcache 100
static struct UltimaEntrada UltimasEntradas[tamcache];
int punteroCache = 0;

/*
 * Dada una cadena de carácteres camino (que comienza por '/'), seprara su contenido en dos cadenas diferentes
 * Param: camino completo (*camino), puntero que contendrá el primer directorio/fichero (*inicial), el resto del camino será contenido en (*final)
 * y si es un fichero o directorio (*tipo)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    char *found;

    if(camino==NULL) return -1; // Si está vacío error
    if(camino[0]!='/') return -1; // El primer carácter no es '/'

    // Busca el primer '/' del camino
    found=strchr(camino+1,'/'); 
    memset(inicial, 0, strlen(inicial));

    if(!found){ // Si no lo encuentra es un fichero
        strcpy(inicial, camino+1);
        strcpy(final, "\0");
        strcpy(tipo, "f");

    } else { // Es un directorio
        strncpy(inicial, camino+1, found-(camino+1)); 
        strcpy(final, found);
        strcpy(tipo, "d");
    }

    return 0;
}

/*
 * Busca una determinada entrada entre las entradas del inodo padre y la crea si no existe
 * Param: (*camino_parcial), puntero al inodo correspondiente al directorio padre (*p_inodo_dir), el número de inodo correspondiente a la entrada buscada (*p_inodo),
 * el número de entrada dentro del inodo (*p_entrada), reservar o no la entrada (reservar) y permisos de la nueva entrada (permisos)
 * Return: Los punteros de los parámetros y 0 si todo ha ido bien, -1 si error
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    struct inodo inodo_dir;
    //struct entrada entrada;
    struct entrada entradas[BLOCKSIZE / tamentrada];

    char inicial[strlen(camino_parcial)]; // sizeof(entrada.nombre) y sizeof(tamnombre) no me funcionan, con strlen(camino_parcial) sí
    char final[strlen(camino_parcial)];
    char tipo[sizeof(char)];
    memset(inicial, 0, sizeof(inicial));
    memset(final, 0, sizeof(final));
    memset(tipo, 0, sizeof(tipo));
    

    int cant_entradas_inodo;
    int num_entrada_inodo;

    if (!strcmp(camino_parcial, "/")){
        *p_inodo = 0;
        *p_entrada = 0;
        return 0;
    }

    if (extraer_camino(camino_parcial, inicial, final, tipo) == -1) return ERROR_CAMINO_INCORRECTO;
    //fprintf(stderr, "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == -1) return EXIT_FAILURE;

    if ((inodo_dir.permisos & 4) != 4){
        //fprintf(stderr, "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n", *p_inodo_dir);
        return ERROR_PERMISO_LECTURA;
    }

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / tamentrada;
    memset(entradas, 0, BLOCKSIZE); // ?¿?¿?¿?¿
    num_entrada_inodo = 0;

    if (cant_entradas_inodo > 0){
        //leer el primer bloque de entradas y mirarlas desde el buffer hasta acabar de leer el buffer
        if (mi_read_f(*p_inodo_dir, entradas, 0, BLOCKSIZE) == -1) return -1;
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entradas[num_entrada_inodo % (BLOCKSIZE / tamentrada)].nombre)!=0)){
            num_entrada_inodo++;
            if ((num_entrada_inodo % (BLOCKSIZE / tamentrada)) == 0 && num_entrada_inodo < cant_entradas_inodo){ // múltiplo del número de entradas en un bloque
                //leer el siguiente bloque de entradas
                if (mi_read_f(*p_inodo_dir, entradas, num_entrada_inodo * tamentrada, BLOCKSIZE) == -1) return -1;
            
            } 
        }
    }

    if (num_entrada_inodo == cant_entradas_inodo){
        switch (reservar){
        case 0: //modo consulta. Como no existe retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1: //modo escritura
            if (inodo_dir.tipo == 'f'){
                //fprintf(stderr, "[buscar_entrada()→ No se puede crear entrada en un fichero]\n");
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }

            if ((inodo_dir.permisos & 2) != 2){
                //fprintf(stderr, "[buscar_entrada()→ El inodo %d no tiene permisos de escritura]\n", *p_inodo_dir);
                return ERROR_PERMISO_ESCRITURA;

            } else {
                strcpy(entradas[num_entrada_inodo % (BLOCKSIZE / tamentrada)].nombre, inicial);
                if (!strcmp(tipo, "d")){ //es un directorio
                    if (!strcmp(final, "/")){
                        entradas[num_entrada_inodo % (BLOCKSIZE / tamentrada)].ninodo = reservar_inodo('d', permisos);
                        //fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n", entradas[num_entrada_inodo].ninodo, permisos, entradas[num_entrada_inodo].nombre);
                        //fprintf(stderr, "[buscar_entrada()→ creada entrada: %s, %d]\n", entradas[num_entrada_inodo].nombre, entradas[num_entrada_inodo].ninodo);
                    } else {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }

                } else { //es un fichero
                    entradas[num_entrada_inodo % (BLOCKSIZE / tamentrada)].ninodo = reservar_inodo('f', permisos);
                    //fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n", entradas[num_entrada_inodo].ninodo, permisos, entradas[num_entrada_inodo].nombre);
                    //fprintf(stderr, "[buscar_entrada()→ creada entrada: %s, %d]\n", entradas[num_entrada_inodo].nombre, entradas[num_entrada_inodo].ninodo);
                }

                // if (mi_write_f(*p_inodo_dir, entradas, (num_entrada_inodo / (BLOCKSIZE / tamentrada)) * BLOCKSIZE, BLOCKSIZE) == -1)
                if (mi_write_f(*p_inodo_dir, &entradas[num_entrada_inodo % (BLOCKSIZE / tamentrada)], num_entrada_inodo * tamentrada, tamentrada) == -1){
            
                    if (entradas[num_entrada_inodo % (BLOCKSIZE / tamentrada)].ninodo != -1){
                        liberar_inodo(entradas[num_entrada_inodo % (BLOCKSIZE / tamentrada)].ninodo);
                    }

                    return EXIT_FAILURE;
                }
            }
        }
    }
    

    if (!strcmp(final, "/") || !strcmp(final, "\0")){
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) return ERROR_ENTRADA_YA_EXISTENTE;
        *p_inodo = entradas[num_entrada_inodo % (BLOCKSIZE / tamentrada)].ninodo;
        *p_entrada = num_entrada_inodo;
        return EXIT_SUCCESS;

    } else {
        *p_inodo_dir = entradas[num_entrada_inodo % 16].ninodo;
        return (buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos));

    }

    return EXIT_SUCCESS;
}

/*
 * Crea un fichero/directorio y su entrada de directorio
 * Param: camino del fichero directorio a crear (*camino_parcial) y permisos del fichero/directorio (permisos)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int mi_creat(const char *camino, unsigned char permisos){
    mi_waitSem();
    unsigned int p_inodo_dir, p_inodo, p_entrada, error;
    p_inodo_dir=0;

    if((error=buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos))!=EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error,NULL); // Muestra el error si se ha producido
        mi_signalSem();
        return -1;
    }

    if(error == 0 && p_inodo==0){
        fprintf(stderr, BOLD RED "Error: El directorio raíz ya está creado\n" RESET);
        mi_signalSem();
        return -1;
    }

    mi_signalSem();
    return 0;
}

/*
 * Obtiene el contenido de un directorio (las diferentes entradas) y las mete en un buffer
 * Param: camino del directorio (*camino), donde se carga el contenido a del directorio (*buffer) y forma de mostrar los datos (tipo)
 * Return: número de entradas (numentradas), -1 si error, -2 si error permisos
 */
int mi_dir(const char *camino, char *buffer, char tipo){
    struct inodo inodo;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    int numentradas;
    int nentrada;
    struct entrada entradas[BLOCKSIZE / tamentrada];
    struct tm *tm;
    char tmp[100], tam[100], nom[100];
    p_inodo_dir = 0;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error,NULL);
        return -1;
    }

    if (leer_inodo(p_inodo, &inodo) == -1) return -1;
    
    memset(buffer,0,BLOCKSIZE); // Se me añadía basura, por lo que lo incializo a cero
    if (inodo.tipo == 'f'){ // Comprueba si es un fichero  
        strcat(buffer, "f\t");

        // Concatenamos permisos
        if (inodo.permisos & 4){ // Lectura
            strcat(buffer, "r");
        } else {
            strcat(buffer, "-");
        }

        if (inodo.permisos & 2){ // Escritura
            strcat(buffer, "w");
        } else {
            strcat(buffer, "-");
        }

        if (inodo.permisos & 1){ // Ejecución
            strcat(buffer, "x\t");
        } else {
            strcat(buffer, "-\t");
        }

        // mtime del inodo
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "\t%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);

        // Tamaño del inodo
        sprintf(tam, "%d\t", inodo.tamEnBytesLog);
        strcat(buffer, tam);

        // Nombre de la entrada
        if (mi_read_f(p_inodo_dir, entradas, p_entrada * tamentrada, tamentrada) == -1) return -1;
        sprintf(nom, BOLD RED "%s\n" RESET, entradas[0].nombre);
        strcat(buffer, nom);
        return 1;
    }

    if ((inodo.permisos & 2) != 2) return -2; // comprueba el permiso de lectura    

    if (inodo.tipo=='d' && camino[strlen(camino)-1]!='/'){
        fprintf(stderr, BOLD RED "Error: La ruta indicada no es correcta\n" RESET);
        return -1;
    }

    numentradas = inodo.tamEnBytesLog / tamentrada;
    nentrada = 0;
    if (numentradas > 0){
        if (mi_read_f(p_inodo, entradas, 0, BLOCKSIZE) == -1) return -1;

        while (nentrada < numentradas){
            if (leer_inodo(entradas[nentrada % (BLOCKSIZE / tamentrada)].ninodo, &inodo) == -1) return -1;
            
            if (tipo){
                // Tipo de inodo
                if (inodo.tipo == 'f'){
                    strcat(buffer, "f\t");
                }
                else{
                    strcat(buffer, "d\t");
                }
                // Permisos del inodo
                if (inodo.permisos & 4){ // Lectura
                    strcat(buffer, "r");
                } else {
                    strcat(buffer, "-");
                }

                if (inodo.permisos & 2){ // Escritura
                    strcat(buffer, "w");
                } else {
                    strcat(buffer, "-");
                }

                if (inodo.permisos & 1){ // Ejecución
                    strcat(buffer, "x\t");
                } else {
                    strcat(buffer, "-\t");
                }
                // mtime del inodo
                tm = localtime(&inodo.mtime);
                sprintf(tmp, "\t%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);
                // Tamaño del inodo
                sprintf(tam, "%d\t", inodo.tamEnBytesLog);
                strcat(buffer, tam);
            }

            // Nombre de la entrada
            if (inodo.tipo == 'f'){
                sprintf(nom, BOLD RED "%s" RESET, entradas[nentrada % (BLOCKSIZE / tamentrada)].nombre);
            } else {
                sprintf(nom, BOLD BLU "%s" RESET, entradas[nentrada % (BLOCKSIZE / tamentrada)].nombre);
            }
            
            strcat(buffer, nom);
            switch (tipo){
                case 0:
                    strcat(buffer, "\t");
                    break;

                case 1:
                    strcat(buffer, "\n");
                    break;
            }

            nentrada++;
            if (((nentrada) % (BLOCKSIZE / tamentrada)) == 0){
                // Leo el siguiente bloque de entradas
                if (mi_read_f(p_inodo, entradas, nentrada * tamentrada, BLOCKSIZE) == -1) return -1;
            }
        }
        
        if (!tipo) strcat(buffer, "\n");
    }

    return numentradas;
}

/*
 * Cambia los permisos de un fichero/directorio
 * Param: camino del fichero/directorio (*camino) y permisos (permisos)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int mi_chmod(const char *camino, unsigned char permisos){
    struct inodo inodo;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;

    p_inodo_dir = 0;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error,NULL);
        return -1;
    }

    if (leer_inodo(p_inodo, &inodo) == -1) return -1;
   
    return mi_chmod_f(p_inodo, permisos);
}

/*
 * Obtiene la información del inodo correspondiente al fichero/directorio seleccionado
 * Param: camino del fichero/directorio (*camino) y puntero stat (*p_stat)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int mi_stat(const char *camino, struct STAT *p_stat){
    //struct inodo inodo;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;

    p_inodo_dir = 0;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error,NULL);
        return -1;
    }

    //if (leer_inodo(p_inodo, &inodo) == -1) return -1;

    return mi_stat_f(p_inodo, p_stat); // mi_stat_f devuelve 0 si todo ha ido bien, -1 si error 
}

/*
 * Escribe el contenido de un buffer en un fichero
 * Param: camino del fichero (*camino), cotenido a escribir (*buf), offset (offset), cantidad de nytes a escribir (nbytes)
 * Return: numero de bytes escritos (resultado), -1 si error
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int encontrado = 0, error;
    struct UltimaEntrada ultima;
    struct inodo inodo;

    // Comprobar caché de directorios
    int i = 0;
    while (i < tamcache && !encontrado){
        if (strcmp(camino, UltimasEntradas[i].camino) == 0){
            encontrado = 1;
            p_inodo = UltimasEntradas[i].p_inodo;
        }
        i++;
    }

    // Si no lo hemos encontrado, buscamos la entrada
    if (!encontrado){
        p_inodo_dir = 0;
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
            mostrar_error_buscar_entrada(error,NULL);
            return -1;

        } else {
            // Actualizar caché
            //fprintf(stderr,"[mi_write() → Actualizamos la caché de escritura]\n");
            strcpy(ultima.camino, camino);
            ultima.p_inodo = p_inodo;
            UltimasEntradas[punteroCache] = ultima;
            punteroCache = (punteroCache + 1) % tamcache;
        }
    } else {
        //fprintf(stderr,"[mi_write() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n");
    }

    // Control de error aparecido al intentar ejecutar ./mi_escribir disco /dir texto 0
    // dir se tratababa de un directorio, y aun asi escribía. Con esto lo solucionamos
    if(leer_inodo(p_inodo, &inodo)==-1) return -1;
    if (inodo.tipo=='d'){
        fprintf(stderr, BOLD RED "Error: No es posible escribir en un directorio\n" RESET);
        return -1;
    }

    // Escribimos en el fichero
    return mi_write_f(p_inodo, buf, offset, nbytes);
}

/*
 * Lee un fichero y lo vuelca en un buffer
 * Param: camino del fichero (*camino), donde se vuelca el contenido a leer (*buf), offset (offset), cantidad de nytes a leer (nbytes)
 * Return: numero de bytes leídos (resultado), -1 si error
 */
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int encontrado = 0, error;
    struct UltimaEntrada ultima;
    //struct inodo inodo;

    // Comprobar caché de directorios
    int i = 0;
    while (i < tamcache && !encontrado){
        if (strcmp(camino, UltimasEntradas[i].camino) == 0){
            encontrado = 1;
            p_inodo = UltimasEntradas[i].p_inodo;
        }
        i++;
    }

    // Si no lo hemos encontrado, buscamos la entrada
    if (!encontrado){
        p_inodo_dir = 0;
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
            mostrar_error_buscar_entrada(error,NULL);
            return -1;

        } else {
            // Actualizar caché
            //fprintf(stderr,"[mi_read() → Actualizamos la caché de lectura]\n");
            strcpy(ultima.camino, camino);
            ultima.p_inodo = p_inodo;
            UltimasEntradas[punteroCache] = ultima;
            punteroCache = (punteroCache + 1) % tamcache;
        }
    } else {
        //fprintf(stderr,"[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n");
    }

    /*// Control de error aparecido al intentar ejecutar ./mi_cat disco /dir 
    // dir se trataba de un directorio, y aun asi leia las entradas y las imprimía por pantalla
    // Con esto lo solucionamos
    if(leer_inodo(p_inodo, &inodo)==-1) return -1;
    if (inodo.tipo=='d'){
        fprintf(stderr, "Error: No es posible leer un directorio\n");
        return -1;
    }*/
    // Leemos el fichero
    return mi_read_f(p_inodo, buf, offset, nbytes);
}

/*
 * Crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1
 * Param: camino de los ficheros (*camino1 y *camino2)
 * Return:  0 si todo ha ido bien, -1 si error
 */
int mi_link(const char *camino1, const char *camino2){
    mi_waitSem();
    unsigned int p_inodo_dir1, p_inodo_dir2, p_inodo1, p_inodo2, p_entrada1, p_entrada2;
    struct inodo inodo1;
    struct entrada entrada;
    int error;
    p_inodo_dir1 = 0;
    p_inodo_dir2 = 0;
    
    // Busco el inodo del camino1
    if ((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error,NULL);
        mi_signalSem();
        return -1;
    }
    
    if (leer_inodo(p_inodo1, &inodo1) == -1){
        mi_signalSem();
        return -1;
    } 

    // Comprobamos si tiene permisos de lectura
    if ((inodo1.permisos & 4) != 4){
        fprintf(stderr, BOLD RED "Error: Permiso denegado de lectura %s" RESET, camino1);
        mi_signalSem();
        return -1;
    }

    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error, NULL);
        mi_signalSem();
        return -1;
    }
    
    // Leer la entrada del camino 2
    if (mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * tamentrada, tamentrada) == -1){
        mi_signalSem();
        return -1;
    } 

    // Crear el enlace y escribir la entrada
    entrada.ninodo = p_inodo1;
    if (mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * tamentrada, tamentrada) == -1){
        mi_signalSem();
        return -1;
    }
    
    // Liberar inodo camino 2
    if (liberar_inodo(p_inodo2) == -1){
        mi_signalSem();
        return -1;
    }

    // Actualizar metadatos
    inodo1.nlinks++;
    inodo1.ctime = time(NULL);

    // Guardar inodo
    if (escribir_inodo(p_inodo1, inodo1) == -1){
        mi_signalSem();
    } 
    
    mi_signalSem();
    return EXIT_SUCCESS;
}

/*
 * Borra la entrada de directorio especificada
 * Param: camino del fichero (*camino)
 * Return:  0 si todo ha ido bien, -1 si error
 */
int mi_unlink(const char *camino){
    mi_waitSem();
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error, numentradas;
    struct inodo inodo;
    struct entrada entrada;
    p_inodo_dir = 0;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error, NULL);
        mi_signalSem();
        return -1;
    }

    if (leer_inodo(p_inodo, &inodo) == -1){
        mi_signalSem();
        return -1;
    } 
    
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0){
        fprintf(stderr, BOLD RED "Error: El directorio %s no está vacío\n" RESET, camino);
        mi_signalSem();
        return -1;
    }

    if (leer_inodo(p_inodo_dir, &inodo) == -1){
        mi_signalSem();
        return -1;
    }

    //Eliminar la entrada en el directorio
    numentradas = inodo.tamEnBytesLog / tamentrada;

    if (p_entrada != (numentradas - 1)){
        if (mi_read_f(p_inodo_dir, &entrada, (numentradas - 1) * tamentrada, tamentrada) == -1){
            mi_signalSem();
            return -1;
        } 
        if (mi_write_f(p_inodo_dir, &entrada, p_entrada * tamentrada, tamentrada) == -1){
            mi_signalSem();
            return -1;
        }
    }

    if (mi_truncar_f(p_inodo_dir, inodo.tamEnBytesLog - tamentrada) == -1){
        mi_signalSem();
        return -1;
    } 
    
    if (leer_inodo(p_inodo, &inodo) == -1){
        mi_signalSem();
        return -1;
    } 
    
    inodo.nlinks--;
    if (inodo.nlinks == 0){
        if (liberar_inodo(p_inodo) == -1){
            mi_signalSem();
            return -1; //Eliminar el inodo
        } 
        
    } else {
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, inodo) == -1){
            mi_signalSem();
            return -1;
        }
    }

    mi_signalSem();
    return EXIT_SUCCESS;
}

/*
 * Renombra un fichero/directorio
 * Param: camino del fichero/directorio (*camino) y nombre nuevo para el fichero/directorio (*nombre_nuevo)
 * Return:  0 si todo ha ido bien, -1 si error
 */
int mi_rn(const char *camino, const char *nombre_nuevo){
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    char directorio_padre[1024];
    struct inodo inodo;
    char *pos;
    int error;

    // Comprobamos si existe el directorio/fichero actual
    p_inodo_dir = 0;
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error, "(directorio/fichero actual)");
        return -1;
    }

    // Si se intenta renombrar el directorio raíz, error
    if(p_inodo==0){
        fprintf(stderr, BOLD RED "Error: No es possible renombrar el directorio raíz\n" RESET);
        return -1;
    }

    // Comprobamos si se han introducido bien las rutas
    leer_inodo(p_inodo, &inodo);

    if((inodo.tipo=='d') && (camino[strlen(camino)-1]!='/')){
        fprintf(stderr, BOLD RED "Error: Los directorios deben acabar en '/' → %s/\n" RESET, camino);
        return -1;
    }

    if((inodo.tipo=='f') && (camino[strlen(camino)-1]=='/')){
        char ruta_correcta[strlen(camino)];
        strncpy(ruta_correcta,camino, strlen(camino)-1);
        fprintf(stderr, BOLD RED "Error: Los ficheros no pueden acabar en '/' → %s\n" RESET, ruta_correcta);
        return -1;
    }

    // Obtenemos la ruta_directorio_padre del archivo que queremos renombrar
    memset(directorio_padre,0,sizeof(directorio_padre)); // Esto es porque me producía basura el buffer
    pos=strchr(camino,'/'); 
    while ((pos!=NULL) && (strcmp(pos,"/")!=0)){ // pos[0]!='\0'
        strncpy(directorio_padre, camino, pos-(camino-1)); 
        pos=strchr(camino+strlen(directorio_padre),'/'); 

    }

    // Concatenamos nombre_nuevo al directorio padre
    strcat(directorio_padre,nombre_nuevo);
    if(pos!=NULL){ // 
        strcat(directorio_padre,"/");
    }

   // Comprobamos si ya existía un fichero/directorio para ese nombre_nuevo
    p_inodo_dir = 0;
    if ((error=buscar_entrada(directorio_padre, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) == EXIT_SUCCESS){
        fprintf(stderr, BOLD RED "Error: No se puede renombrar, ya existe la entrada %s en este directorio\n" RESET, directorio_padre);
        return -1;
    }

    // Si no existía, lo renombramos
    mi_write_f(p_inodo_dir, nombre_nuevo, p_entrada * sizeof(struct entrada), tamnombre);

    return -1;
}

/*
 * Mueve un fichero/directorio de ruta_origen a otro directorio destino
 * Param: ruta original del fichero/directorio (*ruta_origen) y ruta adónde se quiere mover (*destino)
 * Return:  0 si todo ha ido bien, -1 si error
 */
int mi_mv(char *ruta_origen, char *destino){
    unsigned int p_inodo_dir, p_inodo_dir_origen, p_inodo, p_inodo_origen, p_entrada, p_entrada_origen;
    char fd_a_mover[1024]; // Nombre del fichero/directorio a mover
    int error;
    int numentradas;
    struct entrada entrada;
    struct inodo inodo;
    char *pos;
    
    // Comprobamos si existe origen
    p_inodo_dir_origen = 0;
    if ((error = buscar_entrada(ruta_origen, &p_inodo_dir_origen, &p_inodo_origen, &p_entrada_origen, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error, "(no existe la entrada origen)");
        return -1;
    }

    // Comprobamos si se han introducido bien las rutas
    leer_inodo(p_inodo_origen, &inodo);
    if((inodo.tipo=='d') && (ruta_origen[strlen(ruta_origen)-1]!='/')){
        fprintf(stderr, BOLD RED "Error: Los directorios deben acabar en '/' → %s/\n" RESET, ruta_origen);
        return -1;
    }
    if((inodo.tipo=='f') && (ruta_origen[strlen(ruta_origen)-1]=='/')){
        char ruta_correcta[strlen(ruta_origen)];
        memset(ruta_correcta,0,strlen(ruta_correcta)); // Me imprime basura si no
        strncpy(ruta_correcta,ruta_origen, strlen(ruta_origen)+2);
        fprintf(stderr, BOLD RED "Error: Los ficheros no pueden acabar en '/' → %s\n" RESET, ruta_correcta);
        return -1;
    }

    // Si se intenta mover el directorio raíz, error
    if(p_inodo_origen==0){
        fprintf(stderr, BOLD RED "Error: No es possible mover el directorio raíz\n" RESET);
        return -1;
    }

    // Comprobamos si existe destino
    p_inodo_dir = 0;
    if ((error = buscar_entrada(destino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error, "(no existe la entrada destino)");
        return -1;
    }

    // Obtenemos el nombre del fichero/directorio a mover (última trozo de la ruta_origen)
    strcpy(fd_a_mover,ruta_origen);
    pos=strchr(ruta_origen+1,'/'); 
    while ((pos!=NULL) && (strcmp(pos,"/")!=0)){ // pos[0]!='\0'
        strcpy(fd_a_mover,pos);
        pos=strchr(pos+1,'/'); 

    }

    strcat(destino,fd_a_mover+1); // En destino tenemos la nueva ruta del fichero/directorio

    // Si ya existe la entrada a mover en destino, error
    p_inodo_dir = 0;
    if ((error = buscar_entrada(destino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) == EXIT_SUCCESS){
        fprintf(stderr, BOLD RED "Error: No se puede mover, ya existe la entrada %s en este directorio\n" RESET, destino);
        return -1;
    }
    
    // Asignamos el fichero o directorio origen al destino
    if(mi_link(ruta_origen,destino)<0){
        return -1;
    }

    leer_inodo(p_inodo_dir_origen, &inodo);

    // Igual que en unlink, para borrar una entrada
    numentradas = inodo.tamEnBytesLog / tamentrada;

    if (p_entrada_origen != (numentradas - 1)){
        if (mi_read_f(p_inodo_dir, &entrada, (numentradas - 1) * tamentrada, tamentrada) == -1){
            return -1;
        } 
        if (mi_write_f(p_inodo_dir_origen, &entrada, p_entrada_origen * tamentrada, tamentrada) == -1){
            return -1;
        }
    }
    if (mi_truncar_f(p_inodo_dir_origen, inodo.tamEnBytesLog - tamentrada) == -1) return -1;

    leer_inodo(p_inodo_origen, &inodo);

    inodo.nlinks--;
    inodo.ctime = time(NULL);
    if (escribir_inodo(p_inodo_origen, inodo) == -1) return -1;


    return 0;
}

/*
 * Copia un fichero a otro directorio destino
 * Param: ruta original del fichero (*ruta_fichero) y ruta adónde se quiere mover (*destino)
 * Return:  0 si todo ha ido bien, -1 si error
 */
int mi_cp_f(char *ruta_fichero, char *ruta_destino){
    int tambuffer=1024;
    char buffer_texto[tambuffer];
    char vacio[tambuffer];
    struct inodo inodo;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int offset, leidos, error;

    p_inodo_dir= 0;
    if ((error = buscar_entrada(ruta_fichero, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error, "(no existe la entrada origen)");
        return -1;
    }


    leer_inodo(p_inodo,&inodo); // inodo a copiar
    strcat(ruta_destino,strrchr(ruta_fichero, '/') + 1);
    
    // Se hace la copia con los mismos permisos
    if (mi_creat(ruta_destino, inodo.permisos) == -1) exit(EXIT_FAILURE); 

    // Volcamos el contenido del fichero original en su copia
    offset = 0;
    leidos = 0;
    memset(vacio, '\0', tambuffer); // Cadena vacía para comparar
    memset(buffer_texto, 0, tambuffer); // Recorremos el fichero original leyendolo
    leidos = mi_read(ruta_fichero, buffer_texto, offset, tambuffer);
    while (leidos > 0){
        //write(2, buffer_texto, leidos); // Para imprimir lo que leemos
        
        if(memcmp(buffer_texto, vacio, tambuffer)!=0){
            if (mi_write(ruta_destino, buffer_texto, offset, leidos) == -1) exit(EXIT_FAILURE); // Volcamos en el nuevo
        }   

        memset(buffer_texto, 0, tambuffer);
        offset += leidos;
        leidos = mi_read(ruta_fichero, buffer_texto, offset, tambuffer);
    }

    return 0;
}

static struct ruta pilaRutas[PROFUNDIDAD];// Para saber el recorrido que hace dentro de los directorios cp
int puntero_pila=-1; // Inicialmente vacía

/*
 * Copia un directorio y todos sus subdirectorios y ficheros a otro directorio destino
 * Param: ruta original del directorio (*ruta_origen) y ruta adónde se quiere mover (*ruta_destino)
 * Return:  0 si todo ha ido bien, -1 si error
 */
int mi_cp(char *ruta_origen, char *ruta_destino){
    struct inodo inodo, inodoentrada;
    int numentradas;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    struct entrada entrada;
    char rorigen[tamnombre*PROFUNDIDAD], rdestino[tamnombre*PROFUNDIDAD];

    if(puntero_pila==-1){ // Si la pila esta vacía, es la primera iteración
        strcpy(rorigen, ruta_origen);
        strcpy(rdestino, ruta_destino);
        strcat(rdestino, rorigen+1);

        // Empilamos el directorio principal a copiar
        strcpy(pilaRutas[puntero_pila+1].ruta_origen,rorigen);
        strcpy(pilaRutas[puntero_pila+1].ruta_destino,rdestino);
        puntero_pila++;     

    } else { // Si no, obtenemos la ruta actual
        strcpy(rorigen, pilaRutas[puntero_pila].ruta_origen);
        strcpy(rdestino, pilaRutas[puntero_pila].ruta_destino);
    }

    //buffer= strrchr(ruta_origen, '/') + 1;
    strcat(rdestino,strrchr(rorigen, '/') + 1);
    //fprintf(stderr, "ruta origen: %s\n", rorigen);
    //fprintf(stderr, "ruta destino: %s\n", rdestino);

    p_inodo_dir= 0;
    if ((error = buscar_entrada(rorigen, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error, "(no existe la entrada origen)");
        puntero_pila=-1;
        return -1;
    }

    leer_inodo(p_inodo,&inodo); // inodo a copiar

    mi_creat(rdestino, inodo.permisos); // Creamos un nuevo directorio

    if(inodo.tamEnBytesLog>0){ // Si el directorio está lleno
        numentradas=inodo.tamEnBytesLog/tamentrada;
        for (int i = 0; i < numentradas; i++){ // Mientras haya entradas
            mi_read_f(p_inodo, &entrada, i * tamentrada, tamentrada);

            // Miro si la entrada es un fichero o directorio
            leer_inodo(entrada.ninodo,&inodoentrada);

            // Top de la pila
            strcpy(rorigen, pilaRutas[puntero_pila].ruta_origen);
            strcpy(rdestino, pilaRutas[puntero_pila].ruta_destino);

            strcat(rorigen,entrada.nombre);

            if(inodoentrada.tipo=='d'){ // Si es un directorio
                strcat(rdestino,entrada.nombre);
                strcat(rorigen,"/");
                strcat(rdestino, "/");

                // Empilamos la rutas actuales
                strcpy(pilaRutas[puntero_pila+1].ruta_origen,rorigen);
                strcpy(pilaRutas[puntero_pila+1].ruta_destino,rdestino);
                puntero_pila++;

                mi_cp(rorigen,rdestino); // Copiamos el resto de entradas que tiene
                
            } else { // Si es un fichero
                // Empilamos la rutas actuales
                strcpy(pilaRutas[puntero_pila+1].ruta_origen,rorigen);
                strcpy(pilaRutas[puntero_pila+1].ruta_destino,rdestino);
                puntero_pila++;
                
                mi_cp_f(rorigen,rdestino);      
                puntero_pila--;
            }                
        }

        // Desempilamos   
        puntero_pila--;
        return 0;
                
    } else { // Si está vacío

        // Desempilamos
        puntero_pila--;
        return 0;
    }
}

int mi_rm_r(const char *camino){
    struct inodo inodo, inodoentrada;
    int numentradas;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    struct entrada entrada;
    char ruta[tamnombre*PROFUNDIDAD];

    if(puntero_pila==-1){ // Si la pila esta vacía, es la primera iteración
        strcpy(ruta, camino);

        // Empilamos el directorio principal a copiar
        strcpy(pilaRutas[puntero_pila+1].ruta_origen,ruta);
        puntero_pila++;     

    } else { // Si no, obtenemos la ruta actual
        strcpy(ruta, pilaRutas[puntero_pila].ruta_origen);

    }


    p_inodo_dir= 0;
    if ((error = buscar_entrada(ruta, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) != EXIT_SUCCESS){
        mostrar_error_buscar_entrada(error, "(no existe la entrada origen)");
        puntero_pila=-1;
        return -1;
    }

    leer_inodo(p_inodo,&inodo); // Inodo principal

    if(inodo.tamEnBytesLog>0){ // Si el directorio está lleno
        numentradas=inodo.tamEnBytesLog/tamentrada;
        for (int i = numentradas; i > 0; i--){ // Mientras haya entradas
            mi_read_f(p_inodo, &entrada, (i-1) * tamentrada, tamentrada);

            // Top de la pila
            strcpy(ruta, pilaRutas[puntero_pila].ruta_origen);
            strcat(ruta,entrada.nombre);

            // Miro si la entrada es un fichero o directorio
            leer_inodo(entrada.ninodo,&inodoentrada);

            if(inodoentrada.tipo=='d'){
                strcat(ruta,"/");

                // Empilamos la rutas actuales
                strcpy(pilaRutas[puntero_pila+1].ruta_origen,ruta);
                puntero_pila++;
                
                mi_rm_r(ruta);

            } else {
                mi_unlink(ruta);
            }     

            leer_inodo(p_inodo,&inodo); // Inodo principal
            numentradas=inodo.tamEnBytesLog/tamentrada;
        }

        mi_unlink(pilaRutas[puntero_pila].ruta_origen);
        puntero_pila--; 
        return 0;
                
    } else { // Si está vacío
        
   
        // Desempilamos
        mi_unlink(ruta);
        puntero_pila--;
        return 0;
    }
}


/*
 * Función auxiliar para mostrar error por pantalla para buscar_entrada()
 * Comments es para añadir comentarios al mostrar el error
 */
void mostrar_error_buscar_entrada(int error, char comments[]) {
    // fprintf(stderr, "Error: %d\n", error);
    switch (error) {
        case -1: fprintf(stderr, BOLD RED "Error: Camino incorrecto" RESET); break;
        case -2: fprintf(stderr, BOLD RED "Error: Permiso denegado de lectura" RESET); break;
        case -3: fprintf(stderr, BOLD RED "Error: No existe el archivo o el directorio" RESET); break;
        case -4: fprintf(stderr, BOLD RED "Error: No existe algún directorio intermedio" RESET); break;
        case -5: fprintf(stderr, BOLD RED "Error: Permiso denegado de escritura" RESET); break;
        case -6: fprintf(stderr, BOLD RED "Error: El archivo ya existe" RESET); break;
        case -7: fprintf(stderr, BOLD RED "Error: No es un directorio" RESET); break;
    }

    if(comments!=NULL) fprintf(stderr, " → %s\n",comments);
    else fprintf(stderr, "\n");  
}