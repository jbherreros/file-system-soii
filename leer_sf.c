#include "directorios.h"

void mostrar_buscar_entrada(char *camino, char reservar){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
        mostrar_error_buscar_entrada(error, NULL);
    }
    printf("**********************************************************************\n");
    
}

int main(int argc, char **argv){
    char nombre_dispositivo[1024];
    strcpy(nombre_dispositivo,argv[1]);
    struct superbloque SB;
 /*   struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
 
    struct inodo inodo;
    int ninodo;*/

    bmount(nombre_dispositivo);

    if(bread(posSB,&SB)<1) return -1;

    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB: %i\n",SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB: %i\n",SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI: %i\n",SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI: %i\n",SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos: %i\n",SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos: %i\n",SB.posUltimoBloqueDatos);
    printf("posInodoRaiz: %i\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre: %i\n", SB.posPrimerInodoLibre);
    printf("cantidadBloquesLibres: %i\n", SB.cantBloquesLibres);
    printf("cantInodosLibres: %i\n", SB.cantInodosLibres);
    printf("totBloques: %i\n", SB.totBloques);
    printf("totInodos: %i\n", SB.totInodos);


    /*//Mostrar creación directorios y errores
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
    //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2*/
    
    /*int ninodo=reservar_inodo('f',6);
    printf("\n\nTraducción de los bloques lógicos. Inodo %i\n", ninodo);
    traducir_bloque_inodo(ninodo, 8, 1);
    traducir_bloque_inodo(ninodo, 204, 1);
    traducir_bloque_inodo(ninodo, 30004, 1);
    traducir_bloque_inodo(ninodo, 400004, 1);
    traducir_bloque_inodo(ninodo, 468750, 1);

    printf("\nDATOS DEL INODO RESERVADO %i\n", ninodo);
    struct inodo inodo;
    leer_inodo(ninodo,&inodo);

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ATIME: %s\nMTIME: %s\nCTIME: %s\n",atime,mtime,ctime);
    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamEnBytesLog: %i\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %i\n", inodo.numBloquesOcupados);

    if(bread(posSB,&SB)<1) return -1;
    printf("\nSB.posPrimerInodoLibre: %i\n", SB.posPrimerInodoLibre);
*/
    
    
    /*
    printf ("sizeof struct superbloque is: %lu\n", sizeof(struct superbloque));
    printf ("sizeof struct inodo is: %lu\n", sizeof(struct inodo));
    
    printf("RECORRIDO LISTA ENLAZADA INODOS LIBRES");
	struct inodo inodos[BLOCKSIZE/INODOSIZE];
	int contInodos=0;

	for(int i = SB.posPrimerBloqueAI ; i <= SB.posUltimoBloqueAI ; i++){ // Recorro todos los bloques del array de inodos
		bread(i,inodos);
		int j=0;
		while(j < (BLOCKSIZE/INODOSIZE) && contInodos < SB.totInodos){
			if(inodos[j].tipo == 'l'){
				printf("%i ",inodos[j].punterosDirectos[0]);
				contInodos++;
			}
			j++;
		}
	}
	printf("inodosLibres: %i\n",contInodos);
    printf("\n\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int bloquereservado=reservar_bloque();
    printf("Se ha reservado el bloque: %i\n", bloquereservado);
    if(bread(posSB,&SB)<1) return -1; // Leo otra vez el SB
    printf("SB.cantBloquesLibres= %i\n", SB.cantBloquesLibres);
    printf("Se ha liberado el bloque: %i\n", bloquereservado);
    liberar_bloque(bloquereservado);
    if(bread(posSB,&SB)<1) return -1; // Leo otra vez el SB
    printf("SB.cantBloquesLibres= %i\n", SB.cantBloquesLibres);

    printf("\n\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    int valorLB;
    valorLB=leer_bit(posSB);
    printf("%i\n", valorLB);
    valorLB=leer_bit(SB.posPrimerBloqueMB);
    printf("%i\n", valorLB);
    valorLB=leer_bit(SB.posUltimoBloqueMB);
    printf("%i\n", valorLB);
    valorLB=leer_bit(SB.posPrimerBloqueAI);
    printf("%i\n", valorLB);
    valorLB=leer_bit(SB.posUltimoBloqueAI);
    printf("%i\n", valorLB);
    valorLB=leer_bit(SB.posPrimerBloqueDatos);
    printf("%i\n", valorLB);
    valorLB=leer_bit(SB.posUltimoBloqueDatos);
    printf("%i\n", valorLB);

    struct inodo inodo;
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    printf("\n\nDATOS DEL INODO RAÍZ, POS: %i\n", SB.posInodoRaiz);
    leer_inodo(SB.posInodoRaiz, &inodo);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ATIME: %s\nMTIME: %s\nCTIME: %s\n",atime,mtime,ctime);
    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamEnBytesLog: %i\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %i\n", inodo.numBloquesOcupados);
    */

    bumount(nombre_dispositivo);
    
    return 0;
}

