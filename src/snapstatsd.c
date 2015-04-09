#include "defines.h"
struct sembuf UP = {0, 1, 0};
struct sembuf DOWN = {0, -1, 0};

typedef struct{
	char user[40];
	char data[12];
}Tlinks;

typedef struct{
	char username[40];
	char ncompleto[60];
	char curso[20];
	char pass_sha1[42];
	char access_token[21];
	int last_pid;
	int nligacoes;
	Tlinks ligacoes[100];
}Tuser;

Tuser* u;
int nr_users;


void obter_substring(char linha[], char resultado[], char separador, int indice) {
   int i, j=0, meu_indice = 0;
   for (i=0; linha[i] != '\0'; i++) {
     if ( linha[i] == separador ) { 
       meu_indice++;
     } else if (meu_indice == indice) {
       resultado[j++]=linha[i];
     }
   }
   resultado[j]='\0';
}


void troca_ponto(char resultado[]){

	int i;
	
	for(i = 0; resultado[i] != '\0'; i++){
		if ( resultado[i] == '@' ){
			resultado[i] = '.';
		}
		
	}
}



void stats300(int signal){
	FILE *fd = fopen("stats.txt", "w");
	FILE *fu = fopen("../users/utilizadores.txt","r");
	char linha[100];

	if (fd == NULL){
                printf("Erro ao tentar abrir ficheiro!(1)\n");
                exit(1);
        }

	char tempo[30];
	struct tm *st;
    	time_t t = time (0);
	st = gmtime ( &t );
	
	strftime (tempo, sizeof(tempo), "%Y-%m-%d %H:%M:%S", st); 
	
	if(fu == NULL){
		printf("Erro ao tentar abrir ficheiro!(2)\n");
                exit(2);
	}	
	int nusers = 0;

	while(fgets(linha, 100, fu) != NULL ){
		nusers++;
	

	}		
		

	fprintf(fd, "%s\n", tempo);
	fprintf(fd, "Total utilizadores: %d\n", nusers);
	
	fclose(fu);

	char res[100];
	char linhanet[100];

	FILE *u = fopen("../users/utilizadores.txt","r");

	if(u == NULL){
                printf("Erro ao tentar abrir ficheiro!(2)\n");
                exit(2);
	}


 	while(fgets(linha, 100, u) != NULL ){
        	obter_substring(linha, res, ',', 0);
	
		fprintf(fd, "%s:",res);
        
		troca_ponto(res); // dada uma string substituir @ por .
	
		char local_fich[100];
		snprintf(local_fich, sizeof(local_fich), "../users/network_%s", res);	// Safe version of sprintf - sao iguais mas no snprintf 
											// indicamos o tamanho maximo de caracteres a produzir 
	
		FILE *nt = fopen(local_fich, "r");
		int l = 0;
	
		while(fgets(linhanet, 100, nt) != NULL ){
			l++;
		}
		fprintf(fd, " %d ligacoes\n", l);
		fclose(nt);	
	}

	fclose(fd);
	fclose(u);
	if (signal == 15 ){
		printf("Snapstatsd - Apanhei um sinal SIGTERM. A sair...\n");
		exit(0);
	}
}

void trata_sigint(int signal){
	printf("\nSnapstatsd - Recebi e ignorei um sinal SIGINT\n");
}

void checkNrUtilizadores(){
 	nr_users = 0;
        int i;

	for(i=0;i<100;i++) {
                if(strcmp(u[i].username, "(vazio)(1)") != 0){
                        nr_users++;
		}

        }
	printf("i ) Numero total de utilizadores: %d\n", nr_users);

}
void listarCursosDiferentes(){
        int i, j = 0;
        int posicao=0;
        char cursos[20][10];
        int h = 0;
        for(i=0; i != nr_users; i++){
                h = 0;
                if(i == 0){
                        strcpy(cursos[j], u[i].curso);
                        posicao++;
                        h = 1;
                }


                for(j = 0; j!= i; j++){
                        if((strcmp(u[i].curso, cursos[j])) == 0){
                                h = 1;
                        }
                }

                if(h != 1){
                        strcpy(cursos[posicao], u[i].curso);
                        posicao++;
                }


        }
        printf("ii ) Número de cursos diferentes: %d\n", posicao);
}

void listarNrDeLigacoes(){
	int i, j = 0;
        int nligacoestotal = 0;
        for(i = 0; i != nr_users; i++){
                for(j = 0; j != u[i].nligacoes; j++){
                        nligacoestotal++;
                }

        }

        printf("iii ) Número total de ligações : %d\n", nligacoestotal);
}


void regstats300(int signal){
	int idsem = semget ( 67671, 1, IPC_CREAT | 0666 );
	exit_on_error (idsem, "erro na Criação/Ligação sem");
	
	int statussem = semctl(idsem, 0, SETVAL, 100);
        exit_on_error(statussem, "erro ao fazer SETVAL");


	int id = shmget(67282, 0, 0 );
	exit_on_error(id, "erro ao criar a memoria");
 	printf("Estou ligado a %d\n", id);

  	u = (Tuser *) shmat(id, 0, 0);
  	if (u==NULL) { 
		perror("erro no attach");
		exit(1);
	 }
	nr_users = 0;
  	int i;
	
	statussem = semop ( idsem, &DOWN, 1 );
        exit_on_error(statussem, "erro ao fazer DOWN");
	
	checkNrUtilizadores();
	listarCursosDiferentes();
	listarNrDeLigacoes();
	
	statussem = semop (idsem, &UP,1);
        exit_on_error(statussem, "erro ao fazer UP");

	

}


int main(){

	signal(SIGALRM, regstats300);
	signal(SIGTERM, stats300);
	signal(SIGINT, trata_sigint);

	FILE *f = fopen("snapstatsd.pid", "w");
	if (f == NULL){
    		printf("Erro ao tentar abrir ficheiro!(3)\n");
    		exit(3);
	}

	int p = getpid();
	
	fprintf(f, "%d\n", p);
	fclose(f);
	
	// will run in background until it gets a SIGTERM sinal

	while(1){
		alarm(300);
		pause();
	}
	exit(1);

	

}
