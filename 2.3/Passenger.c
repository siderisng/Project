#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define trainCap 10			//xwritikotita trainou

void startTrain();
void getOff();
void TrainReturn();

void * Passenger();


int timesToRun, passLeft,noBoard, capacity, flag;
pthread_mutex_t entry, board, end, ready, ride;




int main (int argc, char* argv[]){
	int i, noPass, thrCheck;
	pthread_t *pasThread;
	
	noBoard=0;
	flag=0;
	printf ("How many passengers? ");
	scanf (" %d", &noPass);				//dilwnoume posoi einai oi synolikoi epivates
	printf ("\n");
	
	timesToRun= noPass/trainCap;			//poses fores tha ksekinisei to treno (o pragmatikos arithmos einai timesToRun+1)
	passLeft= noPass%trainCap;			//posoi epivates perisevoun sto teleytaio treno
	capacity= trainCap;				//xwritikotita trenou
	
	if (pthread_mutex_init(&end, NULL) != 0)	//arxikopoihsh mutex end kai kleidwma
	{
		perror ("Mutex error");
		return 1;
	}
	
	pthread_mutex_lock (&end);
	
	if (pthread_mutex_init(&ready, NULL) != 0)	//arxikopoihsh mutex ready kai kleidwma
	{
		perror ("Mutex error");
		return 1;
	}
	
	pthread_mutex_lock (&ready);
	
	if (pthread_mutex_init(&ride, NULL) != 0)	//arxikopoihsh mutex ride kai kleidwma
	{
		perror ("Mutex error");
		return 1;
	}
	pthread_mutex_lock (&ride);
	
	if (pthread_mutex_init(&entry, NULL) != 0)	//arxikopoihsh mutex entry
	{
		perror ("Mutex error");
		return 1;
	}
	
	
	if (pthread_mutex_init(&board, NULL) != 0)	//arxikopoihsh mutex board kai kleidwma
	{
		perror ("Mutex error");
		return 1;
	}
	pthread_mutex_lock (&board);
	
	
	if (NULL==(pasThread= (pthread_t*)malloc(sizeof(pthread_t)*noPass))){	//desmefsi xwrou gia ton pinaka apo threads
		perror ("Memory allocation error!!!");
		return (1);
		
	}
	
	
	for (i=0; i<noPass; i++){
		
		thrCheck = pthread_create( &pasThread[i], NULL, Passenger , NULL);		//dimiourgia threads
		if(thrCheck){
			fprintf(stderr,"Error - pthread_create() return code: %d\n",thrCheck);
			exit(EXIT_FAILURE);
		}
		
		
	}
	pthread_mutex_lock (&end);	//blokarisma mexri na oloklirwthei i diadikasia
	pthread_mutex_destroy (&end);	//diagrafi twn mutexes
	pthread_mutex_destroy (&entry);
	pthread_mutex_destroy (&board);
	pthread_mutex_destroy (&ready);
	pthread_mutex_destroy (&ride);
	
	free (pasThread);		//apeleftherwsi xwrou tou pinaka twn threads
	return (0);
	
}


void startTrain(){
	
	noBoard--;	
	
	printf ("YOOOOOHOOOOOOO\n");
}
void getOff(){
	static int i=0;
	if (i==0){
		printf ("End of the ride!!!!\n");		//telos diadromis
	}
	printf ("It Was Great!!!\n");
	i= (i+1)%capacity;
	if (i==0){
		flag=1;
	}
}


void TrainReturn(){
	int i;
	
	if (timesToRun!=0){		//epistrofi trenou
		printf ("Returning back\n");
		for (i=0;i<3; i++){
			printf (".\n");
			sleep (1);
		}
		printf ("Waiting for Passengers\n");}	
		else {		
			printf ("Nothing else to do let's go home\n");
			pthread_mutex_unlock (&end); //termatismos programmatos
			
		}
}

void *Passenger(){
	
	pthread_mutex_lock (&entry);		//blokarisma twn epivatwn mexri na vevaiwthoume oti xwrane
	
	if ((timesToRun==0)&&(passLeft!=0)){ //an einai to teleytaio treno den perimenei na gemisei
			capacity=passLeft;		
			pthread_mutex_unlock (&entry);	
		}
	
	
	noBoard++;				//posoi einai sto treno
	
	
	
	if (noBoard!=capacity){			
		pthread_mutex_unlock (&entry);	//an den einai gemato bainei kai allos
	}else {
		pthread_mutex_unlock(&board);	//alliws fevgei to treno
	}
	
	pthread_mutex_lock(&board);		//kleidwma mexri na bei o teleytaios epivatis kathe trenou
	if (noBoard==capacity){
		printf ("Train Starts\n");
	}
	
	startTrain();			//ksekinaei to treno
	if (noBoard!=0){
		pthread_mutex_unlock (&board);	//ksekleidwma gia na ksekinisei to epomeno treno
	}
	
	
	if (noBoard==0){
		
		pthread_mutex_unlock (&ride);	 
	}
	
	
	pthread_mutex_lock (&ride);
	
	
	
	getOff();		//apovivasi
	
	
	if ((noBoard==0)&&(flag==1)){		//an exoun apovivastei oloi
		flag=0;
		printf ("Train returns\n");
		TrainReturn();
		pthread_mutex_unlock (&entry);	
		timesToRun--;
		
		
		if ((timesToRun==0)&&(passLeft==0)){
			printf ("Nothing else to do let's go home\n");
			pthread_mutex_unlock(&end); //termatismos
		}else{
			pthread_mutex_unlock (&entry);	
		}
	}
	else if (flag==0){		//epivivasi
		pthread_mutex_unlock (&ride);
	}
	
	
	return (NULL);
	
}



