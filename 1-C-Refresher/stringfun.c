#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
int  reverse_string(char *, int, int);
int  print_words(char *, int, int);

int setup_buff(char *buff, char *user_str, int len) {
    //TODO: #4:  Implement the setup buff as per the directions
    if (buff == NULL || user_str == NULL) return -2;  // Invalid input
    
    int user_idx = 0;
    int buff_idx = 0;
    int last_was_space = 1;  // Start true to handle leading spaces
    
    // Process input string
    while (*(user_str + user_idx) != '\0') {
        char current = *(user_str + user_idx);
        
        // Skip if multiple whitespace or tab
        if ((current == ' ' || current == '\t')) {
            if (!last_was_space) {
                if (buff_idx >= len) return -1;  // String too long
                *(buff + buff_idx) = ' ';
                buff_idx++;
                last_was_space = 1;
            }
        } else {
            if (buff_idx >= len) return -1;  // String too long
            *(buff + buff_idx) = current;
            buff_idx++;
            last_was_space = 0;
        }
        user_idx++;
    }
    
    // Remove trailing space if exists
    if (buff_idx > 0 && *(buff + buff_idx - 1) == ' ') {
        buff_idx--;
    }
    
    // Fill remainder with dots
    for (int i = buff_idx; i < len; i++) {
        *(buff + i) = '.';
    }
    
    return buff_idx;  // Return length of processed string
}

void print_buff(char *buff, int len) {
    printf("Buffer:  [");
    for (int i = 0; i < len; i++) {
        putchar(*(buff + i));
    }
    printf("]\n");
}

void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int len, int str_len) {
    if (buff == NULL || str_len <= 0) return -1;
    
    int count = 0;
    int in_word = 0;
    
    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) == ' ') {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            count++;
        }
    }
    
    return count;
}

int reverse_string(char *buff, int len, int str_len) {
    if (buff == NULL || str_len <= 0) return -1;
    
    char temp;
    for (int i = 0; i < str_len / 2; i++) {
        temp = *(buff + i);
        *(buff + i) = *(buff + str_len - 1 - i);
        *(buff + str_len - 1 - i) = temp;
    }
    
    return str_len;
}

int print_words(char *buff, int len, int str_len) {
    if (buff == NULL || str_len <= 0) return -1;
    
    printf("Word Print\n");
    printf("----------\n");
    
    int word_count = 0;
    int word_start = 0;
    int in_word = 0;
    
    for (int i = 0; i <= str_len; i++) {
        if (i == str_len || *(buff + i) == ' ') {
            if (in_word) {
                word_count++;
                int word_len = i - word_start;
                printf("%d. ", word_count);
                for (int j = word_start; j < i; j++) {
                    putchar(*(buff + j));
                }
                printf("(%d)\n", word_len);
                in_word = 0;
            }
        } else if (!in_word) {
            word_start = i;
            in_word = 1;
        }
    }
    
    printf("\nNumber of words returned: %d\n", word_count);
    return word_count;
}

int main(int argc, char *argv[]) {
    char *buff;             //placeholder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING

    /*
     * This check is safe because the logical OR (||) uses short-circuit evaluation.
     * If argc < 2 is true (meaning argv[1] doesn't exist), the second condition 
     * (*argv[1] != '-') won't be evaluated at all, avoiding a potential 
     * segmentation fault from accessing non-existent memory. This ensures we have
     * both the minimum required arguments and proper flag format before proceeding.
     */
    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h') {
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING

    /*
     * This if statement ensures that we have at least 3 arguments:
     * argv[0] = program name
     * argv[1] = option flag (-h, -c, -r, etc)
     * argv[2] = input string
     * Without this check, attempting to access argv[2] for the input string
     * would cause a segmentation fault if not enough arguments were provided.
     */
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3

    // Allocate buffer memory
    buff = (char *)malloc(BUFFER_SZ);
    if (buff == NULL) {
        printf("Error: Failed to allocate memory\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0) {
        printf("Error setting up buffer, error = %d\n", user_str_len);
        free(buff);
        exit(2);
    }

    switch (opt) {
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error counting words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options        
        case 'r':
            rc = reverse_string(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error reversing string, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            break;
            
        case 'w':
            rc = print_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error printing words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            break;
        case 'x':
        if (argc != 5) {
            usage(argv[0]);
            free(buff);
            exit(1);
        }
        printf("Not Implemented!\n");
        free(buff);
        exit(0);
        break;
            
        default:
            usage(argv[0]);
            free(buff);
            exit(1);
            
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff, BUFFER_SZ);
    free(buff);  // Free allocated memory before exiting
    exit(0);
}

// TODO: #7 Notice all of the helper functions provided in the
// starter take both the buffer as well as the length. Why
// do you think providing both the pointer and the length
// is a good practice, after all we know from main() that
// the buff variable will have exactly 50 bytes?

// Answer: Providing both the pointer and the length is good practice because it ensures flexibility and safety.
// Functions can handle buffers of varying sizes without assuming a fixed size, making the code reusable.
// It also prevents potential bugs if the buffer size changes in the future or if the function is reused elsewhere.