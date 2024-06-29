#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool run = true;
//args
int tmv; //tam memoria virt em bits
int tmf; //tam memoria fisica em bits
int tpag; //tam pagina em bits
//tam das tabelas
int titpagv; //tam indice da pagina virtual(bits)
int timold; //tam indice das molduras(bits)
//primeiro nivel
int tipag1; //tam indice do primeiro nivel(bits)
//segundo nivel
int tipag2; //tam indice 2 nivel (bits)
unsigned long endvirt; //endereco virtual

//estrutura pag nivel 2
typedef struct {
    long int idxtabfis;
} tabpn2_t;
tabpn2_t *tabpn2;
unsigned long idxtabpn2;

//estrutura pag nivel 1
typedef struct {
    tabpn2_t *tabpnv2;
} tabpn1_t;
tabpn1_t *tabpn1;
unsigned long idxtapn1;

//estrutura memoria fisica
typedef struct {
    unsigned char inuse;
} tabmold_t;
tabmold_t *tabmold;
long int idxtabmold;


void init() {
    long int i;
    //aloca memoria pagina nivel 1
    tabpn1 = malloc(sizeof(int *) * ((unsigned long)1 << tipag1));
    //inicializa com -1
    for (i = 0; i < (1 << tipag1); ++i) {
        tabpn1[i].tabpnv2 = (tabpn2_t *) -1;
    }
    printf ("Tamanho tabela nivel 1 = %ld\n",i);
    printf("Tamanho tabela nivel 2 = %lu\n",(unsigned long)1<<tipag2);

    //aloca memoria para molduras
    tabmold = malloc(sizeof(unsigned char) * ((unsigned long)1 << timold));
    //inicializa com 0
    for (i = 0; i < (1 << timold); ++i) {
        tabmold[i].inuse = 0;
    }
    printf ("Tamanho vetor molduras = %ld\n", i);
};

tabpn2_t *alocaTabNv2() {
    //aloca memoria
    tabpn2_t *tmp = malloc(sizeof(long int) * ((unsigned long)1 << tipag2));
    //inicializa a tabela de nivel 2
    for (long int i = 0; i < (1 << tipag2); ++i) {
        tmp[i].idxtabfis = -1;
    }
    return tmp;
};


long int alocaRealMem() {
    //aloca moldura que estiver livre na tabela
    for (long int i = 0; i < ((long int)1 << timold); ++i) {
        if (tabmold[i].inuse == 0) {
            tabmold[i].inuse = 1;
            //retorna o indice da tabela alocada
            return i;
        }
    }
    //caso todas estiverem cheias
    return -1;
};


long int obtemIdxMold() {
    //encontra o indice o da tabela de paginacao de 1 nivel
    idxtapn1 = endvirt >> (tpag + tipag2);
    //se estiver vazia solicita alocacao de uma tabela de nivel 2
    if (tabpn1[idxtapn1].tabpnv2 == (tabpn2_t *) -1) {
        tabpn1[idxtapn1].tabpnv2 = alocaTabNv2();
    }
    //consulta de nivel 2
    tabpn2 = tabpn1[idxtapn1].tabpnv2;
    //calcula o indice da tabela de nivel 2
    idxtabpn2 = endvirt >> (tpag);
    //isola somente os bits que compoe o indice 2
    idxtabpn2 &= (((unsigned long)1 << tipag2) - 1);
    //se estiver vazia tenta alocar uma moldura
    if (tabpn2[idxtabpn2].idxtabfis == -1) {
        tabpn2[idxtabpn2].idxtabfis = alocaRealMem();
    }
    //retorna o indice da moldura alocada
    return tabpn2[idxtabpn2].idxtabfis;
};

unsigned long obtemEndReal() {
    //pega os bits de deslocamento da pagina
    unsigned long offset = endvirt &= ((1 << tpag) - 1);
    //obtem os bits do endereco base de memoria real
    unsigned long base = (idxtabmold << tpag);
    //calcula endereco real final
    unsigned long realend = base + offset;
    return realend;
};

void showmap(void) {
    unsigned long endreal = obtemEndReal();
    printf("End. Virtual:%lu --> ", endvirt);
    printf("End. Real:%lu\n", endreal);
};

void showtabpn() {
    for (long int i = 0; i < (1 << tipag1); ++i) {
        tabpn2 = tabpn1[i].tabpnv2;
        printf ("Nivel 1 - indice %ld\n",i);
        printf (" Nivel 2: ");
        for (long int j = 0; j < (1 << tipag2); ++j) {
            if (tabpn2 == (tabpn2_t *) -1) {
                printf("Nao utilizada");
                break;
            } else {
                printf("%ld ", tabpn2[j].idxtabfis);
            }
        }
        printf("\n");
    }
};
void showtabmold(){
    printf ("Tabela de molduras:\n");
    for (long int i = 0; i < (1 << timold); ++i) {
        printf ("Moldura [%li]: ender base: %li  usada: %u\n",i,(i<<tpag),tabmold[i].inuse);
    }
};

void showInit(){
    printf("\nResumo dos parametros de operacao\n");
    printf("Tamanho memoria virtual = %lu\nTamanho memoria fisica = %lu\nTamanho da pagina = %lu\n",(unsigned long)1 << tmv,(unsigned long)1 << tmf,(unsigned long)1 << tpag);
    printf ("Tamanho tabela nivel 1 = %ld\n",(unsigned long)1<<tipag1);
    printf("Tamanho tabela nivel 2 = %lu\n",(unsigned long)1<<tipag2);
    printf("Tamanho vetor de molduras = %lu\n",(unsigned long)1 << timold);

};

int main(int argc, char *argv[]) {
    tmv = atoi(argv[1]);
    tmf = atoi(argv[2]);
    tpag = atoi(argv[3]);
    //tam das tabelas
    titpagv = tmv - tpag;
    timold = tmf - tpag;
    //primeiro nivel
    tipag1 = titpagv / 2;
    //segundo nivel
    tipag2 = titpagv - tipag1;

    // cria estruturas
    init();

    while (run) {
        printf("Digite um endereco virtual\n");

        char str[128];
        scanf("%s", str);
        char *eptr;
        endvirt=strtoul(str,&eptr,10);
        printf("%lu ",endvirt);

        if (endvirt == (unsigned long) ~0) {
            printf("Fim do programa\n");
            break;
        }

        if(endvirt>= (unsigned long)1 << tmv){
            printf("Valor fora da faixa enderecamento virtual %lu\n",endvirt);
            continue;
        }

        idxtabmold = obtemIdxMold();
        if (idxtabmold == -1) {
            printf("Memory Full\n");
            break;
        }

        showmap();
        showtabpn();
        showtabmold();

    }
    showInit();
    return 0;
}
