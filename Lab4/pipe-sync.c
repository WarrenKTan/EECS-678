/*
  pipe-sync.c: Use Pipe for Process Synchronization
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// Question: Update this program to synchronize execution of the parent and
// child processes using pipes, so that we are guaranteed the following
// sequence of print messages:
// Child line 1
// Parent line 1
// Child line 2
// Parent line 2


int main()
{
  char *s, buf[1024];
  int ret, stat;
  s  = "Use Pipe for Process Synchronization\n";

  /* create pipe */
  int p1[2], p2[2];
  pipe(p1);
  pipe(p2);

  ret = fork();
  if (ret == 0) {
    /* child process. */
    close(p1[0]);
    close(p2[1]);

    printf("Child line 1\n");
    
    // tell parent to continue execution
    write(p1[1], "_", 1);
    
    // wait for parent to send signal
    read(p2[0], &buf, 1);
    printf("Child line 2\n");
  } else {
    /* parent process */
    close(p1[1]);
    close(p2[0]);
    
    // wait for child to send signal
    read(p1[0], &buf, 1);
    printf("Parent line 1\n");
    
    // tell child to continue execution
    write(p2[1], "_", 1);

    // wait for child to finish execution
    wait(&stat);
    printf("Parent line 2\n");
  }
}
