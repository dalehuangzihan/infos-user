/*
 * The Tree Command
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 3
 */

#include <infos.h>

// function protoypes:
bool does_satisfy_regex(const char* text, const char* pattern, bool do_lookahead);

#define STR_BUF_LEN  256
#define TREE_DEPTH_BEFORE_ROOT -1
#define NO_BRACKET 0
#define RANGE_BRACKET 1
#define FULL_BRACKET 2

int num_of_files = 0;
int num_of_directories = 0;

const char* TAB = "   ";
const char* DASHES = "---";
const char SLASH = '/';
const char WHITESPACE = ' ';
const char PIPE = '|';
const char DASH = '-';
const char OPEN_BKT = '(';
const char CLOSE_BKT = ')';
const char STAR = '*';
const char QNMK = '?';
const char DOT = '.';

const char REGEX_CMD_CHAR = 'P';
const char* DEFAULT_PATH = "/usr";

char indent_tracker[STR_BUF_LEN];
char pattern[STR_BUF_LEN];
bool do_regex = false;

char subpattern_buf[STR_BUF_LEN];
char subtext_buf[STR_BUF_LEN];

/**
 * @brief Concatenates two strings together with a slash in the middle;
 * requires a buffer to be provided
 * 
 * @param str1 is first string to be concatenated; goes before the slash
 * @param str2 is second string to be concatenated; goes after the slash 
 * @param str_buffer is the buffer that the concatenated string goes into
 */
void str_concat_slash (const char* str1, const char* str2, char* str_buffer) {
    int str1_len = strlen(str1);
    int str2_len = strlen(str2);
    for (int i = 0; i < str1_len; i++) {
        str_buffer[i] = str1[i];
    }
    str_buffer[str1_len] = SLASH;
    for (int j = 0; j < str2_len; j ++) {
        str_buffer[str1_len + 1 + j] = str2[j];
    }
    str_buffer[str1_len + 1 + str2_len] = '\0';
}

/**
 * @brief Get the substr of the given str, starting from index
 * substr_start_i; requires a buffer to be provided
 * 
 * @param str is the source string that we want to get the substring from
 * @param substr_buf is the buffer to store the substring
 * @param substr_start_i is the start index (inclusive) of the substring in the source string
 */
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

/**
 * @brief Checks if a given path is a directory (can be entered)
 * 
 * @param path is the path to the directory under inspection
 * @return true if the path leads to a directory
 * @return false if the path leads to a file
 */
bool is_path_a_directory (const char* path) {
    HDIR dir = opendir(path, 0);
    if (is_error(dir)) {
        // unable to open dir => dir is a file!
        return false;
    }
    return true;
}

/**
 * @brief Prints the indentations in the tree display
 * 
 * @param tree_depth is the current depth of the tree that we're reading
 */
void print_indents(int tree_depth) {
    for (int i = 0; i < tree_depth; i ++) {
        if (indent_tracker[i] == PIPE) {
            printf("%c%s", PIPE, TAB);
        } else {
            printf("%c%s", WHITESPACE, TAB);
        }
    }
}

/**
 * @brief Checks if the command given is a regex command (i.e. contains a "-P")
 * 
 * @param cmd is the command under inspection
 * @return true if the command is a regex command
 * @return false if the command is not a regex command
 */
bool is_cmdline_regex(const char* cmd) {
    int cmd_len = strlen(cmd);
    for (int i = 0; i < cmd_len; i ++) {
        if (i+1 < cmd_len and cmd[i] == DASH and cmd[i+1] == REGEX_CMD_CHAR) {
            // cmd contains "-P"
            return true;
        }
    }
    return false;
}

/**
 * @brief Parse the path from the valid regex command
 * 
 * @param cmd is the regex command that we're parsing the path from
 * @param path_buf is the buffer to store the parsed path
 */
void parse_path_from_valid_regex_cmd(const char* cmd, char* path_buf) {
    int cmd_len = strlen(cmd);
    int path_buf_i = 0;

    if (cmd[0] == DASH and cmd[1] == REGEX_CMD_CHAR) {
        // cmd is "-P ..." => has no specified path
        path_buf[0] = '\0';
        return;
    }
    
    for (int i = 0; i < cmd_len; i ++) {
        if (i+2 < cmd_len and cmd[i] == WHITESPACE and cmd[i+1] == DASH and cmd[i+2] == REGEX_CMD_CHAR) {
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

/**
 * @brief Parse the pattern from the valid regex command
 * 
 * @param cmd is the regex comand that we're parsing the pattern from
 * @param pattern_buf is the buffer to store the parsed pattern
 */
void parse_pattern_from_valid_regex_cmd(const char* cmd, char* pattern_buf) {
    int cmd_len = strlen(cmd);
    int pattern_buf_i = 0;
    bool has_reached_pattern = false;
    for (int i = 0; i < cmd_len; i ++) {
        if (i > 1 and cmd[i-2] == DASH  and cmd[i-1] == REGEX_CMD_CHAR and cmd[i] == WHITESPACE and cmd[i-2] == DASH) {
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

/**
 * @brief Checks if the given path is valid
 * 
 * @param path is the path under inspection
 * @return true if the path has the correct syntax
 * @return false if the path has the wrong syntax
 */
bool is_path_valid(const char* path) {
    int path_len = strlen(path);
    // check if last character is '/' or ' ', a la "/usr/" or "/usr ":
    if (path[path_len-1] == SLASH or path[path_len-1] == WHITESPACE) {
        printf("Error: illegal path \"%s\" entered!\n", path);
        return false;
    }
    return true;
}

/**
 * @brief Checks if the character is any of the following symbols:
 * '*', '?', '(', ')'
 * 
 * @param c is the character under inspection
 * @return true if the given character is a special symbol
 * @return false if the given character is not a special symbol
 */
bool is_special_symbol(char c) {
    return c == OPEN_BKT or c == CLOSE_BKT or c == STAR or c == QNMK;
}

/**
 * @brief Checks if the brackets in a given pattern tallies;
 * pattern should have the same number of open and close brackets
 * 
 * @param pattern is the pattern under inspection
 * @return true if the brackets tally up in the pattern
 * @return false if the brackets do not tally up in the pattern
 */
bool do_brackets_tally(const char* pattern) {
    int num_open_brackets = 0;
    int num_close_brackets = 0;
    for (int i = 0; i < strlen(pattern); i ++) {
        if (pattern[i] == OPEN_BKT) {
            num_open_brackets ++;
        } else if (pattern[i] == CLOSE_BKT) {
            num_close_brackets ++;
        }
    }
    return num_open_brackets == num_close_brackets;
}

/**
 * @brief Checks if a given pattern is valid
 * 
 * @param pattern is the pattern under inspection
 * @return true if the pattern is of a valid format
 * @return false if the pattern is not of a valid format
 */
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
        } else if (pattern[1] == OPEN_BKT or pattern[1] == CLOSE_BKT) {
            // dont allow "(x", ")x", "*x", "?x":
            is_valid = false;
        }   
    } else {
        for (int i = 0; i < pattern_len; i ++) {
            if (pattern[i] == OPEN_BKT) {
                if (i+1 < pattern_len and is_special_symbol(pattern[i+1])) {
                    // dont allow "((", "(*", "(?", "()":
                    is_valid = false;
                } else if (i+2 < pattern_len and pattern[i+1] == DASH and pattern[i+2] == CLOSE_BKT) {
                    // dont allow "(-)"
                    is_valid = false;
                } else if (i+3 < pattern_len and pattern[i+2] == DASH and pattern[i+3] == CLOSE_BKT) {
                    // dont allow "(x-)..."
                    is_valid = false;
                }
            } else if (pattern[i] == CLOSE_BKT) {
                // dont allow "))":
                if (i+1 < pattern_len and pattern[i+1] == CLOSE_BKT) is_valid = false;
            } else if (pattern[i] == STAR or pattern[i] == QNMK) {
                // dont allow "**", "*?", "??", "?*":
                if (i+1 < pattern_len and pattern[i+1] == STAR or pattern[i+1] == QNMK) is_valid = false;
            }
            if (!is_valid) break;
        }
    }
    // check that brackets tally:
    is_valid = is_valid and do_brackets_tally(pattern);
    if (!is_valid) printf("Error: illegal pattern \"%s\" entered!\n", pattern);
    return is_valid;
}

/**
 * @brief Checks if the given character is contained in the full bracket (a..z)
 * 
 * @param bracket_buf is the buffer containing the elements inside the brackets
 * @param c is the character under inspection
 * @return true if the character is contained in the bracket contents
 * @return false if the character is not contained in the bracket contents
 */
bool is_char_in_full_bracket(const char* bracket_buf, char c) { //const char* text, int text_m) {
    for (int i = 0; i < strlen(bracket_buf); i ++) {
        if (bracket_buf[i] == c) {// text[text_m]) {
            // printf("brkt match: bracket_buf[%d] (%c) == text[%d] (%c)\n", i, bracket_buf[i], m, text[m]);
            return true;
        }
    }
    return false;
}

/**
 * @brief Performs the recursive look-ahead for regex evaluation
 * 
 * @param text is the text that we're working with before lookahead
 * @param text_m is the index position of the text character that we are currently inspecting
 * @param pattern is the pattern that we're working with before lookahead
 * @param pattern_j is the index position of the regex pattern character that we are currently inspecting
 * @param subtext_buf is the buffer to store the subtext used for the lookahead
 * @param subpattern_buf is the buffer used to store the regex subpattern used for the lookahead
 * @param is_recurse is a flag to indicate whether or not this lookahead should allow recursive/nested calls within itself
 * @return true if the lookahead subtext satisfies the regex subpattern
 * @return false if the lookahead does not satisfy the regex subpattern
 */
bool do_recursive_regex_lookahead(const char* text, int text_m, const char* pattern, int pattern_j, char* subtext_buf, char* subpattern_buf, bool is_recurse) {
    get_substr(pattern, subpattern_buf, pattern_j);
    get_substr(text, subtext_buf, text_m);
    // printf("* START lookahead, j=%d(%c), subtext = %s, subpattern = %s\n", pattern_j-2, pattern[pattern_j-2], subtext_buf, subpattern_buf);
    bool does_satisfy_lookahead = does_satisfy_regex(subtext_buf, subpattern_buf, is_recurse);   // only look ahead one level (dont recurse)!              
    // printf("* End lookahead , text = %s, pattern = %s\n", text, pattern);
    return does_satisfy_lookahead;
}

int get_num_of_star_qnmk_terms(const char* pattern) {
    int pattern_len = strlen(pattern);
    int num_of_star_qnmk_terms = 0;
    for (int i = 0; i < pattern_len; i ++) {
        if (pattern[i] == '*' or pattern[i] == '?') {
            num_of_star_qnmk_terms ++;
        }
    }
    return num_of_star_qnmk_terms;
}

/**
 * @brief Checks if the given text satisfies the given regex pattern
 * 
 * @param text is the text that we're checking agaist the regex pattern
 * @param pattern is the regex pattern that we are checking the text against
 * @param do_lookahead is the flag indicating if this function call should perform nested lookahead calls
 * @return true if the text satisfies the regex expression
 * @return false if the text does not satisfy the regex expression
 */
bool does_satisfy_regex(const char* text, const char* pattern, bool do_lookahead) {
    // // TODO FOR TESTING:
    // do_lookahead = false;   // REMOVE
    
    int text_len = strlen(text);
    int pattern_len = strlen(pattern);
    int text_i = 0;
    bool has_satisfied_regex_before = false;
    bool has_star_qnmk_overhang = false;
    bool has_match_in_overhang = false;
    int num_of_star_qnmk_terms_remaining = get_num_of_star_qnmk_terms(pattern);
    // printf("START num_of_star_qnmk_terms_remaining = %d\n", num_of_star_qnmk_terms_remaining);

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

        if (text_i+1 == text_len and num_of_star_qnmk_terms_remaining > 0) has_star_qnmk_overhang = true;
        // if (has_star_qnmk_overhang) printf("has_star_qnmk_overhang = %d\n", has_star_qnmk_overhang);

        if (pattern[j] == OPEN_BKT) {
            // printf("bloop1\n");
            is_within_brackets = true;
            if (j+2 < pattern_len and pattern[j+2] == DASH) {
                // (x-y)
                bracket_type = RANGE_BRACKET;
            } else {
                // (a..z)
                bracket_type = FULL_BRACKET;
            }

        } else if (is_within_brackets and pattern[j+1] != CLOSE_BKT) {
            // printf("bloop2\n");
            bracket_buf[bracket_buf_i] = pattern[j];
            bracket_buf_i ++;

        } else if (is_within_brackets and pattern[j+1] == CLOSE_BKT) {
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
            }

        // Check for "<...>*":
        } else if (j+1 < pattern_len and pattern[j+1] == STAR) {
            // printf("bloop_4\n");
            char pattern_char = pattern[j];
            // printf("pattern[%d] = %c, bracket_type = %d\n", j, pattern_char, bracket_type);
            num_of_star_qnmk_terms_remaining --;
            // printf("num_of_star_qnmk_terms_remaining = %d\n", num_of_star_qnmk_terms_remaining);

            for (int m = text_i; m < text_len; m ++) {

                // perform lookahead:
                bool does_satisfy_lookahead = false;
                // char subpattern_buf[STR_BUF_LEN];
                // char subtext_buf[STR_BUF_LEN];
                
                if (j+2 < pattern_len and do_lookahead) {
                    does_satisfy_lookahead = do_recursive_regex_lookahead(text, m, pattern, j+2, subtext_buf, subpattern_buf, false);
                }
                
                bool is_match_no_bracket = bracket_type == NO_BRACKET and text[m] == pattern_char;
                bool is_match_range_bracket = bracket_type == RANGE_BRACKET and bracket_start_char <= text[m] and text[m] <= bracket_end_char;
                bool is_match_full_bracket = bracket_type == FULL_BRACKET and is_char_in_full_bracket(bracket_buf, text[m]);

                if (do_lookahead and does_satisfy_lookahead) {
                    // printf("Subpattern %s satisfied, m=%d (%c)\n", subpattern_buf, m, text[m]);
                    text_i = m;
                    j ++ ; // skip the * symbol to the next regex term
                    bracket_type = NO_BRACKET;
                    break;

                } else if (is_match_no_bracket or is_match_range_bracket or is_match_full_bracket) {
                    // match found, update text_i: 
                    // if (is_match_no_bracket) printf("*1 text[%d] (%c) == pattern[%d] (%c)\n", m, text[m], j, pattern[j]);
                    // if (is_match_range_bracket) printf("*2 %c <= text[%d] (%c) <= %c\n", bracket_start_char, m, text[m], bracket_end_char);
                    // if (is_match_full_bracket) printf("*3 text[%d] (%c) found in full bracket %s\n", m, text[m], bracket_buf);

                    has_satisfied_regex_before = true;
                    text_i = m;

                    if (text_i+1 == text_len and num_of_star_qnmk_terms_remaining > 0) has_star_qnmk_overhang = true;
                    // if (has_star_qnmk_overhang) printf("has_star_qnmk_overhang = %d\n", has_star_qnmk_overhang);
                    if (has_star_qnmk_overhang) has_match_in_overhang = true;
                    // printf("has_match_in_overhang = %d\n", has_match_in_overhang);

                    if (text_i+1 == text_len) {
                        j++;  // is last char of text, move to "next" pattern term; skips * symbol
                        bracket_type = NO_BRACKET;
                        break;
                    } 

                } else {
                    // printf("*4 text[%d] (%c) != pattern[%d] (%c)\n", m, text[m], j, pattern[j]);
                    if (text_len == 1 and j+1 == pattern_len-1 and !has_satisfied_regex_before) return false; 
                                    // text only has one char; 
                                    // curr regex term is last term in pattern (not matched); 
                                    // has not matched any terms before now
                    if (j+2 < pattern_len) text_i = m; // dont update if there are no more regex terms to compare to after  
                    // printf("text_i = %d\n", text_i);
                    if (text_i+1 == text_len and j+2 == pattern_len) {
                        if (!has_star_qnmk_overhang) return false;
                        if (has_star_qnmk_overhang and !has_match_in_overhang) return false;
                    }
                                    // last text char does not match with last regex pattern term;
                                    // this last pattern char is not an "extra overhanging * or ? regex term" 
                                    // => mismatch cannot be ignored! 
                    j ++;   // to skip the * symbol and move on to the next/"next" regex term
                    bracket_type = NO_BRACKET;  // reset bracket type since we're moving to next regex term
                    break;
                }  
            }
// ab*c* vs ab
        } else if (j+1 < pattern_len and pattern[j+1] == QNMK) {
            // printf("bloop_5\n");
            char pattern_char = pattern[j];
            // printf("pattern[%d] = %c, bracket_type = %d\n", j, pattern_char, bracket_type);
            num_of_star_qnmk_terms_remaining --;
            // printf("num_of_star_qnmk_terms_remaining = %d\n", num_of_star_qnmk_terms_remaining);

            if (text_i+1 == text_len and num_of_star_qnmk_terms_remaining > 0) has_star_qnmk_overhang = true;
            // if (has_star_qnmk_overhang) printf("has_star_qnmk_overhang = %d\n", has_star_qnmk_overhang);

            // perform lookahead:
            bool does_satisfy_lookahead = false;
            // char subpattern_buf[STR_BUF_LEN];
            // char subtext_buf[STR_BUF_LEN];
            if (j+2 < pattern_len and do_lookahead) {
                does_satisfy_lookahead = do_recursive_regex_lookahead(text, text_i, pattern, j+2, subtext_buf, subpattern_buf, false);
            }

            bool is_match_no_bracket = bracket_type == NO_BRACKET and text[text_i] == pattern_char;
            bool is_match_range_bracket = bracket_type == RANGE_BRACKET and bracket_start_char <= text[text_i] and text[text_i] <= bracket_end_char;
            bool is_match_full_bracket = bracket_type == FULL_BRACKET and is_char_in_full_bracket(bracket_buf, text[text_i]);

            if (do_lookahead and does_satisfy_lookahead) {
                // printf("Subpattern %s satisfied, text_i=%d (%c)\n", subpattern_buf, text_i, text[text_i]);
                // do nothing here; increment j on the outside

            } else if (is_match_no_bracket or is_match_range_bracket or is_match_full_bracket) {
                // match found, update text_i:
                // if (is_match_no_bracket) printf("?1 text[%d] (%c) == pattern[%d] (%c)\n", text_i, text[text_i], j, pattern[j]);
                // if (is_match_range_bracket) printf("?2 %c <= text[%d] (%c) <= %c\n", bracket_start_char, text_i, text[text_i], bracket_end_char);
                // if (is_match_full_bracket) printf("?3 text[%d] (%c) found in full bracket %s\n", text_i, text[text_i], bracket_buf);
                
                has_satisfied_regex_before = true;
                if (has_star_qnmk_overhang) has_match_in_overhang = true;
                // printf("has_match_in_overhang = %d\n", has_match_in_overhang);
                if (text_i+1 < text_len and j+2 < pattern_len) text_i ++;   // dont increment if there are no more regex chars to compare to

            } else {
                // does not match; check curr text char again with next regex
                // printf("?4 text[%d] (%c) != pattern[%d] (%c)\n", text_i, text[text_i], j, pattern[j]);
                if (text_len == 1 and j+1 == pattern_len-1 and !has_satisfied_regex_before) return false;
                                    // text only has one char
                                    // curr regex term is last term in pattern (not matched)
                                    // has not matched any terms before now
                                    // e.g. "f" against "a?b?c?d?"
                // printf("text_i = %d\n", text_i);
                if (text_i+1 == text_len and j+2 == pattern_len) {
                    if (!has_star_qnmk_overhang) return false;
                    if (has_star_qnmk_overhang and !has_match_in_overhang) return false;
                }
                                    // last text char does not match with last regex pattern term;
                                    // this last pattern char is not an "extra overhanging * or ? regex term" 
                                    // => mismatch cannot be ignored! 
            }
            bracket_type = NO_BRACKET;  // reset bracket mode since moving on to next regex term
            j ++;  // move to next/"next" pattern term; skips ? symbol
   

        } else {
            // printf("bloop6\n");
            // pattern char is not *-regex nor ?-regex
            // check if curr pattern char == curr text char
            // printf("pattern[%d] (%c), text[%d] (%c)\n", j, pattern[j], text_i, text[text_i]);

            bool is_match_no_bracket = bracket_type == NO_BRACKET and pattern[j] == text[text_i];
            bool is_match_range_bracket = bracket_type == RANGE_BRACKET and bracket_start_char <= text[text_i] and text[text_i] <= bracket_end_char;
            bool is_match_full_bracket = bracket_type == FULL_BRACKET and is_char_in_full_bracket(bracket_buf, text[text_i]);

            if (is_match_no_bracket or is_match_range_bracket or is_match_full_bracket) {
                // match found, move on to next j text_i pair:
                // if (is_match_no_bracket) printf("_1 pattern[%d] (%c) == text[%d] (%c)\n", j, pattern[j], text_i, text[text_i]);
                // if (is_match_range_bracket) printf("_2 %c <= text[%d] (%c) <= %c\n", bracket_start_char, text_i, text[text_i], bracket_end_char);
                // if (is_match_full_bracket) printf("_3 text[%d] (%c) found in full bracket %s\n", text_i, text[text_i], bracket_buf);
                
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

/**
 * @brief Enters the directory and prints the contents in the tree 
 * 
 * @param path is the current path that we're attempting to enter
 * @param tree_depth is the current depth of the tree that we're traversing
 */
void enter_directory_tree(const char* path, int tree_depth) {
    if (!is_path_a_directory(path)) {
        printf("Error: path \"%s\" is not a directory!", path);
        return;
    }
    // at this point, path must be a directory that we can enter!
    // sweep once to find the number of dirents in the dir:
    int num_of_dirents_remaining = 0;
    HDIR dir_sweep1 = opendir(path, 0);
    struct dirent de;
    while (readdir(dir_sweep1, &de)) {
        bool is_valid_dirent = true;
        if (do_regex) is_valid_dirent = does_satisfy_regex(de.name, pattern, true);
        if (is_valid_dirent) num_of_dirents_remaining ++;
    }
    closedir(dir_sweep1);
    // sweep second time, this time entering each directory:
    tree_depth ++;
    HDIR dir_sweep2 = opendir(path, 0);
    char new_path_buf [STR_BUF_LEN];
    while (readdir(dir_sweep2, &de)) {
        bool is_valid_dirent = true;
        if (do_regex) is_valid_dirent = does_satisfy_regex(de.name, pattern, true);
        if (is_valid_dirent) {
            num_of_dirents_remaining --;
            // update indent_tracker array for printing:
            if (num_of_dirents_remaining == 0) {
                indent_tracker[tree_depth] = WHITESPACE;
            } else {
                indent_tracker[tree_depth] = PIPE;
            }

            // get new path name of subdirectories / files:
            str_concat_slash(path, de.name, new_path_buf);
            if (is_path_a_directory(new_path_buf)) {
                // is a directory
                num_of_directories ++;
                print_indents(tree_depth);
                printf("%c%s%s\n", PIPE, DASHES, de.name);
                // enter into subdirectory:
                enter_directory_tree(new_path_buf, tree_depth);
            } else {
                // is a file
                num_of_files ++;
                print_indents(tree_depth);
                printf("%c%s%s\n", PIPE, DASHES, de.name);
            }
        }
    }
    closedir(dir_sweep2);
}

int main(const char *cmdline)
{
    // initialise indent_tracker arr:
    indent_tracker[0] = PIPE;

    const char* cmd;
    const char* path;

    char path_buf[STR_BUF_LEN];

    if (!cmdline || strlen(cmdline) == 0) {
        cmd = DEFAULT_PATH;
    } else {
        cmd = cmdline;
    }

    if (is_cmdline_regex(cmd)) {
        do_regex = true;        
        // get path from cmd
        parse_path_from_valid_regex_cmd(cmd, path_buf);
        if (strlen(path_buf) == 0) {
            // path_buf is empty
            path = DEFAULT_PATH;
        } else {
            path = path_buf;
        }

        // get pattern from cmd
        parse_pattern_from_valid_regex_cmd(cmd, pattern);
        if (!is_pattern_valid(pattern)) return 1;
        printf("pattern = %s, len = %d\n", pattern, strlen(pattern));

    } else {
        path = cmd;
    }

    if (!is_path_valid(path)) return 1;
    printf("path = %s\n", path);

    // // regex testing ground:
    // if (do_regex) {
    //     bool is_satisfy = does_satisfy_regex(path, pattern, true);
    //     printf("does_satisfy_regex = %d\n", is_satisfy);
    // }

    printf("%c\n", DOT);
    enter_directory_tree(path, TREE_DEPTH_BEFORE_ROOT);
    printf("\n%d directories, %d files\n", num_of_directories, num_of_files);

    return 0;
}
