/**********************************************************/
/*                    2nd project IOS                     */
/*                      Michal Sova                       */
/*                    Login: xsovam00                     */
/*                      April 2019                        */
/**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h>
#include "barrier.h"

#define shmSIZE sizeof(int)
#define shmCOUNTER "/xsovam00-shmCOUNTER"
#define shmPORT "/xsovam00-shmPORT"
#define shmSERFS "/xsovam00-shmSERFS"
#define shmHACKS "/xsovam00-shmHACKS"
#define shmAVIABLESERFS "/xsovam00-shmAVIABLESERFS"
#define shmAVIABLEHACKS "/xsovam00-shmAVIABLEHACKS"

#define semMUTEX "/xsovam00-semMUTEX"
#define semHACKS "/xsovam00-semHACKS"
#define semSERFS "/xsovam00-semSERFS"
#define semCAPTAINRELEASE "/xsovam00-semCAPTAINRELEASE"
#define semCAPTAINBOARD "/xsovam00-CAPTAINBOARD"

int P, H ,S, R, W, C;
FILE *proj2out;

//initialize shared memory objects
void init_shms(){
    int *counter;
    int shmID;
	shmID = shm_open(shmCOUNTER, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmID, shmSIZE);
    counter = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE , MAP_SHARED, shmID, 0);
    close(shmID);
    *counter = 0;
    munmap(counter, shmSIZE);
	
	int *serfs;
	shmID = shm_open(shmSERFS, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmID, shmSIZE);
    serfs = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
	*serfs = 0;
	munmap(serfs, shmSIZE);

	int *hacks;
    shmID = shm_open(shmHACKS, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmID, shmSIZE);
    hacks = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
    *hacks = 0;
    munmap(hacks, shmSIZE);

    int *port;
    shmID = shm_open(shmPORT, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmID, shmSIZE);
    port = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
    *port = 0;
    munmap(port, shmSIZE);

    int *aviablehacks;
    shmID = shm_open(shmAVIABLEHACKS, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmID, shmSIZE);
    aviablehacks = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
    *aviablehacks = 0;
    munmap(aviablehacks, shmSIZE);

    int *aviableserfs;
    shmID = shm_open(shmAVIABLESERFS, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmID, shmSIZE);
    aviableserfs = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);
    *aviableserfs = 0;
    munmap(aviableserfs, shmSIZE);
}

void init_semaphores(){
	sem_t *mutex;
    mutex = sem_open(semMUTEX, O_CREAT, 0644, 1);
    sem_close(mutex);

    sem_t *hacksQueue;
    hacksQueue = sem_open(semHACKS, O_CREAT, 0644, 0);
    sem_close(hacksQueue);

    sem_t *serfsQueue;
    serfsQueue = sem_open(semSERFS, O_CREAT, 0644, 0);
    sem_close(serfsQueue);

    sem_t *captainRelease;
    captainRelease = sem_open(semCAPTAINRELEASE, O_CREAT, 0644, 0);
    sem_close(captainRelease);

    sem_t *captain_board;
		captain_board = sem_open(semCAPTAINBOARD, O_CREAT, 0644, 1);
		sem_close(captain_board);
	}

	void unlink_all() {
		shm_unlink(shmCOUNTER);
		shm_unlink(shmHACKS);
		shm_unlink(shmSERFS);
		shm_unlink(shmPORT);
		shm_unlink(shmAVIABLEHACKS);
		shm_unlink(shmAVIABLESERFS);
		sem_unlink(semMUTEX);
		sem_unlink(semHACKS);
		sem_unlink(semSERFS);
		sem_unlink(semCAPTAINRELEASE);
		sem_unlink(semCAPTAINBOARD);
		b_unlink();
	}

	//returns unsigned int or -1 in case of failure
	int get_number(char *content){
		int number = 0;
		int j = 0;
		while(content[j] != '\0'){
			if(content[j] >= '0' && content[j] <= '9'){
				number = number * 10 + content[j] - '0';
				j++;
			}
			else{
				return -1;
			}
		}
		return number;
	}

	int get_conditions(int argc, char **argv){
		if(argc < 7){
			fprintf(stderr, "need more arguments\n");
			fprintf(stderr, "./proj2 P H S R W C\n");
			return 1;
		}
		P = get_number(argv[1]);
		if(P < 2 || (P % 2) != 0){
			fprintf(stderr, "wrong P = %s\n", argv[1]);
			fprintf(stderr, "P >= 2 && (P %% 2) == 0\n");
			return 1;
		}
		H = get_number(argv[2]);
		if(H < 0 || H > 2000){
			fprintf(stderr, "wrong H = %s\n", argv[2]);
			fprintf(stderr, "H >= 0 && H <= 2000\n");
			return 1;
		}
		S = get_number(argv[3]);
		if(S < 0 || S > 2000){
			fprintf(stderr, "wrong S = %s\n", argv[3]);
			fprintf(stderr, "S >= 0 && S <= 2000\n");
			return 1;
		}
		R = get_number(argv[4]);
		if(R < 0 || R > 2000){
			fprintf(stderr, "wrong R = %s\n", argv[4]);
			fprintf(stderr, "R >= 0 && R <= 2000\n");
			return 1;
		}
		W = get_number(argv[5]);
		if(W < 20 || W > 2000){
			fprintf(stderr, "wrong W = %s\n", argv[5]);
			fprintf(stderr, "W >= 20 && W <= 2000\n");
			return 1;
		}
		C = get_number(argv[6]);
		if(C < 5){
			fprintf(stderr, "wrong C = %s\n", argv[6]);
			fprintf(stderr, "C >= 5\n");
			return 1;
		}
		return 0;
	}

	int *open_COUNTER(){
		int shmID;
		int *shm;
		shmID = shm_open(shmCOUNTER, O_RDWR, S_IRUSR | S_IWUSR);
		shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
		close(shmID);
		return shm;
	}

	int *open_HACKS(){
		int shmID;
		int *shm;
		shmID = shm_open(shmHACKS, O_RDWR, S_IRUSR | S_IWUSR);
		shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
		close(shmID);
		return shm;
	}

	int *open_SERFS(){
		int shmID;
		int *shm;
		shmID = shm_open(shmSERFS, O_RDWR, S_IRUSR | S_IWUSR);
		shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
		close(shmID);
		return shm;
	}

	int *open_PORT(){
		int shmID;
		int *shm;
		shmID = shm_open(shmPORT, O_RDWR, S_IRUSR | S_IWUSR);
		shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
		close(shmID);
		return shm;
	}

	int *open_AVIABLEHACKS(){
		int shmID;
		int *shm;
		shmID = shm_open(shmAVIABLEHACKS, O_RDWR, S_IRUSR | S_IWUSR);
		shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
		close(shmID);
		return shm;
	}

	int *open_AVIABLESERFS(){
		int shmID;
		int *shm;
		shmID = shm_open(shmAVIABLESERFS, O_RDWR, S_IRUSR | S_IWUSR);
		shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
		close(shmID);
		return shm;
	}

	void serf(int num){
		//opens all shared memory objects, semaphores and barrier
		sem_t *mutex;
		sem_t *serfsQueue;
		sem_t *hacksQueue;
		sem_t *captainRelease;
		sem_t *captain_board;
		mutex = sem_open(semMUTEX, O_RDWR);
		serfsQueue = sem_open(semSERFS, O_RDWR);
		hacksQueue = sem_open(semHACKS, O_RDWR);
		captainRelease = sem_open(semCAPTAINRELEASE, O_RDWR);
		captain_board = sem_open(semCAPTAINBOARD, O_RDWR);
		int *counter;
		int *hacks;
		int *serfs;
		int *port;
		int *aviablehacks;
		int *aviableserfs;
		counter = open_COUNTER();
		hacks = open_HACKS();
		serfs = open_SERFS();
		port = open_PORT();
		aviablehacks = open_AVIABLEHACKS();
		aviableserfs = open_AVIABLESERFS();
		int wait = 0;
		int isCaptain = 0;
		barrier_t board;
		b_open(&board, 4);

		//begins process
		sem_wait(mutex);
		*counter = *counter + 1;
		fprintf(proj2out, "%d : SERF %d : starts\n", *counter, num);
		sem_post(mutex);

		//if port is full -> leaves queue and wait
		sem_wait(mutex);
		if(*port >= C){
			wait = 1;
		}
		else {
			wait = 0;
			*port = *port + 1;
			*counter = *counter + 1;
			*serfs = *serfs + 1;
			*aviableserfs = *aviableserfs + 1;
			fprintf(proj2out, "%d : SERF %d : waits : %d : %d\n", *counter, num, *hacks, *serfs);
		}
	while(wait){
        *counter = *counter + 1;
        fprintf(proj2out, "%d : SERF %d : leaves queue : %d : %d\n", *counter, num, *hacks, *serfs);
        sem_post(mutex);

		int Random = rand() % (W + 1 - 20);
        Random += 20;        
		usleep(W * 1000);
		
		//if port is not full join the queue
		sem_wait(mutex);
	   	*counter = *counter + 1;
    	fprintf(proj2out, "%d : SERF %d : is back\n", *counter, num);
	   	if(*port >= C){
        	wait = 1;
	   	}
    	else {
			wait = 0;
       		*port = *port + 1;
			*counter = *counter + 1;
    		*serfs = *serfs + 1;
			*aviableserfs = *aviableserfs + 1;
	   		fprintf(proj2out, "%d : SERF %d : waits : %d : %d\n", *counter, num, *hacks, *serfs);
    	}
	}

	//when there are enough processes on boat this process becomes captain
	//and waits until another captain exits
	//if there is not enough processes, member waits on captain
	if (*aviableserfs == 4){
		*aviableserfs = *aviableserfs - 4;
		isCaptain = 1;
		sem_post(mutex);
		
		//waits for another captain to finish
		sem_wait(captain_board);
		sem_wait(mutex);
		sem_post(serfsQueue);
		sem_post(serfsQueue);
		sem_post(serfsQueue);
		*counter = *counter + 1;
		*serfs = *serfs - 4;
		fprintf(proj2out, "%d : SERF %d : boards : %d : %d\n", *counter, num, *hacks, *serfs);
		*port = *port - 4;
	}
	else if(*aviableserfs == 2 && *aviablehacks >= 2){
		*aviableserfs = *aviableserfs - 2;
		*aviablehacks = *aviablehacks - 2;
		isCaptain = 1;
		sem_post(mutex);

		sem_wait(captain_board);
		sem_wait(mutex);
        sem_post(serfsQueue);
		sem_post(hacksQueue);
		sem_post(hacksQueue);
		*counter = *counter + 1;
        *hacks = *hacks - 2;
        *serfs = *serfs - 2;
        fprintf(proj2out, "%d : SERF %d : boards : %d : %d\n", *counter, num, *hacks, *serfs);
        *port = *port - 4;
	}
	else{
		sem_post(mutex);
		sem_wait(serfsQueue);
	}

	b_join(&board);
    if(isCaptain){
        sem_post(mutex);
    }

	usleep(R * 1000);

	if(isCaptain){
		//waits to other processes until they exits
		sem_wait(captainRelease);
		sem_wait(captainRelease);
		sem_wait(captainRelease);
		sem_wait(mutex);
		*counter = *counter + 1;
		fprintf(proj2out, "%d : SERF %d : captain exits : %d : %d\n", *counter, num, *hacks, *serfs);
		//another captain can enter
		sem_post(captain_board);
	}
	else{
		sem_wait(mutex);
		*counter = *counter + 1;
        fprintf(proj2out, "%d : SERF %d : member exits : %d : %d\n", *counter, num, *hacks, *serfs);
		sem_post(captainRelease);
	}
	sem_post(mutex);

	//close everything(semaphores, shms and barrier)
	b_close(&board);
	munmap(counter, shmSIZE);
	munmap(hacks, shmSIZE);
	munmap(serfs, shmSIZE);
	munmap(port, shmSIZE);
	munmap(aviablehacks, shmSIZE);
    munmap(aviableserfs, shmSIZE);
	sem_close(mutex);
    sem_close(hacksQueue);
    sem_close(serfsQueue);
	sem_close(captainRelease);
	sem_close(captain_board);
}

//similar to function(process) serf()
void hacker(int num){
    sem_t *mutex;
	sem_t *hacksQueue;
	sem_t *serfsQueue;
	sem_t *captainRelease;
	sem_t *captain_board;
    mutex = sem_open(semMUTEX, O_RDWR);
    hacksQueue = sem_open(semHACKS, O_RDWR);
    serfsQueue = sem_open(semSERFS, O_RDWR);
	captainRelease = sem_open(semCAPTAINRELEASE, O_RDWR);
	captain_board = sem_open(semCAPTAINBOARD, O_RDWR);
    int *counter;
    int *hacks;
    int *serfs;
    int *port;
	int *aviableserfs;
	int *aviablehacks;
    counter = open_COUNTER();
    hacks = open_HACKS();
    serfs = open_SERFS();
    port = open_PORT();
	aviablehacks = open_AVIABLEHACKS();
	aviableserfs = open_AVIABLESERFS();
    int wait = 0;
	int isCaptain = 0;
    barrier_t board;
    b_open(&board, 4);

    sem_wait(mutex);
    *counter = *counter + 1;
    fprintf(proj2out, "%d : HACK %d : starts\n", *counter, num);
	sem_post(mutex);

	sem_wait(mutex);
    if(*port >= C){
        wait = 1;
    }
    else {
        wait = 0;
        *port = *port + 1;
		*counter = *counter + 1;
    	*hacks = *hacks + 1;
		*aviablehacks = *aviablehacks + 1;
	    fprintf(proj2out, "%d : HACK %d : waits : %d : %d\n", *counter, num, *hacks, *serfs);
    }

    while(wait){
        *counter = *counter + 1;
        fprintf(proj2out, "%d : HACK %d : leaves queue : %d : %d\n", *counter, num, *hacks, *serfs);
        sem_post(mutex);
		
		int Random = rand() % (W + 1 - 20);
		Random += 20;
        usleep(W * 1000);

        sem_wait(mutex);
        *counter = *counter + 1;
        fprintf(proj2out, "%d : HACK %d : is back\n", *counter, num);
        if(*port >= C){
            wait = 1;
        }
        else {
            wait = 0;
            *port = *port + 1;
			*counter = *counter + 1;
    		*hacks = *hacks + 1;
			*aviablehacks = *aviablehacks + 1;
	    	fprintf(proj2out, "%d : HACK %d : waits : %d : %d\n", *counter, num, *hacks, *serfs);
        }
    }
    if (*aviablehacks == 4){
        isCaptain = 1;
		*aviablehacks = *aviablehacks - 4;
		sem_post(mutex);

		sem_wait(captain_board);
		sem_wait(mutex);
        sem_post(hacksQueue);
        sem_post(hacksQueue);
        sem_post(hacksQueue);
        *counter = *counter + 1;
        *hacks = *hacks - 4;
        fprintf(proj2out, "%d : HACK %d : boards : %d : %d\n", *counter, num, *hacks, *serfs);
		*port = *port - 4;
    }
    else if(*aviablehacks == 2 && *aviableserfs >= 2){
		*aviablehacks = *aviablehacks - 2;
		*aviableserfs = *aviableserfs - 2;
        isCaptain = 1;
		sem_post(mutex);

		sem_wait(captain_board);
		sem_wait(mutex);
        sem_post(serfsQueue);
        sem_post(serfsQueue);
        sem_post(hacksQueue);
		*counter = *counter + 1;
        *hacks = *hacks - 2;
		*serfs = *serfs - 2;
        fprintf(proj2out, "%d : HACK %d : boards : %d : %d\n", *counter, num, *hacks, *serfs);
		*port = *port - 4;
    }
    else{
        sem_post(mutex);
		sem_wait(hacksQueue);
    }

	b_join(&board);
    if(isCaptain){
		sem_post(mutex);
    }

    usleep(R * 1000);
    if(isCaptain){
		sem_wait(captainRelease);
		sem_wait(captainRelease);
		sem_wait(captainRelease);
		sem_wait(mutex);
		*counter = *counter + 1;
        fprintf(proj2out, "%d : HACK %d : captain exits : %d : %d\n", *counter, num, *hacks, *serfs);
		sem_post(captain_board);
    }
    else{
		sem_wait(mutex);
		*counter = *counter + 1;
        fprintf(proj2out, "%d : HACK %d : member exits : %d : %d\n", *counter, num, *hacks, *serfs);
		sem_post(captainRelease);
    }
	sem_post(mutex);

	b_close(&board);
    munmap(counter, shmSIZE);
    munmap(hacks, shmSIZE);
    munmap(serfs, shmSIZE);
	munmap(port, shmSIZE);
	munmap(aviableserfs, shmSIZE);
	munmap(aviablehacks, shmSIZE);
    sem_close(mutex);
    sem_close(hacksQueue);
    sem_close(serfsQueue);
	sem_close(captainRelease);
	sem_close(captain_board);
}

void gen_serfs(){
	pid_t PID[P];
    int pid;
	//generate P processes
    for(int i = 0; i < P; i++){
        if ((pid = fork()) < 0){
            perror("fork");
			unlink_all();
            exit(2);
        }
		//child
        if (pid == 0) {
            serf(i+1);
            exit(0);
        }
		else {
			PID[i] = pid;
		}
		usleep(S * 1000);
    }
	//waits until all serfs finishes
    for (int i = 0; i < P; i++) {
        waitpid(PID[i], NULL, 0);
    }
}

void gen_hackers(){
	pid_t PID[P];
	int pid;
	//generate P processes
	for(int i = 0; i < P; i++){
        if ((pid = fork()) < 0){
            perror("fork");
			unlink_all();
            exit(2);
        }
		//child
        if (pid == 0) {
            hacker(i+1);
            exit(0);
        }
		else {
			PID[i] = pid;
		}
		usleep(H * 1000);
    }
	//waiting until all hacks finishes
    for (int i = 0; i < P; i++) {
        waitpid(PID[i], NULL, 0);
    }
}

int main(int argc, char *argv[]){
	if(get_conditions(argc, argv)){
		fprintf(stderr, "bad conditions\n");
		return 1;
	}
	proj2out = fopen("proj2.out", "w+");
	if(proj2out == NULL){
		fprintf(stderr, "Failed to create/open file 'proj2.out'\n");
		return 2;
	}
	int pid;
	pid_t genPID[2];
	setbuf(proj2out,NULL);
    setbuf(stderr,NULL);
	
	//initialize shared memory objects and semaphores (and close them)
    init_shms();
	init_semaphores();

	//initialize barrier
	barrier_t board;
    b_init(&board);

	// system call - check if success
    if ((pid = fork()) < 0) {
        perror("fork");
		unlink_all();
        exit(2);
    }
    if (pid == 0) { // child gen_hackers
        gen_hackers();
        exit(0);
    } 
	else { // parent
        genPID[0] = pid;
        if ((pid = fork()) < 0) {
        	perror("fork");
			unlink_all();
    	    exit(2);
	    }

        if (pid == 0) { // child gen_serfs
            gen_serfs();
            exit(0);
        } 
		else { // parent
            genPID[1] = pid;
        }
    }

    //wait for all processes to end
    waitpid(genPID[0], NULL, 0);
    waitpid(genPID[1], NULL, 0);

	//clean all shms and sems
	unlink_all();
	fclose(proj2out);
	return 0;
}
