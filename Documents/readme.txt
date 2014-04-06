You need to have movies.txt and project2.cpp in the same directory. 

each movie needs to have a tab character between the movie name and the available tickets

Use the command "g++ -o project2 project2.cpp -pthread -lrt" (without quotes) in order to compile the program.

To run the program, use the command "./program2" to run the program. //start box office agent threads
	pthread_create(&(thread_agent[0]), NULL, &thread_box_agent, NULL);
	pthread_create(&(thread_agent[1]), NULL, &thread_box_agent, NULL);
	//start concession worker threads
	pthread_create(&thread_conc, NULL, &thread_concession_worker, NULL);
	//start ticket taker threads
	pthread_create(&thread_taker, NULL, &thread_ticket_taker, NULL);
