/*
 * The Tree Command
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 3
 */

#include <infos.h>

#define STR_BUF_LEN  256
#define TREE_DEPTH_BEFORE_ROOT -1
#define NO_BRACKET 0
#define RANGE_BRACKET 1
#define FULL_BRACKET 2

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

bool is_path_valid(const char* path) {
    int path_len = strlen(path);
    // check if last character is '/' or ' ', a la "/usr/" or "/usr ":
    if (path[path_len-1] == '/' or path[path_len-1] == ' ') {
        printf("Error: illegal path \"%s\" entered!\n", path);
        return false;
    }
    return true;
}

bool is_special_symbol(char c) {
    return c == '(' or c == ')' or c == '*' or c == '?';
}

bool do_brackets_tally(const char* pattern) {
    int num_open_brackets = 0;
    int num_close_brackets = 0;
    for (int i = 0; i < strlen(pattern); i ++) {
        if (pattern[i] == '(') {
            num_open_brackets ++;
        } else if (pattern[i] == ')') {
            num_close_brackets ++;
        }
    }
    return num_open_brackets == num_close_brackets;
}

bool is_pattern_valid(const char* pattern) {
    bool is_valid = true;
    int pattern_len = strlen(pattern);
    
    if (pattern_len == 1) {
        // if there is only one pattern char, cannot be a special symbol!
        if (is_special_symbol(pattern[0])) is_valid = false;
    } else if (pattern_len == 2) {
        if (is_special_symbol(pattern[0])) {
            // dont allow "(x", ")x", "*x", "?x":
            is_valid = false;
        } else if (pattern[1] == '(' or pattern[1] == ')') {
            // dont allow "(x", ")x", "*x", "?x":
            is_valid = false;
        }   
    } else {
        for (int i = 0; i < pattern_len; i ++) {
            if (pattern[i] == '(') {
                if (i+1 < pattern_len and is_special_symbol(pattern[i+1])) {
                    // dont allow "((", "(*", "(?", "()":
                    is_valid = false;
                } else if (i+2 < pattern_len and pattern[i+1] == '-' and pattern[i+2] == ')') {
                    // dont allow "(-)"
                    is_valid = false;
                } else if (i+3 < pattern_len and pattern[i+2] == '-' and pattern[i+3] == ')') {
                    // dont allow "(x-)..."
                    is_valid = false;
                }
            } else if (pattern[i] == ')') {
                // dont allow "))":
                if (i+1 < pattern_len and pattern[i+1] == ')') is_valid = false;
            } else if (pattern[i] == '*' or pattern[i] == '?') {
                // dont allow "**", "*?", "??", "?*":
                if (i+1 < pattern_len and pattern[i+1] == '*' or pattern[i+1] == '?') is_valid = false;
            }
            if (!is_valid) break;
        }
    }
    // check that brackets tally
    is_valid = is_valid and do_brackets_tally(pattern);
    if (!is_valid) printf("Error: illegal pattern \"%s\" entered!\n", pattern);
    return is_valid;
}

bool does_satisfy_regex(const char* text, const char* pattern, bool do_lookahead) {
    int text_len = strlen(text);
    int pattern_len = strlen(pattern);
    int text_i = 0;
    bool has_satisfied_regex_before = false;

    char bracket_buf[STR_BUF_LEN];
    int bracket_buf_i = 0;  // is index for bracket buf elem
    int bracket_buf_len = 0;
    bool is_within_brackets = false;
    
    int bracket_type = NO_BRACKET;
    char bracket_start_char = 0;
    char bracket_end_char = 0;

    // printf("pattern_len = %d\n", pattern_len);
    if (strlen(pattern) == 0) return false; // nothing can match an emtpy regex pattern
    for (int j = 0; j < pattern_len; j ++) {

        if (pattern[j] == '(') {
            // printf("bloop1\n");
            is_within_brackets = true;
            if (j+2 < pattern_len and pattern[j+2] == '-') {
                // (x-y)
                bracket_type = RANGE_BRACKET;
            } else {
                // (a..z)
                bracket_type = FULL_BRACKET;
            }

        } else if (is_within_brackets and pattern[j+1] != ')') {
            // printf("bloop2\n");
            bracket_buf[bracket_buf_i] = pattern[j];
            bracket_buf_i ++;

        } else if (is_within_brackets and pattern[j+1] == ')') {
            // printf("bloop3\n");
            // add last (curr) elem in brackets to bracket_buf
            bracket_buf[bracket_buf_i] = pattern[j];
            bracket_buf_i ++;
            is_within_brackets = false;
            bracket_buf[bracket_buf_i] = '\0';
            bracket_buf_len = bracket_buf_i;
            bracket_buf_i = 0;
            // printf("() bracket_buf = %s, len = %d\n", bracket_buf, bracket_buf_len);

            if (bracket_type == RANGE_BRACKET) {
                // printf("is range bracket\n");
                bracket_start_char = bracket_buf[0];
                bracket_end_char = bracket_buf[bracket_buf_len-1];

            } else if (bracket_type == FULL_BRACKET) {
                // printf("is full bracket\n");

            } else {
                // printf("CRUMBS bracket type is wrong!\n");
            }


        // Check for "<...>*":
        } else if (j+1 < pattern_len and pattern[j+1] == '*') {
            // printf("bloop_4\n");
            char pattern_char = pattern[j];
            // printf("pattern[%d] = %c, bracket_type = %d\n", j, pattern_char, bracket_type);

        
            for (int m = text_i; m < text_len; m ++) {

                // perform lookahead:
                bool does_satisfy_lookahead = false;
                char subpattern_buf[STR_BUF_LEN];
                if (j+2 < pattern_len and do_lookahead) {
                    // char subpattern_buf[STR_BUF_LEN];
                    get_substr(pattern, subpattern_buf, j+2);
                    char subtext_buf[STR_BUF_LEN];
                    get_substr(text, subtext_buf, m);
                    // printf("* START lookahead, j=%d(%c), subtext = %s, subpattern = %s\n", j, pattern_char, subtext_buf, subpattern_buf);
                    does_satisfy_lookahead = does_satisfy_regex(subtext_buf, subpattern_buf, false);   // only look ahead one level (dont recurse)!              
                    // printf("* End lookahead , text = %s, pattern = %s\n", text, pattern);
                }
                
                bool is_char_in_full_bracket = false;
                for (int i = 0; i < bracket_buf_len; i ++) {
                    if (bracket_buf[i] == text[m]) {
                        // printf("brkt match: bracket_buf[%d] (%c) == text[%d] (%c)\n", i, bracket_buf[i], m, text[m]);
                        is_char_in_full_bracket = true;
                        break;
                    }
                }

                if (do_lookahead and does_satisfy_lookahead) {
                    // printf("Subpattern %s satisfied, m=%d (%c)\n", subpattern_buf, m, text[m]);
                    text_i = m;   
                    j ++ ; // skip the * symbol to the next regex term
                    bracket_type = NO_BRACKET;
                    break;

                } else if (bracket_type == NO_BRACKET and text[m] == pattern_char) {
                    // match found, update text_i:
                    // printf("*1 text[%d] (%c) == pattern[%d] (%c)\n", m, text[m], j, pattern[j]);
                    has_satisfied_regex_before = true;
                    text_i = m;
                    if (text_i+1 == text_len) j++;  // is last char of text, move to "next" pattern term; skips * symbol
                } else if (bracket_type == RANGE_BRACKET and bracket_start_char <= text[m] and text[m] <= bracket_end_char) {
                    // match found, update text_i:
                    // printf("*2 %c <= text[%d] (%c) <= %c\n", bracket_start_char, m, text[m], bracket_end_char);
                    has_satisfied_regex_before = true;
                    text_i = m;
                    if (text_i+1 == text_len) j++;  // is last char of text, move to "next" pattern term; skips * symbol
                } else if (bracket_type == FULL_BRACKET and is_char_in_full_bracket) {
                    // match found, update text_i:
                    // printf("*3 text[%d] (%c) found in full bracket %s\n", m, text[m], bracket_buf);
                    has_satisfied_regex_before = true;
                    text_i = m;
                    if (text_i+1 == text_len) j++;  // is last char of text, move to "next" pattern term; skips * symbol
                } else {
                    // printf("*4 text[%d] (%c) != pattern[%d] (%c)\n", m, text[m], j, pattern[j]);
                    if (text_len == 1 and j+1 == pattern_len-1 and !has_satisfied_regex_before) {
                        // text only has one char
                        // curr regex term is last term in pattern (not matched)
                        // has not matched any terms before now
                        return false;
                    }
                    
                    if (j+2 < pattern_len) {
                        text_i = m; // dont increment if there are no more regex terms to compare to after
                    }
                    j ++;   // to skip the * symbol and move on to the next/"next" regex term
                    bracket_type = NO_BRACKET;  // reset bracket type since we're moving to next regex term
                    break;
                }  
            }

        } else if (j+1 < pattern_len and pattern[j+1] == '?') {
            // printf("bloop_5\n");
            char pattern_char = pattern[j];
            // printf("pattern[%d] = %c, bracket_type = %d\n", j, pattern_char, bracket_type);

            // perform lookahead:
            bool does_satisfy_lookahead = false;
            char subpattern_buf[STR_BUF_LEN];
            if (j+2 < pattern_len and do_lookahead) {
                // char subpattern_buf[STR_BUF_LEN];
                get_substr(pattern, subpattern_buf, j+2);
                char subtext_buf[STR_BUF_LEN];
                get_substr(text, subtext_buf, text_i);
                // printf("? START lookahead, j=%d(%c), subtext = %s, subpattern = %s\n", j, pattern[j], subtext_buf, subpattern_buf);
                does_satisfy_lookahead = does_satisfy_regex(subtext_buf, subpattern_buf, false);   // only look ahead one level (dont recurse)!                
                // printf("? End lookahead , text = %s, pattern = %s\n", text, pattern);
            }      

            bool is_char_in_full_bracket = false;
            for (int i = 0; i < bracket_buf_len; i ++) {
                if (bracket_buf[i] == text[text_i]) {
                    // printf("brkt match: bracket_buf[%d] (%c) == text[%d] (%c)\n", i, bracket_buf[i], text_i, text[text_i]);
                    is_char_in_full_bracket = true;
                    break;
                }
            }
// (a-z)*1?(0-9)? against r11

            if (do_lookahead and does_satisfy_lookahead) {
                // printf("Subpattern %s satisfied, text_i=%d (%c)\n", subpattern_buf, text_i, text[text_i]);
                // do nothing here; increment j on the outside

            } else if (bracket_type == NO_BRACKET and text[text_i] == pattern_char) {
                // match found, update text_i:
                // printf("?1 text[%d] (%c) == pattern[%d] (%c)\n", text_i, text[text_i], j, pattern[j]);
                has_satisfied_regex_before = true;
                if (text_i+1 < text_len and j+2 < pattern_len) text_i ++;   // dont increment if there are no more regex chars to compare to

            } else if (bracket_type == RANGE_BRACKET and bracket_start_char <= text[text_i] and text[text_i] <= bracket_end_char) {
                // match found, update text_i:
                // printf("?2 %c <= text[%d] (%c) <= %c\n", bracket_start_char, text_i, text[text_i], bracket_end_char);
                has_satisfied_regex_before = true;
                if (text_i+1 < text_len and j+2 < pattern_len) text_i ++;   // dont increment if there are no more regex chars to compare to

            } else if (bracket_type == FULL_BRACKET and is_char_in_full_bracket) {
                // match found, update text_i:
                // printf("?3 text[%d] (%c) found in full bracket %s\n", text_i, text[text_i], bracket_buf);
                has_satisfied_regex_before = true;
                if (text_i+1 < text_len and j+2 < pattern_len) text_i ++;   // dont increment if there are no more regex chars to compare to

            } else {
                // does not match; check curr text char again with next regex
                // printf("?4 text[%d] (%c) != pattern[%d] (%c)\n", text_i, text[text_i], j, pattern[j]);
                if (text_len == 1 and j+1 == pattern_len-1 and !has_satisfied_regex_before) {
                    // text only has one char
                    // curr regex term is last term in pattern (not matched)
                    // has not matched any terms before now
                    // e.g. "f" against "a?b?c?d?"
                    return false;
                }
            }
            bracket_type = NO_BRACKET;  // reset bracket mode since moving on to next regex term
            j ++;  // move to next/"next" pattern term; skips ? symbol
            

        } else {
            // printf("bloop6\n");
            // pattern char is not *-regex nor ?-regex
            // check if curr pattern char == curr text char
            // printf("pattern[%d] (%c), text[%d] (%c)\n", j, pattern[j], text_i, text[text_i]);

            bool is_char_in_full_bracket = false;
            for (int i = 0; i < bracket_buf_len; i ++) {
                if (bracket_buf[i] == text[text_i]) {
                    // printf("brkt match: bracket_buf[%d] (%c) == text[%d] (%c)\n", i, bracket_buf[i], text_i, text[text_i]);
                    is_char_in_full_bracket = true;
                    break;
                }
            }

            if (bracket_type == NO_BRACKET and pattern[j] == text[text_i]) {
                // match found, move on to next j text_i pair:
                // printf("_1 pattern[%d] (%c) == text[%d] (%c)\n", j, pattern[j], text_i, text[text_i]);
                has_satisfied_regex_before = true;
                if (text_i+1 < text_len and j+1 < pattern_len) text_i ++;   // dont increment if there are no more regex chars to compare to
            } else if (bracket_type == RANGE_BRACKET and bracket_start_char <= text[text_i] and text[text_i] <= bracket_end_char) {
                // match found, move on to next j text_i pair:
                // printf("_2 %c <= text[%d] (%c) <= %c\n", bracket_start_char, text_i, text[text_i], bracket_end_char);
                has_satisfied_regex_before = true;
                if (text_i+1 < text_len and j+1 < pattern_len) text_i ++;   // dont increment if there are no more regex chars to compare to
            } else if (bracket_type == FULL_BRACKET and is_char_in_full_bracket) {
                // match found, move on to next j text_i pair:
                // printf("_3 text[%d] (%c) found in full bracket %s\n", text_i, text[text_i], bracket_buf);
                has_satisfied_regex_before = true;
                if (text_i+1 < text_len and j+1 < pattern_len) text_i ++;   // dont increment if there are no more regex chars to compare to
            } else {
                // match not found:
                return false;
            }
            bracket_type = NO_BRACKET;  // reset bracket type since we're moving to next regex term

        }
    }
    // have exhausted the number of pattern chars
    int num_of_remainging_text_chars = text_len - (text_i + 1);
    // printf("Final text_i=%d, num of remaining chars=%d\n", text_i, num_of_remainging_text_chars);
    if (num_of_remainging_text_chars == 0) {
        return true;
    } else {
        return false;
    }

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

        // update indent_tracker array for printing:
        if (num_of_dirents_remaining == 0) {
            indent_tracker[tree_depth] = ' ';
        } else {
            indent_tracker[tree_depth] = '|';
        }

        bool is_valid_dirent = true;
        if (do_regex) is_valid_dirent = does_satisfy_regex(de.name, pattern, true);
        if (is_valid_dirent) {
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
        // get path from cmd
        parse_path_from_valid_regex_cmd(cmd, path_buf);
        if (strlen(path_buf) == 0) {
            // path_buf is empty
            path = "/usr";
        } else {
            path = path_buf;
        }

        // get pattern from cmd
        parse_pattern_from_valid_regex_cmd(cmd, pattern);
        if (!is_pattern_valid(pattern)) return 1;
        printf("pattern = %s\n", pattern);

    } else {
        path = cmd;
    }

    if (!is_path_valid(path)) return 1;
    printf("path = %s\n", path);

    // regex testing ground:
    // if (do_regex) {
    //     bool is_satisfy = does_satisfy_regex(path, pattern, true);
    //     printf("does_satisfy_regex = %d\n", is_satisfy);
    // }

    printf(".\n");
    enter_directory_tree(path, TREE_DEPTH_BEFORE_ROOT);
    printf("\n%d directories, %d files\n", num_of_directories, num_of_files);

    return 0;
}