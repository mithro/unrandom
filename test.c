#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  int notok = 0;

  srand(1);
  int x = rand();
  srand(2);
  int y = rand();

  puts("rand(): ");
  if (x != y) {
    notok += 1;
    puts("fail\n");
  } else {
    puts("ok\n");
  }

  puts("open('/dev/urandom'): ");
  int fd1 = open("/dev/urandom", O_RDONLY);
  char bytes1;
  read(fd1, &bytes1, 1);
  if (bytes1 != '\0') {
    notok += 1;
    puts("fail\n");
  } else {
    puts("ok\n");
  }

  puts("open('test.c'): ");
  int fd2 = open("test.c", O_RDONLY);
  char bytes2;
  read(fd2, &bytes2, 1);
  if (bytes2 != '#') {
    notok += 1;
    puts("fail\n");
  } else {
    puts("ok\n");
  }

  return notok;
}
