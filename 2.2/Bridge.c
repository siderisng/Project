#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

#define StringSize 100		//megistos arithmos amaksiwn synolika
#define Capacity 5		//xwritikotita gefyras

void EnterCar(char *CarDir);
void ExitCar (char *CarDir);

void * Car(void* carDir);

char currDir;
pthread_mutex_t leftDir, rightDir, mtxCheck, mtxExit, mtxend;
int countl, countr, flag;
int carsPassed, carsleft, flag,waiting,startflag;


int main (int argc, char *argv[]){
 	char directions[StringSize];
	int i,thrCheck;
	pthread_t *p_thrd;
	
	flag=0;
	startflag=0;
	countl=0, countr=0;
	carsPassed= Capacity;
	carsleft=0;
	printf ("Please type the directions of the cars. l for left r for right:");
	scanf ("%s", directions);			//pairnoume eisodo gia ta aytokinita (l gia aristera, r gia deksia)
	printf ("\n");
	
	for (i=0; i< strlen(directions); i++){
		if (directions[i]=='r'){
			countr++;			//afksanoume to countr gia kathe amaksi apo deksia
		}
		else if(directions[i]=='l'){		//afksanoume to countl gia kathe amaksi apo aristera
			countl++;
		}
	}
	
	if (pthread_mutex_init(&mtxExit, NULL) != 0)	//arxikopoihsh mutex mtxExit
    {
        perror ("Mutex error");
        return 1;
    }
	
	if (pthread_mutex_init(&mtxend, NULL) != 0)	//arxikopoihsh mutex mtxend kai kleidwma
    {
        perror ("Mutex error");
        return 1;
    }
	
	pthread_mutex_lock (&mtxend);
	if (pthread_mutex_init(&rightDir, NULL) != 0)	//arxikopoihsh mutex rightDir
    {
        perror ("Mutex error");
        return 1;
    }
	
	if (pthread_mutex_init(&leftDir, NULL) != 0)	//arxikopoihsh mutex leftDir
    {
        perror ("Mutex error");
        return 1;
    }
	
	if (pthread_mutex_init(&mtxCheck, NULL) != 0)	//arxikopoihsh mutex mtxCheck
    {
        perror ("Mutex error");
        return 1;
    }
	
	
	
	if (NULL==(p_thrd= (pthread_t*)malloc(strlen(directions)*sizeof(pthread_t)))){	//desmefsi xwrou gia ton pinaka apo threads
		perror ("Memory allocation error");
	}
		
	for (i=0; i< strlen(directions); i++){
		thrCheck = pthread_create( &p_thrd[i], NULL, Car , (void *)(&directions[i]));		//dimiourgia threads
		if(thrCheck){
			fprintf(stderr,"Error - pthread_create() return code: %d\n",thrCheck);
			exit(EXIT_FAILURE);
		}
	}
	pthread_mutex_lock(&mtxend);		//lockarei edw mexri na oloklirwthei oli i diadikasia
	
	
	
	 pthread_mutex_destroy(&mtxend);	//diagrafi mutexes
	 pthread_mutex_destroy(&leftDir);
	pthread_mutex_destroy(&rightDir);
	pthread_mutex_destroy(&mtxCheck);
	pthread_mutex_destroy(&mtxExit);
	free (p_thrd);				//apeleftherwsi xwrou gia ton pinaka apo threads
	return 0;
}


void EnterCar(char *CarDir){
	
	carsPassed--;				// +1 amaksi sti gefira
	printf ("%c car on bridge\n",*CarDir);
	if ((carsPassed!=0)&&(currDir=='l')){
		pthread_mutex_unlock (&leftDir);	//an den einai gemati i gefira kai i katefthinsi einai aristera kseblokaroume ena amaksi apo aristera
	}
	else if ((carsPassed!=0)&&(currDir=='r')){	//to antistoixo gia amaksia apo deksia
		pthread_mutex_unlock (&rightDir);
		
	}
	
}

void ExitCar(char *CarDir){
	pthread_mutex_lock (&mtxExit);		//mono ena thread bainei kathe fora sto krisimo simeio 
	printf ("%c car leaving bridge\n", *CarDir);
	carsleft++;				// +1 amaksi stin gefyra
	
	if (*CarDir=='l'){			// an erxetai apo aristera h deksia to amaksi afairoume apo ton antistoixo counter
		countl--;}
	if (*CarDir=='r'){
		countr--;
	}
	if (countl==0){				//an den exei alla amaksia apo aristera kanoume tin katefthinsi deksia kai kseblokaroume amaksi apo deksia
		
		currDir='r';
		flag=1;
		
	}
	
	if (countr==0){				//an den exei alla amaksia apo deksia kanoume tin katefthinsi aristera kai kseblokaroume amaksi apo aristera
		
		currDir='l';
		flag=1;
	}
	
	if (flag==0){
		if ((carsleft==Capacity)&&(currDir=='r')){	//an exei gemisei i gefyra kai i diefthinsi einai apo deksia adeiazoume tin gefyra kai allazoume tin katefthinsi an exei enapomeinanta amaksia apo aristera
		
		carsleft=0;
		carsPassed= Capacity;
		
		
		
		
		if (countl>0){
			currDir='l';
			pthread_mutex_unlock(&leftDir);}
		}
		if ((carsleft==Capacity)&&(currDir=='l')){	//antistoixa me panw
		
		carsleft=0;
		carsPassed= Capacity;
		
		
		
		if (countr>0){
			currDir='r';
			pthread_mutex_unlock(&rightDir);
		}
		
		}
		
	}else {
		if (carsleft==Capacity){
			carsleft=0;
			carsPassed= Capacity;
		}
		if ((carsleft!=Capacity)&&(currDir=='r')){
			pthread_mutex_unlock(&rightDir);
		}else if((currDir=='l')&&(carsleft!=Capacity)){
			pthread_mutex_unlock(&leftDir);
		}
		
		
	}
	pthread_mutex_unlock (&mtxExit);		//kseblockarisma gia to epomeno thread 
}



void * Car(void* carDir){
	
	char * CarDir;
	CarDir =(char*)carDir;	

	pthread_mutex_lock (&mtxCheck);			//krisimo tmima
	if (startflag==0){
			
			//to prwto amaksi orizei tin arxiki katefthinsi
			if ('r'== *CarDir){			
				currDir='r';
				pthread_mutex_lock (&leftDir);
			}
			else if ('l'== *CarDir){
				currDir='l';
				pthread_mutex_lock (&rightDir);
			}
			startflag=1;
			
	}
	pthread_mutex_unlock (&mtxCheck);
	
	//blokarisma twn amaksiwn stin antistoixi oura deksia h aristera
	if ('r'== *CarDir){
		pthread_mutex_lock (&rightDir);		//an kseblokarei kai exei ti swsti katefthinsi mpainei stin gefyra
	}
	if ('l'== *CarDir){
		pthread_mutex_lock(&leftDir);
		
	}
	
	
	EnterCar (CarDir);
	
	ExitCar (CarDir);
	if ((countl==0) && (countr==0)){
		pthread_mutex_unlock (&mtxend);		//an teleiwsoun ta amaksia termatizei kai to programa
	}
	
	return (carDir);
}
