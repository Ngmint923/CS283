#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFFER_SZ 50

// Prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);
int count_words(char *, int, int);
void reverse_string(char *, int);
void print_words(char *, int);
void free_buffer(char *);
int search_replace(char *, int, char *, char *);

// #TODO: #4 Implement the setup buff as per the directions
// Processes the user input string by removing extra spaces, collapsing multiple spaces into one,
// and padding the buffer with '.' characters up to BUFFER_SZ.
int setup_buff(char *buff, char *user_str, int len) {
    char *src = user_str, *dst = buff;
    int count = 0;
    int in_space = 1;

    while (*src) {
        if (*src == ' ') {
            if (!in_space) {  
                if (count >= len) return -1; 
                *dst++ = ' ';
                count++;
            }
            in_space = 1;
        } else {
            if (count >= len) return -1; 
            *dst++ = *src;
            count++;
            in_space = 0;
        }
        src++;
    }

    // Remove trailing space before padding with '.'
    if (count > 0 && *(dst - 1) == ' ') {
        dst--;
        count--;
    }

    // Fill remaining buffer with '.'
    while (count < len) {
        *dst++ = '.';
        count++;
    }

    return count;
}




// Frees allocated memory for the buffer
void free_buffer(char *buff) {
    free(buff);
}

// Counts the number of words in the processed buffer
int count_words(char *buff, int len, int str_len) {
    int count = 0, in_word = 0;
    for (int i = 0; i < str_len; i++) {
        if (buff[i] == ' ') {
            in_word = 0;
        } else if (!in_word) {
            count++;
            in_word = 1;
        }
    }
    return count;
}


// Reverses the processed string in place using two-pointer technique, manually count the dots, remove it and add it back at the end because it got flipped with the sentence
void reverse_string(char *buff, int str_len) {
    int dot_count = 0;
    int i = BUFFER_SZ - 1;

    while (i >= 0 && buff[i] == '.') {
        dot_count++;
        i--;
    }

    char *left = buff;
    char *right = buff + (BUFFER_SZ - dot_count - 1);  

    while (left < right) {
        char temp = *left;
        *left++ = *right;
        *right-- = temp;
    }

    int text_len = BUFFER_SZ - dot_count; 
    for (i = 0; i < text_len; i++) {
        buff[i] = buff[i];
    }

    for (i = text_len; i < BUFFER_SZ; i++) {
        buff[i] = '.';
    }
}




// Prints words in the buffer along with their lengths
void print_words(char *buff, int str_len) {
    printf("Word Print\n");
    printf("----------\n");

    // Count and remove trailing dots
    int dot_count = 0;
    int i = BUFFER_SZ - 1;
    while (i >= 0 && buff[i] == '.') {
        dot_count++;
        i--;
    }

    // Adjust `str_len` to ignore dots
    str_len -= dot_count;
    if (str_len < 0) str_len = 0; 

    int index = 1;
    char *ptr = buff, *start;
    
    while (*ptr && (ptr - buff) < str_len) {
        // Skip spaces before a word
        while (*ptr == ' ' && (ptr - buff) < str_len) ptr++;

        if (*ptr && *ptr != '.') { 
            start = ptr; 

            // Find the end of the word
            while (*ptr != ' ' && *ptr != '.' && (ptr - buff) < str_len) ptr++;

            int length = ptr - start; 

            printf("%d. ", index++);
            while (start < ptr) putchar(*start++);
            printf("(%d)\n", length);
        }

        // Move to the next word (skip spaces after the word)
        while (*ptr == ' ' && (ptr - buff) < str_len) ptr++;
    }

    printf("\nNumber of words returned: %d\n", index - 1);
}



// Prints the full buffer including padding
void print_buff(char *buff, int len) {
    printf("Buffer:  [");
    for (int i = 0; i < len; i++) putchar(buff[i]);
    printf("]\n");
}

// Displays the usage message for the program
void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

/// Replaces the first occurrence of an arg with another, if string is too long, truncates instead of failing
int search_replace(char *buff, int str_len, char *find, char *replace) {
    char *src = buff, *match = NULL;
    int find_len = 0, replace_len = 0, i;

    while (find[find_len]) find_len++;
    while (replace[replace_len]) replace_len++;

    while (src < buff + str_len) {
        char *temp = src, *f = find;

        while (*temp == *f && *f && temp < buff + str_len) {
            temp++;
            f++;
        }

        if (!*f) {
            match = src;
            break;
        }

        src++;
    }

    if (!match) {
        return -1;  
    }

    int remaining_len = str_len - (match - buff) - find_len;
    int new_str_len = (match - buff) + replace_len + remaining_len;

    if (new_str_len > BUFFER_SZ) {
        new_str_len = BUFFER_SZ;  
        remaining_len = BUFFER_SZ - ((match - buff) + replace_len);
    }

    if (replace_len != find_len) {
        char *end = buff + str_len;
        char *shift_src = match + find_len;
        char *shift_dest = match + replace_len;

        if (replace_len > find_len) {  
            for (i = end - buff; i >= (match - buff) + find_len; i--) {
                if (i + (replace_len - find_len) < BUFFER_SZ) {
                    buff[i + (replace_len - find_len)] = buff[i];
                }
            }
        } else {
            for (i = 0; i < remaining_len; i++) {
                shift_dest[i] = shift_src[i];
            }
        }
    }

    for (i = 0; i < replace_len; i++) {
        match[i] = replace[i];
    }

    for (i = new_str_len; i < BUFFER_SZ; i++) {
        buff[i] = '.';
    }

    return 0;
}



int main(int argc, char *argv[]) {
    char *buff;
    char *input_string;
    char opt;
    int rc, user_str_len;

    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = (char) *(argv[1] + 1);

    if (opt == 'h') {
        usage(argv[0]);
        exit(0);
    }

    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2];

    // #TODO: #3 Allocate space for the buffer using malloc
    buff = (char *)malloc(BUFFER_SZ);
    if (!buff) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0) {
        printf("Error: Provided input string is too long\n");
        free_buffer(buff);
        exit(3);
    }

    switch (opt) {
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            printf("Word Count: %d\n", rc);
            break;

        case 'r':
            reverse_string(buff, user_str_len);
            break;

        case 'w':
            print_words(buff, user_str_len);
            break;

        case 'x':
            if (argc < 5) { 
                printf("Error: -x requires 2 additional arguments\n");
                free_buffer(buff);
                exit(3);
            }

            rc = search_replace(buff, user_str_len, argv[3], argv[4]);

            if (rc < 0) {
                free_buffer(buff);
                exit(3);
            }

    break;
	default:
            usage(argv[0]);
            free_buffer(buff);
            exit(1);
    }

    print_buff(buff, BUFFER_SZ);
    free_buffer(buff);
    exit(0);
}
