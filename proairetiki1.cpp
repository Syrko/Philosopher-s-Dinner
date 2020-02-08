#include <iostream>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

time_t t;
struct tm *now;
struct sched_param param;
int philosopher_id=0;
bool forks[11]={false, true, true, true, true, true, true, true, true, true, true};
float eat_time[10]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int philosophers_num = 0;
pthread_mutex_t mutex;

float wait_time[10]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int wait_counter[10]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void printTime(){
  t = time(0);
  now = localtime(&t);
  std::cout<<now->tm_hour<<":"<<now->tm_min<<":"<<now->tm_sec<<std::endl;
}

void think(pthread_t thread, int id){
  std::cout<<"Philosopher "<<id<<" is THINKING at ";
  printTime();
  sleep(std::rand()%5);
  std::cout<<"Philosopher "<<id<<" is HUNGRY at ";
  printTime();
}

bool take_forks(int id){
  pthread_mutex_lock(&mutex);
  if(forks[id]==true && forks[(id%philosophers_num)+1]==true){
    forks[id]=false;
    forks[(id%philosophers_num)+1]=false;
    std::cout<<"Philosopher "<<id<<" successfully took the forks\n";
    pthread_mutex_unlock(&mutex);
    return true;
  }
  else if(forks[id]==false){
    if(id>1)
      std::cout<<"Philosopher "<<id<<" failed to take the fork because philosopher "<<id-1<<" is eating\n";
    else
      std::cout<<"Philosopher "<<id<<" failed to take the fork because philosopher "<<philosophers_num<<" is eating\n";
  }
  else if(forks[(id%philosophers_num)+1]==false){
    std::cout<<"Philosopher "<<id<<" failed to take the fork because philosopher "<<(id%philosophers_num)+1<<" is eating\n";
  }
  pthread_mutex_unlock(&mutex);
  return false;
}

void put_forks(int id, clock_t tm){
  tm=clock()-tm;
  eat_time[id-1]+=(float)tm/CLOCKS_PER_SEC;
  forks[id]=true;
  forks[(id%philosophers_num)+1]=true;
  std::cout<<"Philosopher "<<id<<" put the forks down\n";
  if(eat_time[id-1]>=20){
    std::cout<<"Philosopher "<<id<<" finished EATING and will now be THINKING\n";
    sleep(2000);
  }
}

void eat(int id){
  std::cout<<"Philosopher "<<id<<" is EATING at ";
  printTime();
}

void philosopher(int id){
  while(true){
    think(pthread_self(), id);
    clock_t wait_tm = clock();
    if(take_forks(id)){
      wait_tm = clock()-wait_tm;
      wait_time[id-1]+=(float)wait_tm/CLOCKS_PER_SEC;
      wait_counter[id-1]++;
      clock_t tm=clock();
      eat(id);
      put_forks(id, tm);
    }
  }
}

void *philosopher(void *phil_id){
  philosopher_id++;
  //Change philosopher's priority
  param.sched_priority++;
  pthread_setschedparam(pthread_self(), SCHED_RR, &param);
  philosopher(philosopher_id);
}

int main(){
  std::srand(std::time(NULL));
  do{
    std::cout<<"Input number of philosophers(3-10): ";
    std::cin>>philosophers_num;
  }while(philosophers_num<3||philosophers_num>10);



  pthread_t philosophers[philosophers_num];
  param.sched_priority = 0;
  pthread_mutex_init(&mutex, NULL);

  for(int i=0; i<philosophers_num; i++){
    pthread_create(&philosophers[i], NULL, philosopher, (void *)i);
  }

  while(true){
    for(int i=0;i<philosophers_num;i++){
      if(eat_time[i]<20)
        continue;
      else if(eat_time[philosophers_num-1]>=20){
        sleep(10);
        std::cout<<std::endl<<"------------\n";
        std::cout<<"Average waiting time: "<<std::endl;
        for(int i=0; i<philosophers_num;i++){
          std::cout<<"Philosopher "<<i+1<<" waited for an average of: ";
          std::cout<<wait_time[i]/wait_counter[i]<<std::endl;
        }
        std::cout<<"Total Average: ";
        float temp=0;
        for(int i=0;i<philosophers_num;i++){
          temp+=wait_time[i]/wait_counter[i];
        }
        std::cout<<temp/philosophers_num<<std::endl;
        std::cout<<"------------\n";
        return 0;
      }
    }
  }
}
