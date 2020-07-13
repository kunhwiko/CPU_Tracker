int main(int argc, char *argv[])
{
  int port_number = 3000; 

  // generate lock and threads
  pthread_t t1;
  if (pthread_create(&t1, NULL, &get_data, NULL)) return 1;
  pthread_t t2;
  if (pthread_create(&t2, NULL, &start_server, &port_number)) return 1;
  pthread_t t3; 
  if (pthread_create(&t3, NULL, &quit_program, NULL)) return 1;
   
  // join threads
  if(pthread_join(t1,NULL) != 0) return 1;
  if(pthread_join(t2,NULL) != 0) return 1;
  if(pthread_join(t3,NULL) != 0) return 1;
  

  return 0;
}
