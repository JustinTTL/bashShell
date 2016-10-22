#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
void print_dir(char *dirpath);

int main(int argc, char *argv[]) {
	if (argc == 1) {
		print_dir("./");
	} else if (argc > 2){
		printf("ERROR: Too many arguments\n");
	} else {
		print_dir(argv[1]);
	}

	return EXIT_SUCCESS;
}

void print_dir(char *dirpath){
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (dirpath)) != NULL) {
		  /* print all the files and directories within directory */
		  while ((ent = readdir (dir)) != NULL) {
		    printf ("%s\n", ent->d_name);
		  }
		  closedir (dir);
		} else {
		  /* could not open directory */
		  printf("Error: Directory not found\n");
		}

}