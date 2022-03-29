/*
 * The Tree Command
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 3
 */

#include <infos.h>

#define STR_BUF_LEN  256
#define TREE_DEPTH_BEFORE_ROOT -1

int num_of_files = 0;
int num_of_directories = 0;

const char* TAB = "   ";
const char* DASHES = "---";

char indent_tracker[STR_BUF_LEN];
char pattern[STR_BUF_LEN];
bool do_regex = false;

void str_concat_slash (const char* str1, const char* str2, char* str_buffer) {
    int str1_len = strlen(str1);
    int str2_len = strlen(str2);
    for (int i = 0; i < str1_len; i++) {
        str_buffer[i] = str1[i];
    }
    str_buffer[str1_len] = '/';
    for (int j = 0; j < str2_len; j ++) {
        str_buffer[str1_len + 1 + j] = str2[j];
    }
    str_buffer[str1_len + 1 + str2_len] = '\0';
}

void get_substr (const char* str, char* substr_buf, int substr_start_i) {
    int str_len = strlen(str);
    int substr_buf_i = 0;
    for (int i = 0; i < str_len; i ++) {
        if (i >= substr_start_i) {
            substr_buf[substr_buf_i] = str[i];
            substr_buf_i ++;
        }
    }
    substr_buf[substr_buf_i] = '\0';
}

bool is_path_a_directory (const char* path) {
    HDIR dir = opendir(path, 0);
    if (is_error(dir)) {
        // unable to open dir => dir is a file!
        return false;
    }
    return true;
}

void print_indents(int tree_depth) {
    for (int i = 0; i < tree_depth; i ++) {
        if (indent_tracker[i] == '|') {
            printf("%c%s", '|', TAB);
        } else {
            printf("%c%s", ' ', TAB);
        }
    }
}

bool is_cmdline_regex(const char* cmd) {
    int cmd_len = strlen(cmd);
    for (int i = 0; i < cmd_len; i ++) {
        if (i+1 < cmd_len and cmd[i] == '-' and cmd[i+1] == 'P') {
            // cmd contains "-P"
            return true;
        }
    }
    return false;
}

// # commented out since we allow for cmd to just be "-P" 
// => will use default path with empty regex (nothing will match!)
// bool is_regex_cmd_valid(const char* cmd) {
//     int cmd_len = strlen(cmd);
//     for (int i = 0; i < cmd_len; i++) {
//         if (cmd[i] == '-' and cmd[i+1] == 'P') {
//             if (i+2 < cmd_len and cmd[i+2] == ' ') {
//                 // we permit patterns to be empty
//                 return true;
//             }
//         }
//     }
// }

void parse_path_from_valid_regex_cmd(const char* cmd, char* path_buf) {
    int cmd_len = strlen(cmd);
    int path_buf_i = 0;

    if (cmd[0] == '-' and cmd[1] == 'P') {
        // cmd is "-P ..." => has no specified path
        path_buf[0] = '\0';
        return;
    }
    
    for (int i = 0; i < cmd_len; i ++) {
        if (i+2 < cmd_len and cmd[i] == ' ' and cmd[i+1] == '-' and cmd[i+2] == 'P') {
            // has reached the "-P" part of "... -P ..."
            path_buf[path_buf_i] == '\0';
            return;
        } else {
            // copy cmd characters into path_buf
            path_buf[path_buf_i] = cmd[i];
            path_buf_i ++;
        }
    }
}

void parse_pattern_from_valid_regex_cmd(const char* cmd, char* pattern_buf) {
    int cmd_len = strlen(cmd);
    int pattern_buf_i = 0;
    bool has_reached_pattern = false;
    for (int i = 0; i < cmd_len; i ++) {
        if (i > 1 and cmd[i-2] == '-'  and cmd[i-1] == 'P' and cmd[i] == ' ' and cmd[i-2] == '-') {
            // has reached the end of the "-P " part of cmd; start copying at next iter
            has_reached_pattern = true;
            continue;
        } else if (has_reached_pattern) {
            // copy cmd chars into pattern_buf:
            pattern_buf[pattern_buf_i] = cmd[i];
            pattern_buf_i ++;
        }
    }
    pattern_buf[pattern_buf_i] = '\0';
}

void enter_directory_tree(const char* path, int tree_depth) {
    if (!is_path_a_directory(path)) {
        printf("Error: path is not a directory!");
        return;
    }

    // at this point, path must be a directory that we can enter!
    // sweep once to find the number of dirents in the dir:
    int num_of_dirents_remaining = 0;
    HDIR dir_sweep1 = opendir(path, 0);
    struct dirent de;
    while (readdir(dir_sweep1, &de)) {
        num_of_dirents_remaining ++;
    }
    closedir(dir_sweep1);
    // printf("num_of_dirents_remaining = %d\n", num_of_dirents_remaining);

    // sweep second time, this time entering each directory:
    tree_depth ++;
    HDIR dir_sweep2 = opendir(path, 0);
    char new_path_buf [STR_BUF_LEN];
    while (readdir(dir_sweep2, &de)) {
        num_of_dirents_remaining --;

        // update indent_tracker array:
        if (num_of_dirents_remaining == 0) {
            indent_tracker[tree_depth] = ' ';
        } else {
            indent_tracker[tree_depth] = '|';
        }

        // get new path name of subdirectories / files:
        str_concat_slash(path, de.name, new_path_buf);
        if (is_path_a_directory(new_path_buf)) {
            // is a directory
            num_of_directories ++;
            
            print_indents(tree_depth);
            printf("%c%s%s\n", '|', DASHES, de.name);

            // enter into subdirectory:
            enter_directory_tree(new_path_buf, tree_depth);
        } else {
            // is a file
            num_of_files ++;

            print_indents(tree_depth);
            printf("%c%s%s\n", '|', DASHES, de.name);
        }

    }
    closedir(dir_sweep2);
}

int main(const char *cmdline)
{
    // TODO: Implement me!

    // initialise indent_tracker arr:
    indent_tracker[0] = '|';

    const char* cmd;
    const char* path;

    char path_buf[STR_BUF_LEN];

    if (!cmdline || strlen(cmdline) == 0) {
        cmd = "/usr";
    } else {
        cmd = cmdline;
    }

    if (is_cmdline_regex(cmd)) {
        do_regex = true;
        // get pattern from cmd
        parse_pattern_from_valid_regex_cmd(cmd, pattern);
        printf("pattern = %s\n", pattern);
        // get path from cmd
        parse_path_from_valid_regex_cmd(cmd, path_buf);
        
        if (strlen(path_buf) == 0) {
            // path_buf is empty
            path = "/usr";
        } else {
            path = path_buf;
        }

    } else {
        path = cmd;
    }

    printf("path = %s\n", path);

    printf(".\n");
    enter_directory_tree(path, TREE_DEPTH_BEFORE_ROOT);

    printf("\n%d directories, %d files\n", num_of_directories, num_of_files);

    return 0;
}