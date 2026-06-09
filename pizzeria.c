//den exoyn allax8ei signal k w8


#include "pizzeria.h"
int n_cust;
int n_fail=0;
int n_pass=0;
int total_special = 0;
int total_plain = 0;
int total_cost =0;
int cost = 0;
int n_available_cooks = Ncook;
int n_available_ovens = Noven;
int n_available_packer = Npacker;
int n_available_deliverers = Ndeliverer;
int total_service_time = 0; int  max_service_time = -1; int total_colding_time = 0; int max_colding_time = -1;
unsigned int seed;
int rc;


pthread_mutex_t  lock_fournoi, lock_psistis, lock_packet, lock_deliv, lock_screen, operator_lock  , fail_lock;
pthread_cond_t  cond_fournoi, cond_psistis, cond_packet, cond_deliv;


void rc_check(int rc){
	if(rc != 0){
		printf("ERROR: return code %d from rc",rc);
		destructor();
		exit(-1);
	}	
}
void mutex_destroyer(pthread_mutex_t *thread){
	int rc = pthread_mutex_destroy(thread);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		//exit(-1);	destructor will continue to destroy the other mutexes/conds		
	}
}
void cond_destroyer(pthread_cond_t *thread){
	int rc = pthread_cond_destroy(thread);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		//exit(-1);	destructor will continue to destroy the other mutexes/conds
	}
}

void mutex_lock(pthread_mutex_t *thread){
	int rc = pthread_mutex_lock(thread);
	rc_check(rc);
}

void mutex_unlock(pthread_mutex_t *thread){
	int rc = pthread_mutex_unlock(thread);
	rc_check(rc);
}

void destructor(){
	//Destructors	
	mutex_destroyer(&lock_psistis);
	mutex_destroyer(&lock_fournoi);
	mutex_destroyer(&lock_packet);
	mutex_destroyer(&lock_deliv);
	mutex_destroyer(&lock_screen);
	mutex_destroyer(&operator_lock);
	mutex_destroyer(&fail_lock);

	cond_destroyer(&cond_psistis);
	cond_destroyer(&cond_fournoi);
	cond_destroyer(&cond_packet);
	cond_destroyer(&cond_deliv);
	pthread_exit(NULL);
}





int main(int argc, char *argv[]) {
	
	//Elegxos egkurwn parametrwn.
	if (argc != 3) {
		printf("Δώσατε λάθος αριθμό παραμέτρων.\n");
		exit(-1);
	}
	
	int rc;
	double next_order;
	n_cust = atoi(argv[1]);
	seed = atoi(argv[2]);
	int order_id[n_cust];

    //Check number of customers
    if(n_cust <0) {
        printf("Μη έγκυρος αριθμός.\n\n");
        exit(-1);
    }	

	if(initializations() == 1) {
		pthread_exit(NULL);
		exit(-1);
	}
	pthread_t threads[n_cust];
	
	//Create Customers threads
	for (int i = 0; i < n_cust; i++) {
		order_id[i] = i + 1;
		rc = pthread_create(&threads[i], NULL, order, &order_id[i]);
		if (rc != 0) {
    		printf("ERROR: return code from pthread_create() is %d\n", rc);
			pthread_exit(NULL);
       		exit(-1);
		}
		unsigned int local_seed = seed + order_id[i];
		next_order = rand_r(&local_seed) % (Torderhigh + 1 - Torderlow) + Torderlow;
		sleep(next_order);
	}
	
	for (int i = 0; i < n_cust; i++) {
		rc = pthread_join(threads[i], NULL);
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}
	}
	
	
	printf("Τα συνολικά έσοδα από τις πωλήσεις: %d ευρω.\n", total_cost);
    printf("Tο πλήθος των special παραγγελιών: %d .\n",total_special);
	printf("Tο πλήθος των plain παραγγελιών: %d .\n", total_plain);
	printf("Tο πλήθος των επιτυχημένων παραγγελιών: %d .\n", n_pass);
	printf("Tο πλήθος των αποτυχημένων παραγγελιών: %d .\n", n_fail);
    printf("Μέσος χρόνος εξυπηρέτησης των πελατών: %.2f λεπτά.\n",(double) total_service_time/ n_pass);
	printf("Μέγιστος χρόνος εξυπηρέτησης των πελατών: %d λεπτά.\n", (int) max_service_time);
    printf("Μέσος χρόνος  κρυώματος των παραγγελιών: %.2f λεπτά.\n", (double) total_colding_time/ n_pass);
	printf("Μέγιστος χρόνος  κρυώματος των παραγγελιών: %d λεπτά.\n", (int) max_colding_time);
	
	
	destructor();
	return 0;
}




void * order(void *threadid) {
	int OrderID = *(int *)threadid;											//number of order
	unsigned int local_seed = seed + OrderID;
	int number_of_pizzas = rand_r(&local_seed)%(Norderhigh - Norderlow + 1) + Norderlow;
	struct timespec begin, kryom, end, packet;
	

	
								
	clock_gettime(CLOCK_REALTIME, &begin);		//time order begins

	
	int special = 0; int plain = 0;				//number of plain and special 
    for (int j = 0; j< number_of_pizzas;j++){	//pizzas ordered
    	//int ls = local_seed + j;
        int c = (rand_r(&local_seed)%100+1 <= Pplain*100 ? 1 : 0);//the possibility each pizza is plain or not
        if(c==1) plain++;
        if(c==0) special++;
    }
	cost = 12*special + 10*plain;

	
	mutex_lock(&lock_screen);
	printf("Η παραγγελία με id %d περιέχει %d special, %d απλές και κοστίζει %d ευρώ.\n", OrderID, special, plain, cost);
	mutex_unlock(&lock_screen);
	
	
	int payment = (rand_r(&local_seed)%100+1 <= Pfail*100 ? 0 : 1);
    printf("Payment for order %d %s\n",OrderID,  payment == 0? "failed." : "succeded.");

    sleep((rand_r(&local_seed) %(Tpaymenthigh - Tpaymentlow + 1)) + Tpaymentlow);	//check the card
	if(payment!= 1){ n_fail++; pthread_exit(NULL);}				//if payment failed


	//if payment succeded

	//update total revenue and quantity of each type of pizza
	mutex_lock(&operator_lock);
	total_cost += cost;
	total_plain = total_plain + plain;
	total_special = total_special + special;
	mutex_unlock(&operator_lock);
	//order and payment completed 

	//Cook availability check

	mutex_lock(&lock_psistis);					//cook locked
	while (n_available_cooks == 0) {
		mutex_lock(&lock_screen);
		printf("H παραγγελία με αριθμό %d δεν βρήκε διαθέσιμο ψήστη...\n", OrderID);
		mutex_unlock(&lock_screen);
		pthread_cond_wait(&cond_psistis, &lock_psistis);
    } 

	mutex_lock(&lock_screen);
	printf("H παραγγελία με αριθμό %d βρήκε διαθέσιμο ψήστη!\n", OrderID);
	mutex_unlock(&lock_screen);

    n_available_cooks--;
    mutex_unlock(&lock_psistis);

	//wait for pizzas to be prepared by the cook
    sleep(Tprep * number_of_pizzas);	


	//Oven availability check

    mutex_lock(&lock_fournoi);
	while (n_available_ovens < number_of_pizzas) {
		mutex_lock(&lock_screen);
		printf("H παραγγελία με αριθμό %d δεν βρήκε τον απαραίτητο αριθμό διαθέσιμων φούρνων.... \n", OrderID);
		mutex_unlock(&lock_screen);
		rc = pthread_cond_wait(&cond_fournoi, &lock_fournoi);
		rc_check(rc);
    }

	mutex_lock(&lock_screen);
	printf("H παραγγελία με αριθμό %d βρήκε τον απαραίτητο αριθμό διαθέσιμων φούρνων!\n", OrderID);
	mutex_unlock(&lock_screen);

	n_available_ovens -= number_of_pizzas;
	mutex_unlock(&lock_fournoi);

	//Release cook as the pizzas are baking
	mutex_lock(&lock_psistis);					//changed the positions of where the mutexes are locked and unlocked
	n_available_cooks++;
	rc = pthread_cond_signal(&cond_psistis);
	rc_check(rc);
	mutex_unlock(&lock_psistis);

	//wait till the pizzas are baked
    sleep(Tbake);	

	mutex_lock(&lock_screen);
    printf("H παραγγελία με αριθμό %d ψήθηκε επιτυχώς! \n", OrderID);
	mutex_unlock(&lock_screen);


	clock_gettime(CLOCK_REALTIME, &kryom);

	//Packer availability check

	mutex_lock(&lock_packet);
	while (n_available_packer == 0) {
		mutex_lock(&lock_screen);
		printf("H παραγγελία με αριθμό %d δεν βρήκε διαθέσιμο υπάλληλο πακεταρίσματος...\n", OrderID);
		mutex_unlock(&lock_screen);
		rc = pthread_cond_wait(&cond_packet, &lock_packet);
		rc_check(rc);
    }

	mutex_lock(&lock_screen);
	printf("H παραγγελία με αριθμό %d βρήκε διαθέσιμο υπάλληλο πακεταρίσματος!\n", OrderID);
	mutex_unlock(&lock_screen);

    n_available_packer--;
    mutex_unlock(&lock_packet);

	//wait till the pizzas are packed and ready to go
    sleep(Tpack*number_of_pizzas);
	clock_gettime(CLOCK_REALTIME, &packet);				

	double delta = packet.tv_sec - begin.tv_sec;
    int packettime = (int)delta;

	mutex_lock(&lock_screen);
	printf("H παραγγελία με αριθμό %d ετοιμάστηκε σε %d", OrderID, packettime);	
	if (packettime%60 < 10)
		printf(",0");
	printf(",%d λεπτά\n",packettime%60);
	mutex_unlock(&lock_screen);


	//Release packer, pizzas are ready to go
	mutex_lock(&lock_packet);
	n_available_packer++;
	rc = pthread_cond_signal(&cond_packet);
	rc_check(rc);
	mutex_unlock(&lock_packet);

	//Release ovens, the pizzas are already baked and packed
	mutex_lock(&lock_fournoi);
	n_available_ovens += number_of_pizzas;
	rc = pthread_cond_signal(&cond_fournoi);
	rc_check(rc);
	mutex_unlock(&lock_fournoi);

	//Deliverer availability check

	mutex_lock(&lock_deliv);
	while (n_available_deliverers == 0) {
		mutex_lock(&lock_screen);
		printf("H παραγγελία με αριθμό %d δεν βρήκε διαθέσιμο διανομέα...\n", OrderID);
		mutex_unlock(&lock_screen);
		rc = pthread_cond_wait(&cond_deliv, &lock_deliv);
		rc_check(rc);
    }

	mutex_lock(&lock_screen);
	printf("H παραγγελία με αριθμό %d βρήκε διαθέσιμο διανομέα!\n", OrderID);
	mutex_unlock(&lock_screen);

    n_available_deliverers--;
    mutex_unlock(&lock_deliv);



	//wait till the order are delivered to the customer
    int time = (rand_r(&local_seed) %(Tdelhigh - Tdellow + 1)) + Tdellow;
	sleep(time);													
	clock_gettime(CLOCK_REALTIME, &end);
	delta = end.tv_sec - begin.tv_sec;

	//update total service time
	mutex_lock(&operator_lock);
    total_service_time += (int)delta;
	pthread_mutex_unlock(&operator_lock);

	mutex_lock(&lock_screen);
	printf("H παραγγελία με αριθμό %d παραδόθηκε σε %d λεπτά.\n", OrderID,(int)delta);
	mutex_unlock(&lock_screen);

	mutex_lock(&operator_lock);
	if ((int)delta > max_service_time) {
		max_service_time = (int)delta;						//max service time, since the customer's order arrived
	}
	delta = end.tv_sec - kryom.tv_sec;				
	if ((int)delta > max_colding_time) {
		max_colding_time = (int)delta;						//max colding time, since the order is out of the oven
	}
	total_colding_time+= (int) delta;
	n_pass++;
	
	mutex_unlock(&operator_lock);

	//wait till delivery guy is back in the pizzeria
	sleep(time);													
	mutex_lock(&lock_deliv);
	n_available_deliverers++;
    rc = pthread_cond_signal(&cond_deliv);
	rc_check(rc);
    mutex_unlock(&lock_deliv);

	pthread_exit(NULL);
}




int initializations(){
	//Initialize	
	if (pthread_mutex_init(&lock_psistis, NULL) != 0) {
        return 1;
    }
	if (pthread_mutex_init(&lock_fournoi, NULL) != 0) {
		mutex_destroyer(&lock_psistis);
        return 1;
    }
	if (pthread_mutex_init(&lock_packet, NULL) != 0) {
		mutex_destroyer(&lock_psistis);
		mutex_destroyer(&lock_fournoi);
        return 1;
	}
	if (pthread_mutex_init(&lock_deliv, NULL) != 0) {
		mutex_destroyer(&lock_psistis);
		mutex_destroyer(&lock_fournoi);
		mutex_destroyer(&lock_packet);
        return 1;
    }
	if (pthread_mutex_init(&lock_screen, NULL) != 0) {
		mutex_destroyer(&lock_psistis);
		mutex_destroyer(&lock_fournoi);
		mutex_destroyer(&lock_packet);
		mutex_destroyer(&lock_deliv);
        return 1;
    }
	if (pthread_mutex_init(&operator_lock, NULL) != 0) {
		mutex_destroyer(&lock_psistis);
		mutex_destroyer(&lock_fournoi);
		mutex_destroyer(&lock_packet);
		mutex_destroyer(&lock_deliv);
		mutex_destroyer(&lock_screen);
        return 1;
    }
	if (pthread_mutex_init(&fail_lock, NULL) != 0) {
		mutex_destroyer(&lock_psistis);
		mutex_destroyer(&lock_fournoi);
		mutex_destroyer(&lock_packet);
		mutex_destroyer(&lock_deliv);
		mutex_destroyer(&lock_screen);
		mutex_destroyer(&operator_lock);
        return 1;
    }
	if (pthread_cond_init(&cond_psistis, NULL) != 0) {
		mutex_destroyer(&lock_psistis);
		mutex_destroyer(&lock_fournoi);
		mutex_destroyer(&lock_packet);
		mutex_destroyer(&lock_deliv);
		mutex_destroyer(&lock_screen);
		mutex_destroyer(&operator_lock);
		mutex_destroyer(&fail_lock);
        return 1;
    }
	if (pthread_cond_init(&cond_fournoi, NULL) != 0) {
			mutex_destroyer(&lock_psistis);
			mutex_destroyer(&lock_fournoi);
			mutex_destroyer(&lock_packet);
			mutex_destroyer(&lock_deliv);
			mutex_destroyer(&lock_screen);
			mutex_destroyer(&operator_lock);
			mutex_destroyer(&fail_lock);
			cond_destroyer(&cond_psistis);
			return 1;
		}
	if (pthread_cond_init(&cond_packet, NULL) != 0) {
			mutex_destroyer(&lock_psistis);
			mutex_destroyer(&lock_fournoi);
			mutex_destroyer(&lock_packet);
			mutex_destroyer(&lock_deliv);
			mutex_destroyer(&lock_screen);
			mutex_destroyer(&operator_lock);
			mutex_destroyer(&fail_lock);
			cond_destroyer(&cond_psistis);
			cond_destroyer(&cond_fournoi);
			return 1;
		}
	if (pthread_cond_init(&cond_deliv, NULL) != 0) {
			mutex_destroyer(&lock_psistis);
			mutex_destroyer(&lock_fournoi);
			mutex_destroyer(&lock_packet);
			mutex_destroyer(&lock_deliv);
			mutex_destroyer(&lock_screen);
			mutex_destroyer(&operator_lock);
			mutex_destroyer(&fail_lock);
			cond_destroyer(&cond_psistis);
			cond_destroyer(&cond_fournoi);
			cond_destroyer(&cond_packet);
			return 1;
		}
	return 0;
}
