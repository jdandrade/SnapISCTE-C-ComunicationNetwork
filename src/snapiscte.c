#include "defines.h"

struct sembuf UPR = {0, 1, 0};
struct sembuf DOWNR = {0, -1, 0};
struct sembuf UPW = {0, 100, 0};
struct sembuf DOWNW = {0, -100, 0};

typedef struct{
	long tipo;
	int PID;
	char username[40];
	char pass_sha1[42];
	int resultado;
	char access_token[21];
	char mensagem[182];
}SnapStruct;

typedef struct{
        char user[40];
        char data[12];
} Tlinks;


typedef struct{
	char username[40];
	char ncompleto[60];
	char curso[20];
	char pass_sha1[42];
	char access_token[21];
	int last_pid;
	int nligacoes;
	Tlinks ligacoes[100];
} Tuser;

Tuser* u;

Tuser user[100];
int nusers;

char tmp[21];

void troca_ponto(char resultado[]){

        int i;

        for(i = 0; resultado[i] != '\0'; i++){
                if ( resultado[i] == '@' ){
                        resultado[i] = '.';
                }

        }
}



void obter_substring(char linha[], char resultado[], char separador, int indice) {
   int i, j=0, meu_indice = 0;
   for (i=0; linha[i] != '\0'; i++) {
     if ( linha[i] == separador ) { 
       meu_indice++;
     } else if (meu_indice == indice && linha[i] != '\n') {
       resultado[j++]=linha[i];
     }
   }
   resultado[j]='\0';
}

void enviaSinal(int signal){

	FILE *d = fopen("snapstatsd.pid", "r");

	char pid[10];
	if(d == NULL){
		printf("Erro na leitura do ficheiro (1)");
		exit(1);
}
	fgets(pid, 100, d);
	int pidint = atoi(pid);
	printf("Snapiscte - A enviar sinal para %d (snapstatsd)\n", pidint);
	
	kill(pidint, SIGTERM);
	wait();
	
	printf("Snapiscte -  A sair...\n");
	exit(0);

}



void carregarEstrutura(){
	FILE *f = fopen("../users/utilizadores.txt", "r");
        char linha[100];
        char res[100];
        
        nusers = 0;

        while(fgets(linha, 100, f) != NULL){
                obter_substring(linha, res, ',', 0);
                strcpy(u[nusers].username, res);


                obter_substring(linha, res, ',', 1);
                strcpy(u[nusers].ncompleto, res);


                obter_substring(linha, res, ',', 2);
                strcpy(u[nusers].curso, res);

                obter_substring(linha, res, ',', 3);
                strcpy(u[nusers].pass_sha1, res);

                char userponto[100];
                strcpy(userponto, u[nusers].username);

                troca_ponto(userponto); 			// dada uma string substituir @ por .

         
       		char local_fich[100];
                char linhanet[100];
                char netres[100];

                snprintf(local_fich, sizeof(local_fich), "../users/network_%s", userponto);
		
		FILE *net = fopen(local_fich, "r");
                int j=0;
                while(fgets(linhanet, 100, net) != NULL ){
                        obter_substring(linhanet, netres, ' ', 1);
                        strcpy(u[nusers].ligacoes[j].user, netres);

                        obter_substring(linhanet, netres, ' ', 0);
                        strcpy(u[nusers].ligacoes[j].data, netres);


                        j++;
			u[nusers].nligacoes = j;
                }
		if(u[nusers].nligacoes < 1)
			u[nusers].nligacoes = 0;
		fclose(net);
                nusers++;
	}
	
	fclose(f);
}

void apagarEstrutura(){
	
	int i, y;

	for(i=0; i != nusers; i++){
		for(y=0; y != u[i].nligacoes; y++){ 
			strcpy(u[i].ligacoes[y].user, "NULL");
			strcpy(u[i].ligacoes[y].data, "NULL");
		}
		strcpy(u[i].username, "NULL");
		strcpy(u[i].ncompleto, "NULL");
		strcpy(u[i].curso, "NULL");
		strcpy(u[i].pass_sha1, "NULL");
		u[i].nligacoes=0;
	}

	
}

void listar_CursosDiferentes(){
	int i, j = 0;
	int posicao=0;
	char cursos[20][10];
	int h = 0;
	for(i=0; i != nusers; i++){
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

void listar_numeroLigacoes(){
	int i, j = 0;
	int nligacoestotal = 0;
	for(i = 0; i != nusers; i++){
		for(j = 0; j != u[i].nligacoes; j++){
			nligacoestotal++;
		}

	}

	printf("iii ) Número total de ligações : %d\n", nligacoestotal);
}

void apresentarDados(){
	printf("i ) Número de utilizadores : %d\n", nusers);    // i)
	listar_CursosDiferentes();          			// ii)
	listar_numeroLigacoes();				// iii)
}

void trataUSR1(int signal){
      apagarEstrutura();
      carregarEstrutura();
	apresentarDados();
}

void checkConstaNoutrosNetworks(const char *ler){

	int i, j;
	int flag = 0;
	for(i = 0 ; i != nusers ; i++){
		for( j = 0; j != user[i].nligacoes; j++){
			if(strcmp(user[i].ligacoes[j].user, ler) == 0){
				printf("	%s\n", user[i].username);
				flag = 1;
			}
		}
	
	}
	if (flag == 0){
		printf("	Nenhuma!\n");
	}

}

void checkUtilizador(const char *ler){

	int i, j;
	int existe = 0;
	for(i=0; i!= nusers; i++){
		if(strcmp(user[i].username, ler) == 0){
			existe=1;
			printf("\nUtilizador já existente!\n");
			printf("\nNome: %s\n\n", user[i].ncompleto);
			printf("Curso: %s\n\n", user[i].curso);
			printf("~~Lista de ligações~~\n");
			if(user[i].nligacoes == 0){
				printf("Este utilizador não tem ligações!\n");
			}else{
				for(j = 0; j != user[i].nligacoes; j++){
					printf("	Ligacao #%d: %s\n", j+1, user[i].ligacoes[j].user);
				}
			}
		}		
	}
	if(existe == 0){
		printf("Utilizador não existe!\n");
	}else{
		printf("\n~~Lista de utilizadores em que consta como ligacao~~\n");
		checkConstaNoutrosNetworks(ler);
	}

}

void verificarProcesso(){
	FILE *p = fopen("snapstatsd.pid", "r");
	if (p == NULL){
                printf("Erro ao tentar abrir ficheiro!(3)\n");
                exit(3);
        }
	char pid[10];
        
	fgets(pid, 100, p);
        int pidint = atoi(pid);

	fclose(p);
	
	if(kill(pidint, 0) == 0){
		printf("Processo Snapstatsd já está em execução! A sair...\n");
		exit(0);
	}

}
void criarSHM(){

	int id = shmget(67282, 100*sizeof(Tuser), IPC_CREAT | 0666);
	exit_on_error(id, "Erro ao criar a memoria!");

	u = (Tuser *) shmat(id, 0, 0);
	if (u==NULL) { 
		perror("erro no attach"); 
		exit(1);
	}
	
	int i;
	int j;
  	
	for(i=0;i<100;i++) {
		strcpy(u[i].username, "(vazio)(1)");
		strcpy(u[i].ncompleto, "(vazio)(2)");
		strcpy(u[i].curso, "(vazio)(3)");
		strcpy(u[i].pass_sha1, "(vazio)(4)");
		strcpy(u[i].access_token, "(vazio)(5)");
		u[i].last_pid = -1;
		u[i].nligacoes = -1;
		for(j=0; j<3;j++){
			strcpy(u[i].ligacoes[j].user, "(vazio)(6)");
			strcpy(u[i].ligacoes[j].data, "(vazio)(7)");
		}	

	}
}

char *geradorAccessToken(char tmp[], size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;
    	for(i = 0; i != length; i++) {
        	size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        	tmp[i] = charset[index];
    	}
	tmp[strlen(tmp)-1] = '\0';
	return tmp;
}

int autenticacao(SnapStruct m){
	int i;
	int result = 1;
	for(i = 0; i != 100; i++){
		if(strcmp(u[i].username, m.username)==0){
			strcpy(u[i].access_token, tmp);
			result=2;
			if(strcmp(u[i].pass_sha1, m.pass_sha1)==0){
				result=0;
				u[i].last_pid = m.PID;
			}
		}
	}
	
	return result;
}	

void trataMensagens(int id, int idsem,int statussem, SnapStruct m){
int n = fork();
	
				//tipo1
        if(n==0){
                while(1){
                        int status = msgrcv(id, &m, sizeof(m),1,0);
                        exit_on_error(status, "Erro ao receber mensagem (1)!");
                        printf("[1] Recebi pedido de autenticação do User '%s', PWD_SHA1 '%s'\n",m.username, m.pass_sha1);

                        geradorAccessToken(tmp, 21);
                        strcpy(m.access_token, tmp);

                        statussem = semop ( idsem, &DOWNW, 1 );
                        exit_on_error(statussem, "erro ao fazer DOWN 100");

                        m.resultado = autenticacao(m);

                        statussem = semop (idsem, &UPW,1);
                        exit_on_error(statussem, "erro ao fazer UP 100");

                        if(m.resultado == 0){
                                printf("[1] Autenticação do User '%s' - Sucesso\n", m.username);
                        }else{
                                printf("[1] Autenticação do User '%s' - Insucesso\n", m.username);
                        }

                        printf("[1] Access Token atribuida a '%s' : '%s'\n",m.username, m.access_token);
                        m.tipo = m.PID;
                        int status1 = msgsnd(id, &m, sizeof(m),0);
                        exit_on_error(status, "erro ao enviar");
                        printf("[1] Access Token enviado a '%s'\n", m.username);
                }
				//tipo2
        }else{
		 while(1){
                        int status = msgrcv(id, &m, sizeof(m),2,0);
                        exit_on_error(status, "Erro ao recceber mensagem(2)!");
                        printf("[2] Recebi de %s a mensagem: %s\n", m.username, m.mensagem);
                        printf("[2] Verificação de User '%s' e AT '%s'\n", m.username, m.access_token);

                        statussem = semop ( idsem, &DOWNR, 1 );
                        exit_on_error(statussem, "erro ao fazer DOWN 1");

                        int i, j, x;
                        for(i = 0; i != nusers; i++){
                                if(strcmp(u[i].username, m.username)==0){
                                        printf("[2] Achou o user: '%s'\n", u[i].username);
                                        printf("[2] A comparar '%s' - '%s'\n",u[i].access_token, m.access_token);
                                        if(strcmp(u[i].access_token, m.access_token)==0){
                                                printf("[2] Verificacao de Access Token - sucesso!\n");
                                                if(u[i].nligacoes == 0)
                                                        printf("[2] User '%s' tem 0 ligações. Não foi enviada qualquer mensagem.\n",u[i].username);
                                                for(j=0;j!=u[i].nligacoes;j++){
                                                        for(x=0; x!=nusers; x++){
                                                                if(strcmp(u[i].ligacoes[j].user,u[x].username)==0 && u[x].last_pid != -1){
                                                                        m.tipo = u[x].last_pid;
                                                                        printf("[2] nligacoes - %d\n", u[i].nligacoes);
                                                                        printf("[2] Enviada msg para o processo - %d\n", u[x].last_pid);
                                                                        status = msgsnd(id, &m, sizeof(m),0);
                                                                        exit_on_error(status,"erro ao enviar");
                                                                }
                                                        }

                                                }
                                        }

                                }
                        }
                        statussem = semop (idsem, &UPR,1);
                        exit_on_error(statussem, "erro ao fazer UP 1");
                }
        }

}


int main(){
	signal(SIGINT, enviaSinal);
	signal(SIGUSR1, trataUSR1);

        int idsem = semget(28281, 2, IPC_CREAT | 0666 );
        exit_on_error(idsem, "erro ao criar");

        int statussem = semctl(idsem, 0, SETVAL, 100);
        exit_on_error(statussem, "erro ao fazer SETVAL");
	
	verificarProcesso();
	statussem = semop ( idsem, &DOWNW, 1 );
	exit_on_error(statussem, "erro ao fazer DOWN 100");
	criarSHM();	
	carregarEstrutura();
	statussem = semop (idsem, &UPW,1);
	exit_on_error(statussem, "erro ao fazer UP 100");

	int f = fork();

        if(f==0){
                execl("./snapstatsd","snapstatsd", NULL);
	}

	SnapStruct m;
	int id = msgget(28671, 0666 | IPC_CREAT );
	printf("Estou a usar a fila de mensagens id=%d\n", id);	
	
	
	trataMensagens(id, idsem, statussem, m);
}
