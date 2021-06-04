/*--------------------------------------------------------------
 * PROCESSAMENTO DE IMAGEM -                                               
 * Atividade 3 - Decodificador Huffman - Base64
 * Aluno: Vitor Risso Parisi
 *--------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DIR 1
#define ESQ 0
#define TRUE 1
#define FALSE 0
#define MAXSYMBS 256
#define MAXNODES 511
#define CREATOR "# Imagem criada por Luiz Eduardo da Silva\n"
#define COCREATOR "# Codigo personalizado por Vitor Risso Parisi\n"

typedef int * imagem;
typedef int * histograma;

typedef struct {
    int freq;
    int father;
    int left;
    int right;
} nodetype;
nodetype node[MAXNODES];

int aloca_hist(histograma *H, int mx) {
    *H = (int *) malloc(mx * sizeof (int));
    if (*H) return TRUE;
    else return FALSE;
}

int aloca_ima(imagem *I, int nl, int nc) {
    *I = (int *) malloc(nl * nc * sizeof (int));
    if (*I) return TRUE;
    else return FALSE;
}

int desaloca_hist(histograma *H) {
    free(*H);
}

int desaloca_ima(imagem *B) {
    free(*B);
}

int pqmindelete(int *rootnodes) {
    int aux;
    aux = *rootnodes;
    *rootnodes = node[*rootnodes].father;
    return aux;
}

int pqinsert(int *rootnodes, int i) {
    int k, ant, aux;
    aux = *rootnodes;
    ant = -1;
    while (aux != -1 && node[aux].freq <= node[i].freq) {
        ant = aux;
        aux = node[aux].father;
    }
    if (ant == -1) {
        node[i].father = *rootnodes;
        *rootnodes = i;
    } else {
        node[i].father = node[ant].father;
        node[ant].father = i;
    }
}

int buildTree(int *h, int mn) {
    int i, k, p, p1, p2, rootnodes = -1, root, father;
    for (i = 0; i < mn + 1; i++) {
        node[i].freq = h[i];
        node[i].left = -1;
        node[i].right = -1;
        pqinsert(&rootnodes, i);
    }
    for (p = mn + 1; p < 2 * (mn + 1) - 1; p++) {
        p1 = pqmindelete(&rootnodes);
        p2 = pqmindelete(&rootnodes);
        node[p1].father = p;
        node[p2].father = p;
        node[p].freq = node[p1].freq + node[p2].freq;
        node[p].left = p1;
        node[p].right = p2;
        pqinsert(&rootnodes, p);

    }

    return pqmindelete(&rootnodes);
}

void hb64_pgm(int bit, int mn, histograma *H, imagem *I, int *root, int *cont, int resolucao) {
    int pixel;
    if(*cont<=resolucao){
        if (bit == 1) {
            if (node[*root].right <= mn) {
                pixel = node[*root].right;
                *(*(I) + (*cont)) = pixel;
                *cont = *cont + 1;
                *root = mn * 2;
            } else
                *root = node[*root].right;

        } else if (bit == 0) {
            if (node[*root].left <= mn) {
                pixel = node[*root].left;
                *(*(I) + (*cont)) = pixel;
                *cont = *cont + 1;
                *root = mn * 2;
            } else
                *root = node[*root].left;
        }
    }
}

void converte_bin(char c, imagem *I, int *root, int mn, histograma *H, int *cont, int resolucao) {
    unsigned char mask = 0x20;
    unsigned char i;
    char binv[128];
    int bit;
    char base64[128] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for (i = 0; i < 128; i++) {
        binv[base64[i]] = i;
    }
    while (mask) {

        bit = mask & binv[c] ? 1 : 0;
        hb64_pgm(bit, mn, H, I, root, cont, resolucao);
        mask = mask >> 1;
    }
}

int le_imagem_pgm(char nome[100], int *nl, int *nc, int *mn, histograma *H, imagem *I, char **nome_arq) {
    int i, LIMX, LIMY, MAX_NIVEL;
    char LINHA[100];
    int cont = 0;
    FILE *arq;
    
    if ((arq = fopen(nome, "r")) == NULL) {
        printf("Erro na ABERTURA do arquivo <%s>\n\n", nome);
        return FALSE;
    }
    /*-- PGM = "P2" -----------*/
    fgets(LINHA, 80, arq);
    if (!strstr(LINHA, "hb64")) {
        printf("Erro no FORMATO do arquivo <%s>\n\n", nome);
        return FALSE;
    }
    /*-- Dimensoes da imagem --*/
    fgets(LINHA, 80, arq);
    do {
        fgets(LINHA, 80, arq);
    } while (strchr(LINHA, '#'));
    sscanf(LINHA, "%d %d", &LIMX, &LIMY);
    fscanf(arq, "%d", &MAX_NIVEL);

    if (LIMX == 0 || LIMY == 0 || MAX_NIVEL == 0) {
        printf("Erro nas DIMENSOES do arquivo <%s>\n\n", nome);
        return FALSE;
    }
    
    /*--Histograma--*/
    int *aux;
    aloca_hist(&aux, MAX_NIVEL);
    if (aloca_hist(H, MAX_NIVEL)) {
        for (i = 0; i <= MAX_NIVEL; i++) {
            fscanf(arq, "%d", &aux[i]);
            H[i] = &aux[i];
        }
    } else {
        printf("Erro na ALOCACAO DE MEMORIA para o arquivo <%s>\n\n", nome);
        printf("Rotina: le_imagem_pgm\n\n");
        fclose(arq);
        return FALSE;
    }

    
    /*--Imagem codificada--*/
    int resolucao = LIMX * LIMY;
    int root = buildTree(*H, MAX_NIVEL);
    char s = fgetc(arq);
    s = fgetc(arq);
    
    if (aloca_ima(I, LIMY, LIMX)) {
        while ((s = fgetc(arq)) != EOF) {            
            if(s != 10 && s!=32){
                converte_bin(s, I, &root, MAX_NIVEL, H, &cont, resolucao);                
            }
        }
        fclose(arq);
    } else {
        printf("Erro na ALOCACAO DE MEMORIA para o codigo <%s>\n\n", nome);
        printf("Rotina: le_imagem_pgm\n\n");
        fclose(arq);
        return FALSE;
    }
    strcpy(*nome_arq,nome);
    *nc = LIMX;
    *nl = LIMY;
    *mn = MAX_NIVEL;
    return TRUE;
}

void grava_imagem_pgm(imagem B, char *nome, int nl, int nc, int mn) {
    int i, j, x, k, valores_por_linha;
    FILE *arq;
    if ((arq = fopen(nome, "wt")) == NULL) {
        printf("Erro na CRIACAO do arquivo <%s>\n\n", nome);
    } else {
        fputs("P2\n", arq);
        fputs(COCREATOR, arq);
        fprintf(arq, "%d  %d\n", nc, nl);
        fprintf(arq, "%d\n", mn);
        valores_por_linha = 10;
        for (i = 0, k = 0; i < nl; i++)
            for (j = 0; j < nc; j++) {
                x = *(B + i * nc + j);
                fprintf(arq, "%3d ", x);
                k++;
                if (k > valores_por_linha) {

                    fprintf(arq, "\n");
                    k = 0;
                }
            }
    }
    fclose(arq);
}

void info_imagem(int nl, int nc, int mn, char *nome) {

    printf("\nINFORMACOES SOBRE A IMAGEM:");
    printf("\n--------------------------\n");
    printf("Nome do arquivo.............: %s \n", nome);
    printf("Numero de linhas............: %d \n", nl);
    printf("Numero de colunas...........: %d \n", nc);
    printf("Maximo nivel-de-cinza/cores.: %d \n", mn);
}

void msg(char *s) {

    printf("\nConversor Huffman-Base64 para PGM");
    printf("\n---------------------------------");
    printf("\nUSO.:  %s  <IMG.HB64>", s);
    printf("\nONDE:\n");
    printf("    <IMG.HB64>  Arquivo da imagem em formato HB64\n\n");
}

int main(int argc, char *argv[]) {
    int OK, nc, nl, mn;
    char nome[100];
    char * nome_arq = nome;
    char comando[100];
    histograma H;
    imagem I;   
    argv[1]="imagem.hb64";
    if (argc < 2)
        msg(argv[0]);
    else {
        OK = le_imagem_pgm(argv[1], &nl, &nc, &mn, &H, &I,&nome_arq);
//        printf("\n teste nome :: %s", nome);
//        printf("\n teste nome :: %s", nome_arq);

       if (OK) {
            puts("");

            printf("\nDESCODIFICANDO IMAGEM HB64\n");
            info_imagem(nl, nc, mn,nome_arq);
            printf("\nIMAGEM(%s) DESCODIFICADA COM SUCESSO!!",nome_arq);
            
            strcat(nome_arq, "-descodificada.pgm");
            
            printf("\nNOVA IMAGEM CRIADA: %s",nome_arq);
            
            grava_imagem_pgm(I,nome_arq, nl, nc, mn);
            
            //strcpy(comando, "i_view32.exe "); // executar no WINDOWS
            strcpy(comando, "./i_view32 "); // executar no LINUX
            strcat(comando, nome_arq);
            strcat(comando, " /hide=7");

            system(comando);
            desaloca_hist(&H);
            desaloca_ima(&I);

        } else {
            printf("Le imagem deu ruuim \n\n");
        }
    }
    return 0;
}