/* userspace_test.c
 * -----------
 *  Description: Super simple userspace executable that will write and read from the
 *  device created in the cdev.ko module. Because of the implemented mutexes,
 *  the read and write functions independently open and close the file. This
 *  way, one can run the binary at the same time to test for race condition
 *  vulnerability.
 * */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int cdev_write(const char *filename) {
  char input[256];

  printf("[-] Opening %s to write...\n", filename);
  int fd = open(filename, O_TRUNC | O_WRONLY);
  
  if(fd < 0){
    perror("[!] Error: Could not open device.\n");
    exit(1);
  }

  /* Obtain user space message. */
  printf("[*] Enter text to write to %s:\n\t[*] ", filename);
  fgets(input, sizeof(input), stdin);

  /* Write to the file. */
  int ret = write(fd, input, strlen(input)); 
  
  if (ret < 0){
    perror("[!] Error: Failed to write message to device\n");
    return errno;
  }
  
  close(fd);
    
  return 0;
}

int cdev_read(const char *filename){
  char output[256];
  
  printf("[-] Opening %s to read...\n", filename);
  int fd = open(filename, O_RDONLY); 
  
  if (fd < 0){
    perror("[!] Error: Could not open device.\n");
    exit(1);
  }

  /* Read from the file  */ 
  int ret = read(fd, output, sizeof(output));
  if (ret < 0){
    perror("[!] Error: Could not read message from the device.\n");
    return errno;
  } 
  
  printf("[-] Output from %s:\n", filename);
  printf("\t[*] %s", output);
  
  close(fd);
  return 0;

}

int main(int argc, char* argv[]){

  const char* filename;

  if ( argc != 2) {
    printf("Usage: %s <device>\n", argv[0]);
    exit(-1);
  } else {
    filename = argv[1];
  } 

  printf("[-] Quick test for the cdev.ko module.\n\n");

  /* Open for writing */
  cdev_write(filename);

  /* Whenever user is ready */
  printf("\n[*] Press ENTER to read from file.\n");
  getchar();

  /* Open for reading */
  cdev_read(filename);

  return 0;
}
