#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue>
#include <sstream>
#include <fstream>

using namespace std;
//declaring functions that the threads will use
void *thread_customer(void *arg); 
void *thread_box_agent(void *arg);
void *thread_ticket_taker(void *arg);
void *thread_concession_worker(void *arg);
void *print(string s); //used to provide mutual exclusion to cout 

sem_t box_agent, concession_worker, ticket_taker;
sem_t give_ticket, hand_over_ticket, order_filled, ticket_torn, order_ready, ticket_payment;
sem_t mutex0, mutex1, mutex2, mutex3, mutex4, mutex5;

int time_divisor = 60; //how many simulated seconds to one actual second
int time_tear_ticket = (int)(15 * 1000000) / time_divisor; //these are in microseconds (I'm using usleep)
int time_conc_order = (int)(180 * 1000000) / time_divisor;
int time_sell_ticket = (int)(90 * 1000000) / time_divisor;

bool ticket[300] = {false}; //true if the customer has received a ticket
int choice[300] = {0}; //sets itself to the number associated with customer's chosen movie
string c_order[300] = {""}; //set to user's order from the concession stand
queue <int> to_agent, to_conc, to_taker; //the queue used to tell the service threads who is being served
int cust_count = -1, agent_count = -1; //used to initialize cust and agent counts
string *movie; //used to store movie names from the file
int *seats_available; //used to store seats available from the file

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

	//used for output
	stringstream s;

	//read file into a vector
	ifstream fin("./movies.txt");
	string line;
	vector <string> lines;
	while(getline(fin, line)){
		lines.push_back(line);
	}

	int num_movies = lines.size();
	//initialize the movie array size
	seats_available = new int[num_movies];
	movie = new string[num_movies];
	
	//parse input vector 
	for(int x = 0; x < num_movies; x++){

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

	//initialize threads
	pthread_t thread_cust[300], thread_agent[2], thread_conc, thread_taker;

	//start box office agent threads
	pthread_create(&(thread_agent[0]), NULL, &thread_box_agent, NULL);
	pthread_create(&(thread_agent[1]), NULL, &thread_box_agent, NULL);
	//start concession worker threads
	pthread_create(&thread_conc, NULL, &thread_concession_worker, NULL);
	//start ticket taker threads
	pthread_create(&thread_taker, NULL, &thread_ticket_taker, NULL);
	//start customer threads
	for(int x = 0;x<300;x++){
		pthread_create(&(thread_cust[x]), NULL, &thread_customer, NULL);
	}

	//join customer threads
	for(int x = 0;x<300;x++){
		if(pthread_join(thread_cust[x], NULL) == 0){
			s<<"\nJoined customer "<<x;
			print(s.str());
			s.str("");
		}
	}
	
	//end after last customer joins
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
	//interact with a box office agent 
	sem_wait(&box_agent);
		sem_wait(&mutex2);
			to_agent.push(custnr);
			sem_post(&ticket_payment);
			sem_post(&mutex2);
		sem_wait(&give_ticket);
			sem_post(&box_agent);

	//continue only if the agent had a ticket to sell
	if(ticket[custnr] == true) {
		s<<"\nCustomer "<<custnr<<" is in line to see ticket taker";
		print(s.str());
		s.str("");
		//interact with ticket taker thread
		sem_wait(&ticket_taker);
			sem_wait(&mutex3);
				to_taker.push(custnr);
				sem_post(&mutex3);
			sem_post(&hand_over_ticket);
			sem_wait(&ticket_torn);
				sem_post(&ticket_taker);
		//do this if they randomly decide to buy from the concession stand
		if((rand() % 2)==1){
			//decide what to order
			switch(rand() % 3){
				case 0: c_order[custnr] = "Popcorn and Soda"; break;
				case 1: c_order[custnr] = "Popcorn";break;
				case 2: c_order[custnr] = "Soda";break;
			}
			s<<"\nCustomer "<<custnr<<" is in line to buy "<<c_order[custnr];
			print(s.str());
			s.str("");
			//interact with concession worker
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

		//announce that the customer is done and wait to be joined with main
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
	//main loop
	while(true){
		//interact with customer
		sem_wait(&ticket_payment);
			//take customer number off the queue
			sem_wait(&mutex2);
				b_cust = to_agent.front();
				to_agent.pop();
				s<<"\nBox office agent "<<agentnr<<" is serving customer "<<b_cust;
				print(s.str());
				s.str("");
				//sell ticket to customer if there is an available seat
				if(seats_available[choice[b_cust]] > 0){
					seats_available[choice[b_cust]]--;
					ticket[b_cust] = true;
					s<<"\nBox office agent "<<agentnr<<" sold ticket for "<<movie[choice[b_cust]]<<" to customer "<<b_cust;
				}
				//otherwise send the customer on his/her way
				else{
					s<<"\nBox office agent "<<agentnr<<" turns away customer "<<b_cust<<" because the movie "<<movie[choice[b_cust]]<<" is full";
				}
				sem_post(&mutex2);
			//simulate time spent on service
			usleep(time_sell_ticket);
			print(s.str());
			s.str("");
			//tell customer to continue
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
	//main loop
	while(true){
		//interact with customer
		sem_wait(&hand_over_ticket);
			//take customer number from the queue
			sem_wait(&mutex3);
				t_cust = to_taker.front();
				to_taker.pop();
				sem_post(&mutex3);
			//simulate time spent on service
			usleep(time_tear_ticket);

			s<<"\nTicket taken from customer "<<t_cust;
			print(s.str());
			s.str("");
			//tell customer to continue
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
	//main loop
	while(true) {
		//interact with customer
		sem_wait(&order_ready);
			//take customer number from the queue
			sem_wait(&mutex4);
				c_cust=to_conc.front();
				to_conc.pop();
				sem_post(&mutex4);
				
			s<<"\nOrder of "<<c_order[c_cust]<<" taken from customer "<<c_cust;
			print(s.str());
			s.str("");
			//simulate time spent on service
			usleep(time_conc_order);

			s<<endl<<c_order[c_cust]<<" given to customer "<<c_cust;
			print(s.str());
			s.str("");
			//tell customer to continue
			sem_post(&order_filled);
	}

	return 0;
}

//used to keep output from being corrupted by a semaphore to force mutual exclusion to cout
void *print(string s){
	sem_wait(&mutex5);
	cout<<s;
	sem_post(&mutex5);
	return 0;
}

