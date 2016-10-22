#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

int main(int argc, char *argv[]) {
	(void)argc;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (argv[1])) != NULL) {
	  /* print all the files and directories within directory */
	  while ((ent = readdir (dir)) != NULL) {
	    printf ("%s\n", ent->d_name);
	  }
	  closedir (dir);
	} else {
	  /* could not open directory */
	  printf("Error: Directory not found\n");
	  return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}