1)Abrir directorio de archivos 
2)Abrir terminal e ingresar los siguientes comandos:

*Modo de compilación:
    mpicc -o producto-tensorial.exe producto-tensorial.c

*Modo de ejecución:
     mpirun -np p ./producto-tensorial.exe data.txt
     - Donde "p" es el numero de procesos con el que se realizara la ejecución del programa.
     -data.txt es el fichero con los datos.
