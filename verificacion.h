#include "simulacion.h"

struct INFORMACION {
   int pid;
   unsigned int nEscrituras; // validadas
   struct REGISTRO PrimeraEscritura;
   struct REGISTRO UltimaEscritura;
   struct REGISTRO MenorPosicion;
   struct REGISTRO MayorPosicion;
};

void escribir_info(struct INFORMACION info, char *buffer);