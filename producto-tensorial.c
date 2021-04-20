/*
Autores: Hernán Díaz y Gina Ozimisa
Objetivo: Calcular el producto tensorial de a y b en procesos distribuidos
fecha: 15/01/2020
*/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0               //id del primer proceso
#define TAG1 1          // mensaje de MASTER
#define TAG2 2          //mensaje del proceso


int main (int argc, char *argv[])
{
    int	allwe,              //numero de procesos iniciados
        whoami,               // identificador de proceso
        numproc,           //numero de procesos
        source,               //id del proceso en origen
        dest,                 //id del proceso en destino
        mtype,                // mensaje
        filas,                 //cantidad de filas de a que seran calculadas con b
        canfilas, resto, filaC, //usados para calcular los saltos de filas que hara c
        i, j, l, h, rc, filaA, m, k, n;
    FILE *fichero;
    char caracter;
    if ((fichero = fopen(argv[1],"r")) == NULL)
    {
        printf("Error!");
        exit(1);
    }
//extraer m,k,n del fichero data.txt
    while(caracter!='\n')
    {
        fscanf (fichero, "%d", &m);
        fscanf (fichero, "%d", &k);
        fscanf (fichero, "%d", &n);
        caracter=fgetc(fichero);
    }

    int	a[m][k],           //matriz a
        b[k][n],           //matriz b
        c[m*k][k*n];          //resultado c
    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&whoami);
    MPI_Comm_size(MPI_COMM_WORLD,&allwe);
    if (allwe < 2 )
    {
        printf("Se necesitan al menos 2 tareas\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }

    numproc = allwe-1;



    if (whoami == MASTER)//proceso 0
    {
        //llenar matrices con los datos del archivo data.txt
        for(i=0; i<m; i++)
        {
            for(j=0; j<k; j++)
                fscanf (fichero, "%d ", &a[i][j]);
        }

        for(i=0; i<k; i++)
        {
            for(j=0; j<n; j++)
                fscanf (fichero, "%d ", &b[i][j]);
        }
        fclose(fichero);
        //Tiempo de inicio
        double start = MPI_Wtime();

        // Enviar matriz a calculo
        canfilas = m/numproc; //dividir el calculo segun la cantidad de procesos
        resto = m%numproc;
        filaC = 0; //posicion de filas de C
        filaA = 0; //posicion de filas de A
        mtype = TAG1;
        for (dest=1; dest<=numproc; dest++)
        {
            if (dest<=resto)
                filas=canfilas+1;
            else
                filas=canfilas;
            if(filas>0)
              printf("Enviando la matriz B y %d filas de A al proceso %d,\nrepresentara desde la fila=%d de C\n\n",filas,dest,filaC);
            else
              printf("No quedan mas filas de A para el proceso %d\n",dest );
            MPI_Send(&filaC, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(&filas, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(&a[filaA][0], filas*k, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(&b, k*n, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            filaC = filaC + filas*k;
            filaA=filaA+filas;
        }
        // Resultados del calculo
        mtype = TAG2;
        for (i=1; i<=numproc; i++)
        {
            source = i;
            MPI_Recv(&filaC, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(&filas, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(&c[filaC][0], filas*k*k*n, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
        }

        //imprimir matrices
        printf("Matriz A:\n");
        for (i=0; i<m; i++)
        {
            printf("\n");
            for (j=0; j<k; j++)
                printf("%6d   ", a[i][j]);
        }
        printf("\n\n");
        printf("Matriz B:\n");
        for (i=0; i<k; i++)
        {
            printf("\n");
            for (j=0; j<n; j++)
                printf("%6d   ", b[i][j]);
        }
        printf("\n\n");
        printf("Resultado del producto tensorial, matriz C:\n");
        for (i=0; i<m*k; i++)
        {
            printf("\n");
            for (j=0; j<k*n; j++)
                printf("%6d   ", c[i][j]);
        }
        printf("\n\n");


        // Medir tiempo final
        double finish = MPI_Wtime();
        printf("Calculado en %f segundos\n", finish - start);
    }



    if (whoami > MASTER)//calculo de cada proceso
    {
        int inicioFila =0, inicioColumna=0;

        mtype = TAG1;
        MPI_Recv(&filaC, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&filas, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&a, filas*k, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&b, k*n, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
        for(i=0; i<filas; i++)
        {
            for(j=0; j<k; j++)
            {
                inicioFila = i*k;
                inicioColumna = j*n;
                for(h=0; h<k; h++)
                {
                    for(l=0; l<n; l++)
                        c[inicioFila+h][inicioColumna+l] = a[i][j]*b[h][l];
                }
            }
        }
        mtype = TAG2;
        MPI_Send(&filaC, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
        MPI_Send(&filas, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
        MPI_Send(&c, filas*k*k*n, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}
