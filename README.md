# Descripción
Simulación de un sistema de ficheros basado en EXT2 para la asignatura Sistemas Operativos II de la UIB (curso académico 20/21).

# Mejoras
Algunas de las mejoras añadidas respecto a la propuesta inicial son:
- liberar_bloques_inodo() salta bloques lógicos  inecesarios
- Caché de directorios implementada con un array siguiendo  criterio FIFO
- Buffer entradas en buscar_entrada() y en mi_dir()
- mi_touch
- mejoras mi_ls --> detallado o no (-d) y fichero (con colores según el tipo)
- Caché de directorios de más de una entrada
- mi_rmdir directorios vacíos
- mi_rm_r directorios no vacíos
- mi_rn
- mi_cp_f sólo para ficheros
- mi_cp para directorios, esten llenos o vacíos
- mi_mv
- Granularidad de las secciones críticas

# Autor 
Juan Carlos Bujosa

# Licencia y uso
Teniendo en cuenta que el proyecto corresponde a una práctica final evaluable correspondiente a la asignatura Sistemas Operativos II de la UIB, el autor no autoriza la copia, ya sea total o parcial del código y no se responsabiliza del uso que se le de a la información aquí alojada.
