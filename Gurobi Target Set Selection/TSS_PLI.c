#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "/opt/gurobi951/linux64/include/gurobi_c.h"
#define NUM_VERTICES 6

typedef struct VERTEX {
    int index;
    int num_neigh_in;
    struct VERTEX *neigh_in;
    double threshold;
} VERTEX;

void main() {
    int i, j, m, sum;
    GRBenv   *env   = NULL;
    GRBmodel *model = NULL;
    int       error = 0;
    int      *ind = (int *) malloc(((NUM_VERTICES*NUM_VERTICES)+NUM_VERTICES)*sizeof(VERTEX));
    double   *val = (double *) malloc(((NUM_VERTICES*NUM_VERTICES)+NUM_VERTICES)*sizeof(VERTEX));
    char      name[100];

    VERTEX *G = (VERTEX *) malloc(NUM_VERTICES*sizeof(VERTEX));
    for (i=0;i<NUM_VERTICES;i++) {
        G[i].neigh_in = (VERTEX *) malloc(NUM_VERTICES*sizeof(VERTEX));
    }

    /* le grafo (
        num vertices
        vertice | threshold |vizinhos
    ) */
    FILE* file;
    file = fopen("grafo.txt", "r");
    if (!file) {
        perror ("Erro ao abrir arquivo") ;
        exit(1);
    }
    int num_v;
    fscanf(file, "%d", &num_v);
    for (i=0;i<num_v;i++) {
        fscanf(file, "%d", &G[i].index);
        fscanf(file, "%d", &G[i].num_neigh_in);
        for (j=0;j<G[i].num_neigh_in;j++)
            fscanf(file, "%d", &G[i].neigh_in[j].index);
        fscanf(file, "%lf", &G[i].threshold);
    }
    fclose (file) ;

    /* Create gurobi environment */
    error = GRBloadenv(&env, "tss.log");
    if (error) exit(1);

    /* Create an empty model */
    error = GRBnewmodel(env, &model, "TSS", 0, NULL, NULL, NULL, NULL, NULL);
    if (error) exit(1);

    /* Add variables - one for each node */
    /* obj = coefficient in objective function */
    /* variáveis sao armazenadas no gurobi na forma [x0,x1,...,xn, y01,y02,...,y0n, y1n, ..., ynn] */
    for (i=0; i<NUM_VERTICES; i++) {
        sprintf(name, "x_%d", i);
        error = GRBaddvar(model, 0, NULL, NULL, 1, 0.0, 1.0, GRB_BINARY, name);
        if (error) exit(1);
    }
    for (i=0; i<NUM_VERTICES; i++) {
        // sizeof(G[i].neigh_in)/sizeof(VERTEX)
        for (j=0; j<NUM_VERTICES; j++) {
            sprintf(name, "y_%d_%d", i, j);
            error = GRBaddvar(model, 0, NULL, NULL, 0, 0.0, 1.0, GRB_BINARY, name);
        }
    }

    // Threshold constraints
    for (i=0; i<NUM_VERTICES; i++) { // for every vertex v

        // t(v)*xv + (sum every y neigh of v) >= t(v)

        ind[0] = i; // xv
        val[0] = G[i].threshold;
        for (j=0;j<NUM_VERTICES;j++) {
            ind[j+1] = (NUM_VERTICES*i)+j + NUM_VERTICES;    // yv
            // if (i!=j && eh_vizinho(i, j))
            if (i!=j)
                val[j+1] = 1;
            else
                val[j+1] = 0;
        }

        sprintf(name, "thres_%d", i);

        error = GRBaddconstr(model, 1+NUM_VERTICES, ind, val, GRB_GREATER_EQUAL, G[i].threshold, name);
        if (error) exit(1);
    }

    // yuv + yvu = 1
    for (i = 0; i < NUM_VERTICES; i++) {
        for (j = 0; j < NUM_VERTICES; j++) {
            // if (i!=j && eh_vizinho(i, j)) {
            if (i!=j) {
                ind[0] = i*NUM_VERTICES+j + NUM_VERTICES;
                ind[1] = j*NUM_VERTICES+i + NUM_VERTICES;
                val[0] = 1;
                val[1] = 1;
                error = GRBaddconstr(model, 2, ind, val, GRB_EQUAL, 1, NULL);
            }
            if (error) exit(1);
        }
    }

    // yuv + yvw + ywu <= 2
    for (i = 0; i < NUM_VERTICES; i++) {            // v
        for (j = 0; j < NUM_VERTICES; j++) {        // u
            for (m = 0; m < NUM_VERTICES; m++) {    // w
                // if (i!=j && eh_vizinho(i, j)) {
                if (i!=j) {
                    ind[0] = j*NUM_VERTICES+i + NUM_VERTICES;   // yuv
                    ind[1] = i*NUM_VERTICES+m + NUM_VERTICES;   // yvw
                    ind[2] = m*NUM_VERTICES+j + NUM_VERTICES;   // ywu
                    val[0] = 1;
                    val[1] = 1;
                    val[2] = 1;
                    error = GRBaddconstr(model, 3, ind, val, GRB_LESS_EQUAL, 2, NULL);
                }
                if (error) exit(1);
            }
        }
    }

    //  Optimize model
    error = GRBoptimize(model);
    if (error) exit(1);

    // Extract solution
    double *sol = (double *) malloc(((NUM_VERTICES*NUM_VERTICES)+NUM_VERTICES)*sizeof(VERTEX));
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, NUM_VERTICES*NUM_VERTICES, sol);
    if (error) exit(1);

    printf("\n\nsol: \n");
    for (i=0;i<NUM_VERTICES;i++) {
        printf("x_%d: %f\n", i, sol[i*NUM_VERTICES+j+m]);
    }
    m=i;
    printf("\n");
    for (i=0;i<NUM_VERTICES;i++) {
        for(j=0;j<NUM_VERTICES;j++) {
            printf("y_%d_%d: %f\n", i, j, sol[i*NUM_VERTICES+j+m]);
        }
        printf("\n");
    }
}

/*  
    0  1  2  3  4    5    6  7   8    9   10  11  12
    x0 x1 x2 x3 x4  y00 y01 y02 y03  y10 y11 y12 y13
*/
