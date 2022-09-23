#include <stdio.h>
#include <stdlib.h>
#include "grafo.h"

#define LINESIZE 1024

// entrada:
// nVert nArest
// v1 nVizinhos
// .
// .
// .
// vn nVizinhos
// Vx Vy <- arestas
// . 
// .
// .
void le_grafo(grafo *G, FILE *file) {

    int i, j, v1, v2, iV, ch;

    for(i=0; i<G->n_vert; i++) {
        fscanf(file, "%d", &iV);
        fscanf(file, "%d", &G->v[iV]->nVizinhos);
        G->v[iV]->threshold = (double)G->v[iV]->nVizinhos/2;    // threshold = metade dos vizinhos
    }

    for(i=0; i<G->n_arest; i++) {
        fscanf(file, "%d", &v1);
        fscanf(file, "%d", &v2);
        G->v[v1]->vizinhos[v2]=1;
        G->v[v2]->vizinhos[v1]=1;
    }
}

// simula execução e testa se todos os nós foram ativados no final
int todosAtivos(grafo S) {
    int i, j, vizAtivos, sai, ativou; // flag ativou indica que algum vértice foi ativado no passo k
    int ativados[S.n_vert]; // indica vértices ativos em cada passo
    for (i=0;i<S.n_vert;i++)
        ativados[i]=0;
    for (;;) {   // passo
        ativou=0;
        for (i=0; i<S.n_vert; i++) {
            if (S.v[i]->estado==0) {
                vizAtivos=0;
                for (j=0; j<S.n_vert; j++)  // conta vizinhos ativos
                    if (S.v[i]->vizinhos[j]==1 && S.v[j]->estado==1)    // vizinho j está ativo
                        vizAtivos++;
                if (vizAtivos >= S.v[i]->threshold) {
                    ativados[i]=1;
                    ativou=1;
                }
            }
        }
        sai=0;
        for (i=0; i<S.n_vert && !sai; i++) {    // verifica se todos estão ativos
            if (S.v[i]->estado==0)
                sai=1;
        }
        if (sai==0) return 1;   // todos estão ativos
        if (ativou==0) return 0;   // nenhum vértice foi ativado no passo

        for (i=0;i<S.n_vert;i++)
            if (ativados[i])
                S.v[i]->estado=ativados[i];
    }
}

void copiaAtivos(grafo *G, grafo S) {   // G <- S
    int i;
    for (i=0; i<G->n_vert; i++)
        if (S.v[i]->estado)
            G->v[i]->estado=1;
}

void print_grafo (grafo G) {
    int i;
    char alfa[]={"abcdefghijklmnopqrstuvwxyz"};
    for (i=0; i<G.n_vert; i++)
        printf ("%c %d\n", alfa[i], G.v[i]->estado);
}


// index  ---> index atual em G
// k ---> Tamanho da combinação
int kCombinations (grafo S, int *ativados, int start, int end, int index, int k) {
    int i,j;
    if (index == k) {   // Combinação está completa
        for (i=0;i<k;i++)
            S.v[ativados[i]]->estado=1;
        if (todosAtivos(S)) return 1;
        else return 0;
    }

    // replace index with all possible elements. The condition
    // "end-i+1 >= k-index" makes sure that including one element
    // at index will make a combination with remaining elements
    // at remaining positions
    for (i=start; i<=end && end-i+1 >= k-index; i++) {
        for(j=0;j<S.n_vert;j++) S.v[j]->estado=0;
        ativados[index] = i;
        if (kCombinations(S, ativados, i+1, end, index+1, k))
            return 1;
    }
}


int greedyTSS(grafo *G) {
    int i, j;
    int *ativados = (int*) calloc(sizeof(int),G->n_vert);
    grafo *S = (grafo*) malloc(sizeof(grafo));
    S->n_vert = G->n_vert;    
    S->n_arest = G->n_arest;
    S->v = (vertice**) malloc(S->n_vert* sizeof(vertice*));
    for (i=0; i<G->n_vert; i++) {
        S->v[i] = (vertice*) calloc((S->n_vert), sizeof(vertice*));
        S->v[i]->vizinhos = (int*)malloc((S->n_vert) * sizeof(int));
        for (j=0; j<G->n_vert; j++) {
            if (G->v[i]->vizinhos[j]==1)
                S->v[i]->vizinhos[j]=1;
        }
        S->v[i]->threshold = G->v[i]->threshold;
    }
    
    int k;
    for (k=1; k<=G->n_vert; k++) {
        if (kCombinations(*S, ativados, 0, G->n_vert-1, 0, k) == 1) {    // testa todas as combinações de tamanho k
            for(i=0;i<k;i++)
                G->v[ativados[i]]->estado=1;
            free(S);
            free(ativados);
            return 1;
        }
    }
    return 0;
}

void main() {
    int i;
    grafo *G = (grafo*) malloc(sizeof(grafo));
    FILE *file;
    file = fopen("grafoC.txt", "r");
    fscanf(file, "%d", &G->n_vert);
    fscanf(file, "%d", &G->n_arest);

    G->v = (vertice**) malloc(G->n_vert* sizeof(vertice*));
    for (i=0; i<G->n_vert; i++) {
        G->v[i] = (vertice*) calloc((G->n_vert), sizeof(vertice*));
        G->v[i]->vizinhos = (int*)malloc((G->n_vert) * sizeof(int));
    }

    le_grafo(G, file);
    fclose(file);

    if (greedyTSS(G))
        print_grafo(*G);
    else 
        printf("não encontrou S\n");
    free(G);
}