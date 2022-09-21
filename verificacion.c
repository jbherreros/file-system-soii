#include "verificacion.h"

int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024], dir_simul[1024], camino_fic[2048], camino_prueba[3072], buffer_info[3000], *ret;
    struct entrada buffer_entradas[NUMPROCESOS];
    struct STAT stat;
    struct INFORMACION info;
    int numentradas, offset;
    int cant_reg_buffer = 256;
    struct REGISTRO buffer_escrituras[cant_reg_buffer];

    // Comprobamos los argumentos
    if (argc != 3){
        fprintf(stderr, "Sintaxis:./verificacion <disco> </directorio_simulacion/>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo, argv[1]);

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    strcpy(dir_simul, argv[2]);
    printf("dir_sim:%s\n", dir_simul);
    mi_stat(dir_simul, &stat);

    if (dir_simul[strlen(dir_simul) - 1] != '/'){
        fprintf(stderr, "Error: No es un directorio\n");
        exit(EXIT_FAILURE);
    }

    numentradas = stat.tamEnBytesLog / tamentrada;
    if (numentradas != NUMPROCESOS) return -1;

    sprintf(camino_fic, "%sinforme.txt", dir_simul);
    mi_creat(camino_fic, 6);
    mi_read(dir_simul, buffer_entradas, 0, NUMPROCESOS * sizeof(struct entrada)); // Lee todas las entradas
    
    for (int i = 0; i < NUMPROCESOS; i++){
        info.nEscrituras = 0;
        ret = strchr(buffer_entradas[i].nombre, '_');
        if (!ret){ //ret = null
            fprintf(stderr, "Error: Formato de nombre de directorio incorrecto\n");
            exit(EXIT_FAILURE);
        }

        memset(&info, 0, sizeof(struct INFORMACION));
        info.pid = atoi(ret + 1);

        //Recorrido secuencial del fichero prueba.dat
        offset = 0;
        memset(camino_prueba, 0, sizeof(camino_prueba));
        sprintf(camino_prueba, "%s%s/prueba.dat", dir_simul, buffer_entradas[i].nombre);
        memset(buffer_escrituras, 0, NUMPROCESOS * sizeof(struct REGISTRO));
        while (mi_read(camino_prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0 && info.nEscrituras < NUMESCRITURAS){
            for (int j = 0; j < cant_reg_buffer; j++){
                if (buffer_escrituras[j].pid == info.pid){ //la escritura es válida
                    if (info.nEscrituras == 0){ ///es la primera escritura validada
                        info.PrimeraEscritura = buffer_escrituras[j];
                        info.UltimaEscritura = buffer_escrituras[j];
                        info.MenorPosicion = buffer_escrituras[j];
                        info.MayorPosicion = buffer_escrituras[j];
                    
                    } else {
                        if (difftime(buffer_escrituras[j].fecha, info.PrimeraEscritura.fecha) < 0){ //es anterior
                            info.PrimeraEscritura = buffer_escrituras[j];

                        } else if (buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura){
                            info.PrimeraEscritura = buffer_escrituras[j];

                        }

                        if (difftime(buffer_escrituras[j].fecha, info.UltimaEscritura.fecha) > 0){ //es posterior
                            info.UltimaEscritura = buffer_escrituras[j];

                        } else if (buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura){
                            info.UltimaEscritura = buffer_escrituras[j];

                        }
                    }

                    info.nEscrituras++;
                }
            }

            offset += sizeof(buffer_escrituras);
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras)); //vaciar el buffer
        }

        //Escritura de la última posición
        mi_stat(camino_prueba, &stat);
        mi_read(camino_prueba, &info.MayorPosicion, stat.tamEnBytesLog - sizeof(struct REGISTRO), sizeof(struct REGISTRO));
        //info al final de informe.txt
        mi_stat(camino_fic, &stat);
        escribir_info(info, buffer_info);
        mi_write(camino_fic, buffer_info, stat.tamEnBytesLog, strlen(buffer_info));
        printf("[%d) %d escrituras validadas en %s]\n", i + 1, info.nEscrituras, camino_prueba);
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }
}


/*
 * Escribe la información en un buffer
 * Param: información (struct INFORMACION) y buffer (*buffer)
 * Return: void
 */
void escribir_info(struct INFORMACION info, char *buffer){
    char buffer_secundario[250];
    sprintf(buffer, "PID:%d\n", info.pid);
    sprintf(buffer_secundario, "Numero de escrituras:\t%d\n",  info.nEscrituras);
    strcat(buffer, buffer_secundario);
    memset(buffer_secundario, 0, strlen(buffer_secundario));
    sprintf(buffer_secundario, "Primera Escritura\t%d\t%d\t%s", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, asctime(localtime(&info.PrimeraEscritura.fecha)));
    strcat(buffer, buffer_secundario);
    memset(buffer_secundario, 0, strlen(buffer_secundario));
    sprintf(buffer_secundario, "Ultima Escritura\t%d\t%d\t%s", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, asctime(localtime(&info.UltimaEscritura.fecha)));
    strcat(buffer, buffer_secundario);
    memset(buffer_secundario, 0, strlen(buffer_secundario));
    sprintf(buffer_secundario, "Menor Posición\t\t%d\t%d\t%s", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, asctime(localtime(&info.MenorPosicion.fecha)));
    strcat(buffer, buffer_secundario);
    memset(buffer_secundario, 0, strlen(buffer_secundario));
    sprintf(buffer_secundario, "Mayor Posición\t\t%d\t%d\t%s\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, asctime(localtime(&info.MayorPosicion.fecha)));
    strcat(buffer, buffer_secundario);

}