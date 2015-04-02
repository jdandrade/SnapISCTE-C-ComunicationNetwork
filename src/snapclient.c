#include "defines.h"
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

char name[100];
char pwd[100];

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


void sha1sum(char pass[],char hash[]) {
	// cria 2 pipes
	int pin[2], pout[2];
	pipe(pin);
	pipe(pout);
	// cria um processo filho para executar o comando "sha1sum"
	int n = fork();
	if( n == 0 ) {
		close(pin[0]);
		close(pout[1]);
		dup2(pin[1], 1 );
		dup2(pout[0], 0);
		execl("/usr/bin/sha1sum", "sha1sum", NULL );
		exit(1);
	}
	close(pin[1]);
	close(pout[0]);
	// escreve a password no pipe "pout", que vai ser o input do sha1sum
	FILE *f;
	f = fdopen( pout[1], "w" );
	fprintf(f, "%s", pass );
	fclose(f);
	// le o resultado atraves do pipe "pin", que e o output do sha1sum
	f = fdopen(pin[0], "r" );
	fgets(hash, 100, f);
	fclose(f);
	wait(NULL);

}

void pedeDadosUtilizador(){
        int s=0;


        printf("~~~~Autentificação~~~~\n");

        printf("Utilizador: ");
        fgets ( name, 100, stdin );
        name [strlen(name) -1 ] = '\0';

        printf("password: ");
        fgets ( pwd, 100, stdin );
        pwd[strlen(pwd)-1] = '\0';

}

int main(){
	int aut = 1;
	SnapStruct m;
	int status;
	int id1;
	while(aut!=0){
		pedeDadosUtilizador();
		char hash[50];
		char res[50];

		sha1sum(pwd, hash);
		obter_substring(hash, res, ' ', 0);
		printf("%s\n", res);
	
	    	m.tipo = 1;
		m.PID = getpid();
		strcpy(m.username, name);
		strcpy(m.pass_sha1, res);
		printf("%s\n", res);	

       		id1 = msgget(28671, 0 );
      		exit_on_error(id1, "Erro no msgget.");
      		printf("Estou a usar a fila de mensagens id=%d\n", id1);

		status = msgsnd(id1, &m, sizeof(m), 0);
	        exit_on_error(status, "erro ao enviar");


		status = msgrcv(id1, &m, sizeof(m), getpid(), 0);
		exit_on_error(status, "erro ao receber");
		printf("%d %s\n", m.resultado, m.access_token);
		if(m.resultado == 0){
			aut=0;
			printf("Autenticação - Sucesso\n");
		}else if(m.resultado == 1){
        	        printf("Autenticação - Insucesso ~ nome de utilizador errado\n");
	        }else if(m.resultado == 2){
               		printf("Autenticação - Insucesso ~ password incorrecta\n");
        	}

	}
	if(m.resultado == 0){
	printf("~~~~ MENU ~~~~\n");
		while(1){
			char opcao[1];
			printf("1. Ler mensagens\n");
			printf("2. Escrever mensagens\n");
			printf("3. Sair\n");
			printf("\nEscolha uma opcao (1/2/3)\n");
		
			fgets ( opcao, 100, stdin );
		        opcao[strlen(opcao)-1] = '\0';
			int o = atoi(opcao);
			
			//Opcao Ler mensagens
			
			if( o == 1 ) {
				printf("~~~~ LER MENSAGENS ~~~~\n");
				int x = 1;
				while(1){
					status = msgrcv(id1, &m,sizeof(m), getpid(), 0 | IPC_NOWAIT);
					if(status == -1){
						printf("Não tem mais mensagens!\n");
						break;
					}
					printf("[%d]__________________\n\n",x);
					printf("Username:  %s\n",m.username);
					printf("Msg: '%s'\n",m.mensagem);
					printf("_____________________\n\n");
					x++;
				}
			
			// Opcao Escrever mensagens
			
			}else if ( o == 2 ){
				char mensagem[182];
				printf("~~~~ ESCREVER MENSAGENS ~~~~\n");
				printf("Escreva a mensagem:\n");
				fgets(mensagem, 182, stdin);
				mensagem[strlen(mensagem)-1] = '\0';
				strcpy(m.username, name);
				m.tipo=2;
				strcpy(m.mensagem, mensagem);
				status = msgsnd(id1, &m, sizeof(m), 0);
				
			//Opcao SAIR
				
			}else if ( o == 3){
				printf("A sair...\n");
				exit(0);
			}
		}	
	}
}
