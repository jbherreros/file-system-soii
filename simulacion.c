#include "simulacion.h"

int acabados = 0;
int main(int argc, char const *argv[]){
    char nombre_dispositivo[1024], buf[80], camino[1024];
    time_t now;
    struct tm ts;
    pid_t pid;
    struct REGISTRO registro;

    // Comprobamos los argumentos
    if (argc != 2){
        fprintf(stderr, "Sintaxis: ./simulacion <disco>\n");
        exit(EXIT_FAILURE);
    }

    strcpy(nombre_dispositivo, argv[1]);

    // Montamos el disco
    if(bmount(nombre_dispositivo)==-1){
        fprintf(stderr, "Error: bmount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    // Tiempo actual
    time(&now);
    // Formato de tiempo, "aaaammddhhmmss"
    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &ts);
    sprintf(camino, "/simul_%s/", buf);

    if (mi_creat(camino, 6) == -1){
        fprintf(stderr, "Error: mi_creat()\n");
        bumount();
    }

    signal(SIGCHLD, reaper);
    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS ***\n", NUMPROCESOS, NUMESCRITURAS);
    //printf("Directorio simulación: %s\n", camino);

    for (int i = 1; i <= NUMPROCESOS; i++){
        pid = fork();
        if (pid == 0){ // proceso hijo
            char camino_fic[3072];
            char buffer_directorio[2048];
            int pid_directorio = getpid();
            bmount(nombre_dispositivo);
            memset(buffer_directorio, 0, strlen(buffer_directorio));
            sprintf(buffer_directorio, "%sproceso_%i/", camino, pid_directorio);

            if (mi_creat(buffer_directorio, 6) == -1){
                bumount();
                fprintf(stderr, "Error: mi_creat(%s)\n", buffer_directorio);
            }
            sprintf(camino_fic, "%sprueba.dat", buffer_directorio);

            if (mi_creat(camino_fic, 6) == -1){
                bumount();
                fprintf(stderr, "Error: mi_creat(%s)\n", camino_fic);
            }

            srand(time(NULL) + pid_directorio);
            for (int j = 0; j < NUMESCRITURAS; j++){
                registro.fecha = time(NULL);
                registro.pid = pid_directorio;
                registro.nEscritura = j + 1;
                registro.nRegistro = rand() % REGMAX;

                if (mi_write(camino_fic, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO)) == -1){
                    fprintf(stderr, "Error: mi_write en el fichero %s\n", camino_fic);
                }

                // fprintf(stderr, "[simulación.c → Escritura %d en %s]\n", j, camino_fic);
                usleep(50000); //0,05 segundos
            }

            printf("[Proceso %d: Completadas %d escrituras en %s]\n", i, NUMESCRITURAS, camino_fic);
            bumount();
            exit(EXIT_SUCCESS);
        }

        usleep(200000); //0,2 segundos
    }

    while (acabados < NUMPROCESOS){
        pause();
    }

    // Desmontamos el disco
    if(bumount()==-1){
        fprintf(stderr, "Error: bumount(%s)\n", nombre_dispositivo);
        exit(EXIT_FAILURE);
    }

    printf("Total de procesos terminados: %d.\n", acabados);
    exit(EXIT_SUCCESS);
}


/*
 * Entierra los procesos hijo 
 * Return: void
 */
void reaper(){
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0){
        acabados++;
        //Podemos testear qué procesos van acabando:
        //fprintf(stderr, "[simulación.c → Acabado proceso con PID %d, total acabados: %d]\n", ended, acabados);
    }
}