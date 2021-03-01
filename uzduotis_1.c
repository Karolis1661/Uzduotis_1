#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define ARR_INIT_VALUE 10
#define MAX_LINE_SIZE 250
#define FILE_CHECKING_LIMIT 1000            


static int arr_size = ARR_INIT_VALUE;       // 
static char** file_list;                           // For nftw library, because can't pass any extra arguments to its functions
static int files_found = 0;                     //   

struct DATA {
    int added_results;
    int results_size;
    char *d_path;
    char *t_query;
    char **results;
};


// getopt library, read and save cmd arguments 
void print_help(void);
int save_cmd_args(int, char* [], struct DATA* d_search);

// Nftw library, traverse thru directories and save filepaths
int save_f_paths(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);

// Read from file
int check_file_context(char *file_path, struct DATA *ptr_io_data);


int main(int argc, char* argv[]) {

    int args = 0;

    struct DATA *ptr_io_data, io_data; 
    ptr_io_data = &io_data;
    io_data.results_size = ARR_INIT_VALUE;
    io_data.added_results = 0;


    // init file_list
    file_list = malloc(ARR_INIT_VALUE * sizeof(char *));
    if(file_list == NULL) {
        printf("Memmory allocation failure");
        return -1;
    }

    args = save_cmd_args(argc, argv, &io_data);

    if(args) {

        printf("**** Starting from directory: %s\n", io_data.d_path);
        printf("**** Searching for text: %s\n", io_data.t_query);

        if(nftw(io_data.d_path, save_f_paths, 20, 0) == -1) {
            perror("nftw");
            return -1;
            //exit(EXIT_FAILURE);
        }

        // Disable unused pointers
        for(int i = files_found; i < arr_size; i++) {
            file_list[i] = NULL;
        }

        // init results array
        io_data.results = malloc(ARR_INIT_VALUE * sizeof(char *));
        if(io_data.results == NULL) {
            printf("Memmory allocation failure");
            return -1;
        }

        /*for(int i = 0; i < files_found; i++) {
            printf("files to check: %s \n", file_list[i]);
        }*/

        printf("**** Files to check: %d\n", files_found);

        for(int i = 0; i < files_found; i++) {
            check_file_context(file_list[i], ptr_io_data);
            file_list[i] = NULL;
        }
        
    }

    printf("**** %d matches found \n", io_data.added_results);
    printf("**** final size of results array: %d\n", io_data.results_size);
    printf("RESULTS: \n");
    for(int i = 0; i < io_data.added_results; i++) {
        printf("%s \n", io_data.results[i]);
    }
    
    
    printf("\n");
    return 0;
}


int save_cmd_args(int argc, char* argv[], struct DATA* ptr_io_data) {
    /*
        // Read arguments, validate and store to DATA struct.
    */

    int option;
    int dir_flag = 0;
    int text_flag = 0;
    

    if (argc > 1) {
        while ((option = getopt(argc, argv, "d:t:")) != -1) {
            switch (option) {
                case 'd':
                    dir_flag = 1;
                    ptr_io_data->d_path = malloc(strlen(optarg) * sizeof(char));
                    strcpy(ptr_io_data->d_path, optarg);
                    break;
                case 't':
                    text_flag = 1;  
                    ptr_io_data->t_query = malloc(strlen(optarg) * sizeof(char));
                    strcpy(ptr_io_data->t_query, optarg);
                    break;
                default:
                    break;
            }
        }
    }
   

    if(dir_flag == 1 && text_flag == 1 && argc == 5) {
        return true;
    }
    else {
        printf("Bad input\n");
        print_help();
    }
    
    
    if(dir_flag == 1){
        free(ptr_io_data->d_path);
    }
    if(text_flag == 1){
        free(ptr_io_data->t_query);
    }
    return false;
}


void print_help(void) {
    printf("Example: -d directory -t text\n");
    printf("When using arguments with spaces or longer than 1 word use quotes, example: \n");
    printf("-d \"directory path with spaces\" -t \"longer text query\"\n");
}


int save_f_paths(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    /*  
        // Save all files full path to static file_list
    */

    if(tflag == FTW_F) {
        if((fpath + ftwbuf->base)[0] != '.') {    // Skip files which starts with '.'
            files_found++;

            // Array reached space limits, reallocate more.
            if(files_found >= arr_size) {
                arr_size += arr_size;
                file_list = realloc(file_list, arr_size * sizeof(char *));
                
                if(file_list == NULL) {
                    printf("Memmory reallocation fail\n");
                    exit(EXIT_FAILURE);
                }
            }

            file_list[files_found - 1] = malloc((strlen(fpath) + 1) * sizeof(char));
            if(file_list[files_found-1] == NULL) {
                printf("Memmory allocation fail\n");
                exit(EXIT_FAILURE);
            }

            strcpy(file_list[files_found - 1], fpath);
            
        }
    }
    return 0;
}


int check_file_context(char* file_path, struct DATA *ptr_io_data) {
    /*
        // Open each file from file_list, check if line is there's a match with a given input
        // Save results to DATA struct
        // Free memory from file_list
    */

    char *str_line;
    int len = 0;
    int line = 0;
    FILE *fp;
    char tmp[MAX_LINE_SIZE];
 
    if(ptr_io_data->results_size > FILE_CHECKING_LIMIT) {
        printf("Reached file checking limit\n");
        return 0;
    }
    
    fp = fopen(file_path, "r");
    if(fp == NULL) {
        printf("Cannot open file!\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(tmp, MAX_LINE_SIZE, fp) != NULL) {
        int len = 0;
        char *str_line;
        line++;
        tmp[strcspn(tmp, "\n")] = 0;


        if(strstr(tmp, ptr_io_data->t_query)) {
            
            ptr_io_data->added_results += 1;
            //printf("String matched at line: %d, total results: %d\n", line, ptr_io_data->added_results);

            
            if(ptr_io_data->added_results >= ptr_io_data->results_size) {
                ptr_io_data->results_size += 10;
                ptr_io_data->results = realloc(ptr_io_data->results, ptr_io_data->results_size * sizeof(char *));

                if(ptr_io_data->results == NULL) {
                    printf("Memmory allocation failure");
                    return -1;
                }
            }

            // Cast int line to char *str_line
            len = snprintf(NULL, 0, "%d", line);
            str_line = malloc((len + 1) * sizeof(char));
            snprintf(str_line, len + 1, "%d", line);


            
            ptr_io_data->results[ptr_io_data->added_results - 1] = malloc((strlen(file_path) + strlen(str_line) + 20) * sizeof(char));
            if(ptr_io_data->results[ptr_io_data->added_results - 1] == NULL) {
                printf("Memory allocation failure");
                return -1;
            }
            
            strcpy(ptr_io_data->results[ptr_io_data->added_results - 1], file_path);
            strcat(ptr_io_data->results[ptr_io_data->added_results - 1], " at line: ");
            strcat(ptr_io_data->results[ptr_io_data->added_results - 1], str_line);

            free(str_line);
            str_line = NULL;
        }

        
    }


    fclose(fp);
    free(file_path);

    return 0;

}