#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <queue>
#include <sstream>

using namespace std;

void *thread_customer(void *arg);
void *thread_box_agent(void *arg);
void *thread_ticket_taker(void *arg);
void *thread_concession_worker(void *arg);
void *print(string s);

sem_t box_agent, concession_worker, ticket_taker;
sem_t give_ticket, hand_over_ticket, order_filled, ticket_torn, order_ready, ticket_payment;
sem_t mutex0, mutex1, mutex2, mutex3, mutex4, mutex5;

int time_tear_ticket = 15, time_conc_order = 180, time_sell_ticket = 90;
bool ticket[300] = {false};
int choice[300] = {0};
string c_order[300] = {""};
queue <int> to_agent, to_conc, to_taker;
int cust_count = -1, agent_count = -1;
string *movie;
int *seats_available;

int main(){
	//initialize all the semaphores!
	sem_init(&box_agent, 1, 2);
	sem_init(&concession_worker, 1, 1);
	sem_init(&ticket_taker, 1, 1);
	sem_init(&mutex0, 1, 1);
	sem_init(&mutex1, 1, 1);
	sem_init(&mutex2, 1, 1);
	sem_init(&mutex3, 1, 1);
	sem_init(&mutex4, 1, 1);
	sem_init(&mutex5, 1, 1);
	sem_init(&give_ticket, 1, 0);
	sem_init(&hand_over_ticket, 1, 0);
	sem_init(&order_filled, 1, 0);
	sem_init(&order_ready, 1, 0);
	sem_init(&ticket_payment, 1, 0);

	//read file
	seats_available = new int[4];
	seats_available[0] = 50;
	seats_available[1] = 50;
	seats_available[2] = 50;
	seats_available[3] = 50;

	movie = new string[4];
	movie[0] = "Test Movie One";
	movie[1] = "Test Movie Two";
	movie[2] = "Test Movie Three";
	movie[3] = "Test Movie Four";


	pthread_t thread_cust[300], thread_agent[2], thread_conc, thread_taker;
	for(int x = 0;x<300;x++){
		pthread_create(&(thread_cust[x]), NULL, &thread_customer, NULL);
	}

	pthread_create(&(thread_agent[0]), NULL, &thread_box_agent, NULL);
	pthread_create(&(thread_agent[1]), NULL, &thread_box_agent, NULL);

	//pthread_create(&thread_conc, NULL)
	
	cout<<"hello world";


	for(int x = 0;x<300;x++){
		if(pthread_join(thread_cust[x], NULL) == 0){
			cout<<"\nJoined customer "<<x;
		}
	}

	cout<<endl;
	return 0;
}

void *thread_customer(void *args){
	int custnr;
	stringstream s;
	//determine customer id number
	if (sem_wait(&mutex1) == 0){
		cust_count++;
		custnr=cust_count;
		s<<"\nCustomer "<<custnr<<" created.";
		print(s.str());
		sem_post(&mutex1);
	}

	//pick_movie
	choice[custnr] = rand() % 4;
	s<<"\nCustomer "<<custnr<<" is in line to see "<<movie[choice[custnr]];
	print(s.str());

	sem_wait(&box_agent);
		sem_wait(&mutex2);
			to_agent.push(custnr);
			sem_post(&ticket_payment);
			sem_post(&mutex2);
		sem_wait(&give_ticket);
			sem_post(&box_agent);

	return 0;
}

void *thread_box_agent(void *args){

	int agentnr;
	int b_cust;
	stringstream s;
	//determine agent id number
	if(sem_wait(&mutex0)==0){
		agent_count++;
		agentnr = agent_count;
		s<<"\nBox office agent "<<agentnr<<" created.";
		print(s.str());
		sem_post(&mutex0);
	}

	while(true){
		sem_wait(&ticket_payment);
			sem_wait(&mutex2);
				b_cust = to_agent.front();
				to_agent.pop();
				s<<"\nBox office agent "<<agentnr<<" is serving customer "<<b_cust;
				print(s.str());
				if(seats_available[choice[b_cust]] > 0){
					seats_available[choice[b_cust]]--;
					ticket[b_cust] = true;
				}
				sem_post(&mutex2);
			sleep(time_sell_ticket % 15);
			sem_post(&give_ticket);
	}

	return 0;
}

void *thread_ticket_taker(void *args){

	return 0;
}

void *thread_concession_worker(void *args){

	return 0;
}

void *print(string s){
	sem_wait(&mutex5);
	cout<<s;
	sem_post(&mutex5);
}