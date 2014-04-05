#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <queue>
#include <sstream>
#include <fstream>
#include <string>

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

	stringstream s;

	//read file
	ifstream fin("../Documents/movies.txt");
	string line;
	vector <string> lines;
	while(getline(fin, line)){
		lines.push_back(line);
	}

	int num_movies = lines.size();

	seats_available = new int[num_movies];
	movie = new string[num_movies];
	
	//parse input vector
	for(int x = 0; x < num_movies; x++){

		//cout<<endl<<endl<<lines[x]<<endl;
		int seats;
		string temp_line;
		stringstream temp_ss;
		
		temp_ss.str("");
		temp_line = lines[x];
		temp_ss.str(temp_line);
		
		getline(temp_ss, temp_line, '\t'); 
		
		temp_ss >> seats;

		seats_available[x] = seats;
		movie[x] = temp_line;
	}

	pthread_t thread_cust[300], thread_agent[2], thread_conc, thread_taker;
	for(int x = 0;x<300;x++){
		pthread_create(&(thread_cust[x]), NULL, &thread_customer, NULL);
	}

	pthread_create(&(thread_agent[0]), NULL, &thread_box_agent, NULL);
	pthread_create(&(thread_agent[1]), NULL, &thread_box_agent, NULL);

	pthread_create(&thread_conc, NULL, &thread_concession_worker, NULL);

	pthread_create(&thread_taker, NULL, &thread_ticket_taker, NULL);

	//pthread_create(&thread_conc, NULL)
	
	s<<"\nhello world";
	print(s.str());
	s.str("");


	for(int x = 0;x<300;x++){
		if(pthread_join(thread_cust[x], NULL) == 0){
			s<<"\nJoined customer "<<x;
			print(s.str());
			s.str("");
		}
	}
	
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
		s.str("");
		sem_post(&mutex1);
	}

	//pick_movie
	choice[custnr] = rand() % 4;
	s<<"\nCustomer "<<custnr<<" buying ticket to "<<movie[choice[custnr]];
	print(s.str());
	s.str("");

	sem_wait(&box_agent);
		sem_wait(&mutex2);
			to_agent.push(custnr);
			sem_post(&ticket_payment);
			sem_post(&mutex2);
		sem_wait(&give_ticket);
			sem_post(&box_agent);

	if(ticket[custnr] == true) {
		s<<"\nCustomer "<<custnr<<" is in line to see ticket taker";
		print(s.str());
		s.str("");

		sem_wait(&ticket_taker);
			sem_wait(&mutex3);
				to_taker.push(custnr);
				sem_post(&mutex3);
			sem_post(&hand_over_ticket);
			sem_wait(&ticket_torn);
				sem_post(&ticket_taker);

		if((rand() % 2)==1){
			switch(rand() % 3){
				case 0: c_order[custnr] = "Popcorn and Soda"; break;
				case 1: c_order[custnr] = "Popcorn";break;
				case 2: c_order[custnr] = "Soda";break;
			}
			s<<"\nCustomer "<<custnr<<" is in line to buy "<<c_order[custnr];
			print(s.str());
			s.str("");

			sem_wait(&concession_worker);
				sem_wait(&mutex4);
					to_conc.push(custnr);
					sem_post(&order_ready);
					sem_post(&mutex4);
				sem_wait(&order_filled);
					sem_post(&concession_worker);

				s<<"\nCustomer "<<custnr<<" receives "<<c_order[custnr];
				print(s.str());
				s.str("");
		}

		s<<"\nCustomer "<<custnr<<" enters theater to see "<<movie[choice[custnr]];
		print(s.str());
		s.str("");
	}
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
		s.str("");
		sem_post(&mutex0);
	}

	while(true){
		sem_wait(&ticket_payment);
			sem_wait(&mutex2);
				b_cust = to_agent.front();
				to_agent.pop();
				s<<"\nBox office agent "<<agentnr<<" is serving customer "<<b_cust;
				print(s.str());
				s.str("");
				if(seats_available[choice[b_cust]] > 0){
					seats_available[choice[b_cust]]--;
					ticket[b_cust] = true;
					s<<"\nBox office agent "<<agentnr<<" sold ticket for "<<movie[choice[b_cust]]<<" to customer "<<b_cust;
				}
				else{
					s<<"\nBox office agent "<<agentnr<<" turns away customer "<<b_cust<<" because the movie "<<movie[choice[b_cust]]<<" is full";
				}
				sem_post(&mutex2);
			usleep((int)(time_sell_ticket/6)*1000);
			print(s.str());
			s.str("");
			sem_post(&give_ticket);
	}

	return 0;
}

void *thread_ticket_taker(void *args){
	int t_cust;
	stringstream s;
	s<<"\nTicket taker created";
	print(s.str());
	s.str("");

	while(true){
		sem_wait(&hand_over_ticket);
			sem_wait(&mutex3);
				t_cust = to_taker.front();
				to_taker.pop();
				sem_post(&mutex3);
			
			usleep((int)(time_tear_ticket/6)*1000);

			s<<"\nTicket taken from customer "<<t_cust;
			print(s.str());
			s.str("");
			
			sem_post(&ticket_torn);
	}
	return 0;
}

void *thread_concession_worker(void *args){

	int c_cust;
	stringstream s;

	s<<"Concession_worker created";
	print(s.str());
	s.str("");

	while(true) {

		sem_wait(&order_ready);
			sem_wait(&mutex4);
				c_cust=to_conc.front();
				to_conc.pop();
				sem_post(&mutex4);
				
			s<<"\nOrder of "<<c_order[c_cust]<<" taken from customer "<<c_cust;
			print(s.str());
			s.str("");
			
			usleep((int)(time_conc_order/6)*1000);

			s<<endl<<c_order[c_cust]<<" given to customer "<<c_cust;
			print(s.str());
			s.str("");

			sem_post(&order_filled);
	}

	return 0;
}

void *print(string s){
	sem_wait(&mutex5);
	cout<<s;
	sem_post(&mutex5);
	return 0;
}

