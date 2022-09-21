#include "ficheros_basico.h"

/*
 * Calcula el tamaño en bloques del mapa de bits.
 * Param: número de bloques del SF
 * Return: el número de bloques que ocupa el mapa de bits (MB)
 */
int tamMB(unsigned int nbloques){
    int nbytes_MB=nbloques/8;   // El MB consta de tantos bits como bloques hay, se calcula el 
                                // número de bytes que corresponde --> 100000 bloques --> 100000 bits/8
                                // = 12500 bytes

    // Comprobamos cuantos bloques ocuparan esos 12500 bytes
    if(nbytes_MB % BLOCKSIZE){ 
        return nbytes_MB/BLOCKSIZE+1; // Si hay resto, se le suma uno --> 12500/1024 = 12.2 bloques --> 13
    } else {
        return nbytes_MB/BLOCKSIZE;
    }
}                   

/*
 * Calcula el tamaño en bloques del array de inodos
 * Param: número total de inodos del SF
 * Return: el número de bloques que ocupa el array de inodos (AI)
 */
int tamAI(unsigned int ninodos){
    int nbytes_AI=ninodos*INODOSIZE; // Número de bytes del array de inodos: numero de inodos * tamaño de un inodo

    // Comprobamos cuandos bloques ocuparan nuestros inodos
    if(nbytes_AI % BLOCKSIZE){
        return nbytes_AI/BLOCKSIZE+1; // Si hay resto se le suma uno, igual que en tamMB()
    } else {
        return nbytes_AI/BLOCKSIZE;
    }
}

/*
 * Inicializa los datos del superbloque
 * Param: número de bloques i número de inodos totales del SF
 * Return: 0 si todo ha ido bien, -1 si error
 */
int initSB(unsigned int nbloques, unsigned int ninodos){
    struct superbloque SB;
   
    SB.posPrimerBloqueMB = posSB + tamSB; //posSB = 0, tamSB = 1
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques-1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    if (bwrite(posSB, &SB)==-1) return -1;

    return 0;
}

/*
 * Inicializa el mapa de bits
 * Return: 0 si todo ha ido bien, -1 si error
 */
int initMB(){
    struct superbloque SB;
    unsigned char buffer[BLOCKSIZE];
    memset(buffer,0,BLOCKSIZE); // Inicializo el buffer a 0

    if(bread(posSB,&SB)==-1) return -1; // Leo el superbloque del sistema de ficheros

    // Recorro todos los bloques del mapa de bits y los inicializo a 0 (buffer a 0)
    for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++){
        if(bwrite(i,buffer)<0) return -1;
    }

    // Inicializo los bits del mapa de bits según si los bloques de los metadatos estan ocupados o no (SB, MB, AI)
    for (int i = posSB; i <= SB.posUltimoBloqueAI; i++){ 
		if(escribir_bit(i,1)<0) return -1; 
	}
    SB.cantBloquesLibres -= (SB.posUltimoBloqueAI + 1);
    if(bwrite(posSB, &SB)<0) return -1;
    return 0;
}

/*
 * Inicializa el array de inodos libres
 * Param: número de bloques i número de inodos totales del SF
 * Return: 0 si todo ha ido bien, -1 si error
 */
int initAI(){
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE]; // Inodos que caben en un bloque
    
    if(bread(posSB,&SB)<1) return -1;

    int contInodos=SB.posPrimerInodoLibre+1;
    for (int i= SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){ // Recorro todos los bloques del AI
        for (int j = 0; j < BLOCKSIZE/INODOSIZE; j++){ // Recorro todos los inodos del bloque
            inodos[j].tipo='l';
            if(contInodos < SB.totInodos){ // Si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0]=contInodos; // El inodo apunta al siguiente inodo libre
                contInodos++;
            } else { // Hemos llegado al último inodo
                inodos[j].punterosDirectos[0]=UINT_MAX;
                break; // Salimos del bucle (último bloque no tiene porque estar completo de inodos)
            }
        }
        
        if(bwrite(i,inodos)<1) return -1;
        
    }
    
    return 0;
}

/*
 * Escribe el valor del parámetro bit (0 si libre, 1 si ocupado) en un bit del MB
 * que representa el bloque nbloque
 * Param: bit (indica 0 o 1) i nbloque (bloque a escribir en el MB)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int escribir_bit(unsigned int nbloque, unsigned int bit){
    struct superbloque SB;
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    unsigned char mascara = 128; // 10000000
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(posSB, &SB)<0) return -1; 

    // Extraigo la posición del bit dentro del MB que representa nbloque y su bloque físico
    posbyte = nbloque / 8; // Contiene el byte de MB que representa el bloque nbloque
    posbit = nbloque % 8; // Contiene la posición del bit del posbyte que representa nbloque
    nbloqueMB= posbyte / BLOCKSIZE; // Bloque de MB donde se encuentra el bit
    nbloqueabs= SB.posPrimerBloqueMB + nbloqueMB; // Bloque del dispositivo virtual en que se encuentra el bit

    // Extraigo el bloque físico que contiene el bit del MB
    if (bread(nbloqueabs, bufferMB)<0) return -1;

    // Esribo el bit a 0 o 1 (param: bit)
    posbyte = posbyte % BLOCKSIZE; // Contendrá la posición del byte dentro del rango del bloque [0,1024]
    mascara >>= posbit; // desplazamiento de bits a la derecha
    if(bit==1){
        bufferMB[posbyte] |= mascara; //  Se pone el bit a 1 (operador OR para bits)
    } else if (bit==0){
        bufferMB[posbyte] &= ~mascara;  // Se pone el bit a 0 (operadores AND y NOT para bits)
    } else {
        return -1;
    }

    if (bwrite(nbloqueabs, bufferMB)<0) return -1;

    return 0;
}

/*
 * Lee un determinado bit del MB
 * Param: nbloque (bloque a leer del MB)
 * Return: El bit leído (0 si libre, 1 si ocupado)
 */
char leer_bit(unsigned int nbloque){ // Hace lo mismo que escribir_bit a nivel lógico, pero lee el bit en lugar de escribir
    struct superbloque SB;
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    unsigned char mascara = 128;    // 10000000
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(posSB, &SB)<0) return -1; 

    // Extraigo la posición del bit dentro del MB que representa nbloque y su bloque físico
    posbyte = nbloque / 8; // Contiene el byte de MB que representa el bloque nbloque
    posbit = nbloque % 8; // Contiene la posición del bit del posbyte que representa nbloque
    nbloqueMB= posbyte / BLOCKSIZE; // Bloque de MB donde se encuentra el bit
    nbloqueabs= SB.posPrimerBloqueMB + nbloqueMB; // Bloque del dispositivo virtual en que se encuentra el bit
    if (bread(nbloqueabs, bufferMB)<0) return -1;
    printf("leer bit(%i) [posbyte: %i, posbit: %i, nbloqueMB: %i, nbloqueabs: %i]= ",nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
    // Leo el bit
    posbyte = posbyte % BLOCKSIZE;
    mascara >>= posbit;             // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte];   // operador AND para bits
    mascara >>= (7 - posbit);       // desplazamiento de bits a la derecha

    // Retorna el bit leído
    return mascara;
}

/*
 * Encuentra el primer bloque libre, lo ocupa y devuelve su posición
 * Return: Devuelve la posición del primer bloque libre (ahora ocupado)
 */
int reservar_bloque(){
    struct superbloque SB;
    int posBloqueMB, nbloque;
    int posbit=0, posbyte=0;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];

    if (bread(posSB, &SB)<0) return -1; 

    // Miro primero de todo si quedan bloques libres 
    if(SB.cantBloquesLibres<=0){
        fprintf(stderr, BOLD RED "Error: No quedan bloques libres\n" RESET); 
    }

    // Cargo bloque a bloque todos los bloques del MB y miro cuál es el primer bit libre, cual es el primer 0
    posBloqueMB=SB.posPrimerBloqueMB;
    if (bread(posBloqueMB,bufferMB)<0) return -2;
    memset(bufferAux, 255, BLOCKSIZE); // Buffer auxiliar para comparar, inicializado a 1 (todo el bloque contiene 1s)
    while(posBloqueMB<=SB.posUltimoBloqueMB && memcmp(bufferMB, bufferAux, BLOCKSIZE)==0){
        posBloqueMB++;
        if (bread(posBloqueMB,bufferMB)<0) return -3;
    }

    if(memcmp(bufferMB, bufferAux, BLOCKSIZE)==0) return -4;

    // Ahora en posBloqueMB tenemos el bloque que tiene un bit a 0 (que representa un bloque libre)
    // Buscamos qué byte del bloque contiene ese primer 0
    unsigned char bufferByte=255;
    while (memcmp(&bufferMB[posbyte], &bufferByte, 1) == 0 && (posbyte<BLOCKSIZE)) { // 255(decimal): 11111111(binario)
        posbyte++;
    }

    // Buscamos el primer bit a 0 en ese byte. Se guardará su posición relativa en posbit
    unsigned char mascara = 128; // 128(decimal): 10000000(binario)
    while (bufferMB[posbyte] & mascara) { // operador AND para bits
        bufferMB[posbyte] <<= 1;          // desplazamiento de bits a la izquierda
        posbit++;
    }

    // Calculamos a que bloque corresponde ese bit, que indica el primer bloque libre disponible
    nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

    escribir_bit(nbloque,1); // Reservamos el bloque (lo ponemos a 1)
    SB.cantBloquesLibres--; // Decrementamos el número de bloques libres
    if(bwrite(posSB, &SB)<0) return -5; // Guardamos los cambios en el superbloque
    memset(bufferAux,0,BLOCKSIZE); 
    if(bwrite(nbloque,bufferAux)<0) return -6; // Ponemos a 0 nbloque por si habia basura 

    return nbloque;
}

/*
 * Libera un bloque determinado
 * Param: nbloque (el bloque a liberar)
 * Return: devuelve el bloque liberado si todo ha ido bien, -1 si error
 */
int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;
    if(bread(posSB, &SB)<0) return -1;

    escribir_bit(nbloque, 0); // Ponemos a 0 el bit del MB correspondiente a nbloque
    SB.cantBloquesLibres++; // Incrementamos el número de bloques libres en el superbloque

    if(bwrite(posSB, &SB)<0) return -1;
    return nbloque;
}

/*
 * Escribe el contenido del inodo inodo en el ninodo del AI
 * Param: inodo (struct inodo) y ninodo (número de inodo del AI)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int escribir_inodo(unsigned int ninodo, struct inodo inodo){
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    struct superbloque SB;
    int nbloqueabs;
    
    if(bread(posSB, &SB)<0) return -1;

    // Obtengo el bloque físico que corresponde al ninodo
    nbloqueabs = (ninodo * INODOSIZE) / BLOCKSIZE + SB.posPrimerBloqueAI;

    // Escribo el inodo en el bloque correspondiente 
    if(bread(nbloqueabs, inodos)<0) return -1;
    inodos[ninodo%(BLOCKSIZE/INODOSIZE)]=inodo; // Escribo el inodo en el ninodo
    if(bwrite(nbloqueabs, inodos)<0) return -1;

    return 0;
}

/*
 * Lee el inodo apuntado por ninodo y lo vuelca en *inodo
 * Param:  ninodo (número de inodo a leer) y inodo (donde se guardará el inodo leído)
 * Return: 0 si todo ha ido bien, -1 si error
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int nbloqueabs;
    if(bread(posSB, &SB)<0) return -1;

    // Obtengo el bloque físico que corresponde al ninodo
    nbloqueabs = (ninodo * INODOSIZE) / BLOCKSIZE + SB.posPrimerBloqueAI;

    // Vuelco el inodo a leer en la variable apuntada por *inodo
    if(bread(nbloqueabs, inodos)<0) return -1;
    *inodo=inodos[ninodo%(BLOCKSIZE/INODOSIZE)];

    return 0;
}

/*
 * Encuentra el primer inodo libre, lo reserva y actualiza la lista enlazada de inodos libres
 * Param: tipo ('l':libre, 'd':directorio o 'f':fichero) y permisos (rwx en binario)
 * Return: devuelve el número del inodo reservado (ninodo)
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos){
    struct superbloque SB;
    struct inodo inodo;
    int posInodoReservado;
    if(bread(posSB, &SB)<0) return -1;

    // Comprovamos si aún hay inodos libres
    if(SB.cantInodosLibres<=0){
        fprintf(stderr, BOLD RED "Error: No quedan inodos libres\n" RESET); 
        return -1;
    }
    // Guardo la posición del inodo reservado
    posInodoReservado=SB.posPrimerInodoLibre; 
    // Obtengo el inodo
    if(leer_inodo(SB.posPrimerInodoLibre, &inodo)<0) return -1;
    // Actualizo el apuntador al primer inodo libre
    SB.posPrimerInodoLibre=inodo.punterosDirectos[0];
     
    // Inicializamos el inodo
    inodo.tipo=tipo;
    inodo.permisos=permisos;
    inodo.atime=time(NULL); // time(NULL) pone el tiempo a fecha actual del sistema
    inodo.mtime=time(NULL);
    inodo.ctime=time(NULL);
    inodo.nlinks=1;
    inodo.tamEnBytesLog=0;
    inodo.numBloquesOcupados=0;
    memset(inodo.punterosDirectos, 0, 12 * sizeof(unsigned int)); // Punteros directos a 0
    memset(inodo.punterosIndirectos, 0, 3 * sizeof(unsigned int)); // Punteros indirectos a 0

    // Escribo el inodo y salvo la info del SB
    if(escribir_inodo(posInodoReservado, inodo)<0) return -1;
    SB.cantInodosLibres--;
    if(bwrite(posSB, &SB)<0) return -1;

    return posInodoReservado;
}

/*
 * Asocia un nivel de punteros (Directos y Indirectos0/1/2) al bloque lógico (BL) indicado
 * por parámetro
 * Param: inodo (inodo al que pertenece el BL), nblogico (número BL) y ptr (puntero que devolverá
 * la dirección a la que apunte el puntero del inodo)
 * Return: Devuelve un puntero al nivel de punteros asociado al bloque lógico (*ptr)
 * y el nivel de puntero (0:D, 1:I0, 2:I1, 3:I2)
 */
int obtener_nrangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr){
    if (nblogico < DIRECTOS){
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    } else if (nblogico < INDIRECTOS0){
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    } else if (nblogico < INDIRECTOS1){
        *ptr = inodo->punterosIndirectos[1];
        return 2;
     } else if (nblogico < INDIRECTOS2){
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    } else{
        *ptr = 0;
        fprintf(stderr, BOLD RED "Error: Bloque lógico fuera de rango\n" RESET); 
        return -1;
    }
}

/*
 * Obtiene el índice de los bloques lógicos de punteros
 * Param: número de bloque lógico (nblogico) y el nivel de punteros al índice que queremos acceder
 * Return: Devuelve el índice del nivel de punteros y -1 si error
 */
int obtener_indice(unsigned int nblogico, unsigned int nivel_punteros){
    if (nblogico < DIRECTOS){
        return nblogico;

    } else if (nblogico < INDIRECTOS0){
        return nblogico - DIRECTOS;

    } else if (nblogico < INDIRECTOS1){
        if (nivel_punteros == 2){
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        } else if (nivel_punteros == 1){
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }

    } else if (nblogico < INDIRECTOS2){
        if (nivel_punteros == 3){
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);

        } else if (nivel_punteros == 2){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;

        } else if (nivel_punteros == 1){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;

        }
    }
    return -1;
}

/*
 * Obtiene el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
 * Param: Inodo a leer (ninodo), número de bloque lógico (nblogico) y (reservar, 0 sólo consulta, 1 consultar/reservar)
 * Return: Devuelve el número de bloque físico correspondiente al BL del inodo y -1 si error
 */
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
    struct inodo inodo;
    int salvar_inodo, nRangoBL, nivel_punteros, indice;
    int buffer[NPUNTEROS];
    unsigned int ptr, ptr_ant;
    if (leer_inodo(ninodo, &inodo) == -1){
        return -1;
    }

    ptr = 0, ptr_ant = 0, salvar_inodo = 0;
    nRangoBL = obtener_nrangoBL(&inodo, nblogico, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    if (nRangoBL == -1) return -1;
    nivel_punteros = nRangoBL; //el nivel_punteros +alto es el que cuelga del inodo

    while (nivel_punteros > 0){ //iterar para cada nivel de indirectos
        if (ptr == 0){ //no cuelgan bloques de punteros
            if (reservar == 0) return -1;  //error lectura bloque inexistente
            salvar_inodo = 1;              //reservar bloques punteros y crear enlaces desde inodo hasta datos
            ptr = reservar_bloque();       //de punteros
            if (ptr == -1) return -1;      //error reservar_bloque --> ptr == -1 entonces return -1
            
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);        //fecha actual
            if (nivel_punteros == nRangoBL){ //el bloque cuelga directamente del inodo --> PUNTEROS INDIRECTOS 0
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;
                //printf("[traducir_bloque_inodo()→ punterosIndirectos[%i]= %i (reservado BF %i punteros_nivel%i)]\n", nRangoBL-1, ptr, ptr, nRangoBL);
            
            } else {//el bloque cuelga de otro bloque de punteros --> PUNTEROS NIVEL 2/3
                buffer[indice] = ptr; // IMPRIMIRLO
                //printf("[traducir_bloque_inodo()→ punteros_nivel%i[%i]= %i (reservado BF %i para punteros_nivel%i)]\n", nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
                if (bwrite(ptr_ant, buffer) == -1) return -1;
            }

        }

        if (bread(ptr, buffer) == -1) return -1;
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        //guardamos el puntero
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    } //al salir de este bucle ya estamos al nivel de datos
   
    if (ptr == 0){ //no existe bloque de datos
        if (reservar == 0) return -1;//error lectura ∄ bloque

        salvar_inodo = 1;
        ptr = reservar_bloque();   //de datos
        if (ptr == -1) return -1;  //error reservar_bloque
         
        inodo.numBloquesOcupados++;
        inodo.ctime = time(NULL);
        if (nRangoBL == 0){ // PUNTEROS DIRECTOS 
            inodo.punterosDirectos[nblogico] = ptr;
            //printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%i]= %i (reservado BF %i para el BL %i)]\n", nblogico, ptr, ptr, nblogico);

        } else { // PUNTEROS NIVEL1
            buffer[indice] = ptr;
            //printf("traducir_bloque_inodo()→ punteros_nivel1[%i]= %i (reservado BF %i para el BL %i)]\n", indice, ptr, ptr, nblogico);
            if (bwrite(ptr_ant, buffer) == -1) return -1;
            
        }
    }

    if (salvar_inodo == 1){
        if (escribir_inodo(ninodo, inodo) == -1) return -1; //sólo si lo hemos actualizado        
    }

    return ptr; //nbfisico
}

/*
 * Libera el espacio ocupado por un inodo
 * Param: número de inodo en el array de inodos a liberar (ninodo)
 * Return: número de inodo reservado, -1 si error
 */
int liberar_inodo(unsigned int ninodo){
    struct inodo inodo;
    struct superbloque SB;
    int numBloquesLiberados;

    if(leer_inodo(ninodo, &inodo)==-1) return -1;

    // Liberamos los bloques del inodo
    numBloquesLiberados=liberar_bloques_inodo(0, &inodo);
    if(numBloquesLiberados==-1) return -1;

    // Actualizamos metadatos
    inodo.numBloquesOcupados-= numBloquesLiberados;
    inodo.tipo='l';
    inodo.tamEnBytesLog=0;

    // Actualización lista inodos libres y datos SB
    if(bread(posSB, &SB)==-1) return -1;
    inodo.punterosDirectos[0]=SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre=ninodo;
    SB.cantInodosLibres++;

    if (escribir_inodo(ninodo, inodo)==-1) return -1;
    if (bwrite(posSB, &SB)==-1) return -1;

    return ninodo;

}

/*
 * Libera los bloques lógicos del inodo a partir del bloque lógico hasta el último bloque lógico
 * Param: primer bloque lógico a borrar (primerBL) y apuntador al inodo concreto (inodo)
 * Return: número de bloques liberados, -1 si error
 */
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo){
    unsigned int nRangoBL, nivel_punteros, indice, ptr, ultimoBL;
    unsigned char buffer_aux[BLOCKSIZE];
    int bloques_punteros[3][NPUNTEROS]; //array de bloques de punteros
    int ptr_nivel[3];                   //punteros a bloques de punteros de cada nivel
    int indices[3];                     //indices de cada nivel
    int liberados;                      //nº de bloques liberados

    liberados = 0;
    if (inodo->tamEnBytesLog == 0) return liberados; //fichero vacío

    // Obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0){
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    } else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    memset(buffer_aux, 0, BLOCKSIZE);
    ptr = 0;
    //fprintf(stderr, "primer BL: %u, último BL: %u\n", primerBL, ultimoBL);
    for (int nBL = primerBL; nBL <= ultimoBL; nBL++){
        nRangoBL = obtener_nrangoBL(inodo, nBL, &ptr);

        if (nRangoBL < 0) return -1;

        // -> MEJORA: Si uno de los punteros del inodo apunta a 0 salta al siguiente rango
        if (!ptr){
            if (nRangoBL == 1){
                nBL = INDIRECTOS0 - 1;
            } else if (nRangoBL == 2){
                nBL = INDIRECTOS1 - 1;
            } else if (nRangoBL == 3){
                break;
            }
        } // MEJORA <-

        nivel_punteros = nRangoBL; // El nivel_punteros más alto cuelga del inodo
        while (ptr > 0 && nivel_punteros > 0){ // Mientras cuelgan bloques de punteros
            indice = obtener_indice(nBL, nivel_punteros);

            if ((indice==0) || (nBL==primerBL)){
                bread(ptr, bloques_punteros[nivel_punteros - 1]);
            }

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0){ // Si existe bloque de datos
            liberar_bloque(ptr);
            //fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos correspondiente al BL %d]\n", ptr, nBL);
            liberados++;
            if (nRangoBL == 0){ // Es un puntero directo
                inodo->punterosDirectos[nBL] = 0;

            } else { // Si no es un puntero directo
                nivel_punteros=1;
                while (nivel_punteros <= nRangoBL){
                    indice = indices[nivel_punteros-1];
                    bloques_punteros[nivel_punteros-1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros-1];
                    if (memcmp(bloques_punteros[nivel_punteros-1], buffer_aux, BLOCKSIZE)==0){
                        //No cuelgan bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        //fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%u correspondiente al BL %d]\n",ptr, nivel_punteros, nBL);
                        liberados++;
                        
                        // INCLUIR MEJORA PARA SALTAR LOS BLOQUES QUE NO SEA NECESARIO EXPLORAR

                        if (nivel_punteros == nRangoBL){
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;

                    } else { //escribimos en el dispositivo el bloque de punteros modificado
                        bwrite(ptr, bloques_punteros[nivel_punteros-1]);
                        nivel_punteros = nRangoBL+1; // para salir del bucle
                    }
                }
            }
        
        } else if (ptr == 0){
            // -> MEJORA: Comprueba a qué nivel pertenece el puntero y se salta los BL que no hace falta comprobar
			if (nivel_punteros == 2){
                nBL+= NPUNTEROS - 1;
            } else if (nivel_punteros == 3){
                nBL += (NPUNTEROS * NPUNTEROS) - 1;
            } // MEJORA 2 <-
        }
    } 
        
    //fprintf(stderr, "[liberar_bloques_inodo()→ total bloques liberados: %d]\n\n", liberados);
    //fprintf(stderr, "liberados: %d\n", liberados);
    return liberados;
}