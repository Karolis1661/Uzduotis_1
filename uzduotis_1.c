/*
    * Program reads user given commands -f and -t with their arguments
    * Tree walks from given -f directory to every children directory
    * Saves all found file addresses of each directory
    * And checks every file for a given -t input
    * Saves and prints out all results
    * 
    * Example ----> ./Program -f . -t Hello
*/

#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define ARR_INIT_VALUE 20 // Initialize and reallocation value
#define MAX_LINE_SIZE 250
#define FILE_CHECKING_LIMIT 1000            
#define EXTRA_BYTES 20

struct files_log {
    int arr_size;
    int counter;
    char **list;
};

// For ftw library
static struct files_log Files = {.arr_size = ARR_INIT_VALUE, .counter = 0, .list = NULL};

struct data_log {
    int added_res;
    int res_size;
    char *f_query;
    char *t_query;
    char **res;
};

// getopt library, read and save cmd arguments 
int save_cmd_args(int argc, char **argv, char **const search_path, char **const text_query);

// Nftw, traverse thru directories and save filepaths
void save_file_list(char **const search_path);

// Read from file and save res
void save_results(struct data_log *const data_set);

// Helper functions
struct data_log *init_struct(void);
char *mal_str(size_t size);
char **mal_str_arr(size_t size);
char **re_str_arr(char ***arr, int *const new_size);
void print_results(const struct data_log *data_set);


int main(int argc, char **argv) {
    
    struct data_log *const user_data = init_struct();

    save_cmd_args(argc, argv, &user_data->f_query, &user_data->t_query);

    save_file_list(&user_data->f_query);
    
    save_results(user_data);

    print_results(user_data);

    return 0;
}


struct data_log *init_struct(void) {

    struct data_log *init_struct = malloc(sizeof(struct data_log));

    if (init_struct == NULL) {
        puts("Malloc at init_struct() failed");
        exit(EXIT_FAILURE);
    }

    init_struct->res_size = ARR_INIT_VALUE;
    init_struct->added_res = 0;
    init_struct->f_query = NULL;
    init_struct->t_query = NULL;
    init_struct->res = NULL;

    return init_struct;
}

int save_cmd_args(int argc, char **argv, char **const search_path, char **const text_query) {
    
    /*
        * reads command line arguments and checks with getopt library if they are valid (-f [args], -t [args])
        * if arguments are valid, stores them into provided char arrays (passed by reference)
        * if arguments are not valid - program terminates
    */

    void print_help(void);

    int option;
    int f_flag = 0;
    int t_flag = 0;
    
    // Check if there's more arguments than programs name
    if (argc > 1) 
    {
        // Runs for each given command (-f, -t)
        while ((option = getopt(argc, argv, "f:t:")) != -1)
        {
            switch (option) {
                case 'f':
                    f_flag = 1;
                    *search_path = mal_str(strlen(optarg) + 1);
                    strcpy(*search_path, optarg);
                    break;

                case 't':
                    t_flag = 1; 
                    *text_query = mal_str(strlen(optarg) + 1);
                    strcpy(*text_query, optarg);
                    break;

                default:
                    break;
            }
        }
    }
   
    // If only -f [args] -t [args] was provided and no less or more
    if (f_flag == 1 && t_flag == 1 && argc == 5) {
        return 0;
    }
    
    print_help();
    exit(EXIT_FAILURE);
}

void save_file_list(char **const search_path) {
    /*
        * Initialize memory for files list
        * Using ftw library traverse thru all directories from a given point
        * Save all found file addresses
    */

    int save_f_paths(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);

    Files.list = mal_str_arr(ARR_INIT_VALUE);

    if (nftw(*search_path, save_f_paths, 20, 0) == -1) {
        // If search_path directory wasn't found
        perror("nftw");
        exit(EXIT_FAILURE);
    }

    // Set unused pointers to NULL
    for (size_t i = Files.counter; i < Files.arr_size; i++) {
        Files.list[i] = NULL;
    }

}

void save_results(struct data_log *const data_set) {
    /*  
        * Initializes memory for results array
        * Performs file checking operation for each file from the list
    */

    int check_file_context(const char *file_path, struct data_log *const data_set);

    data_set->res = mal_str_arr(ARR_INIT_VALUE);

    for (size_t i = 0; i < Files.counter; i++) {
        check_file_context(Files.list[i], data_set);

        // Clean up
        free(Files.list[i]);
        Files.list[i] = NULL; 
    }
}

void print_results(const struct data_log *data_set) {

    printf("**** Starting from directory: %s\n", data_set->f_query);
    printf("**** Searching for text: %s\n", data_set->t_query);
    printf("**** Files to check: %d\n", Files.counter);

    printf("**** %d matches found \n", data_set->added_res);
    //printf("**** final size of results array: %d\n", data_set->res_size);

    if (data_set->added_res > 0) 
    {
        puts("**** RESULTS:");
        for (size_t i = 0; i < data_set->added_res; i++) {
            printf("%s \n", data_set->res[i]);
        }
    }

    puts("\n**** DONE, EXITING PROGRAM");
}

char **mal_str_arr(size_t size) {
    
    char **arr = malloc(size * sizeof(char *));
    if (arr == NULL) {
        puts("Memmory allocation at mal_str_arr() failed");
        exit(EXIT_FAILURE);
    }

    return arr;
}

char *mal_str(size_t size) {

    char *ptr = malloc(size * sizeof(char));

    if (ptr == NULL) {
        puts("Memory allocation at mal_str() failed!");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

char **re_str_arr(char ***arr, int * const curr_size) {

    // Dereference pointer and update its size for reallocation
    *curr_size += ARR_INIT_VALUE;
    
    char **tmp = realloc( (*arr), (*curr_size) * sizeof(char *) );

    if (tmp == NULL) {
        printf("Memmory reallocation at re_str_arr() failed!");
        exit(EXIT_FAILURE);
    }

    return tmp;
}


void print_help(void) {
    puts("Example: -d directory -t text");
    puts("When using arguments with spaces or longer than 1 word use quotes, example:");
    puts("-d \"directory path with spaces\" -t \"longer text query\"");
}

int save_f_paths(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    /*
        * Default nftw library function customized only to look at files that fits FTW_F flag requirements
        * Function cannot accept extra custom arguments, so have to use static variables instead to save results
    */
   
    if (Files.counter >= FILE_CHECKING_LIMIT) {
        puts("REACHED FILE CHECKING LIMIT");
        return 1;
    }
    // check if its a regular file
    if (tflag == FTW_F) 
    {
        // check if its not a hidden file
        if ((fpath + ftwbuf->base)[0] != '.')
        {
            int pos = Files.counter;
            ++Files.counter;

            // Reallocate memory if needed
            if (Files.counter >= Files.arr_size) {
                Files.list = re_str_arr(&Files.list, &Files.arr_size);
            }

            // Allocate memory, copy res to file list
            Files.list[pos] = mal_str(strlen(fpath) + 1); // + 1 for null-terminated string
            strcpy(Files.list[pos], fpath);
        }
    }
    return 0;
}

int check_file_context(const char *file_path, struct data_log *const data_set) {
    /*
        * Opens each file from Files.list, compares given input with each line from the file
        * Save all found matches to struct data_log struct
    */

    int line = 0;
    char *casted_line;
    int pos = 0;
    int len = 0;
    FILE *fp;
    char tmp[MAX_LINE_SIZE];
    
    fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("Cannot open file!\n");
        return -1;
    }

    while (fgets(tmp, MAX_LINE_SIZE, fp) != NULL) 
    {
        ++line;
        pos = data_set->added_res;

        // Remove newline char from the end if reached
        tmp[strcspn(tmp, "\n")] = 0;

        if (strstr(tmp, data_set->t_query)) 
        {
            ++data_set->added_res;

            // Reallocate memory if needed 
            if (data_set->added_res >= data_set->res_size) {
                data_set->res = re_str_arr(&data_set->res, &data_set->res_size);
            }

            // Calculate length of string when converted from int
            // Ex. int line = 123; -> string line = "123" -> len = 3;
            len = snprintf(NULL, 0, "%d", line);

            // Malloc memory to hold converted int numbers as char symbols
            casted_line = malloc((len + 1) * sizeof(char));
            
            // Convert int to char 123 -> "123" and push to char* casted_line
            snprintf(casted_line, (len + 1), "%d", line);
            
            // By my calculations +11 extra bytes needed, but just in case added more
            data_set->res[pos] = mal_str(strlen(file_path) + strlen(casted_line) + EXTRA_BYTES);  
            
            strcpy(data_set->res[pos], file_path);
            strcat(data_set->res[pos], " at line: ");
            strcat(data_set->res[pos], casted_line);
            
            free(casted_line);
        }
    }

    fclose(fp);
    return 0;
}