#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_WORDS 8
#define MAX_LEN   17
#define MAX_LINE  512

char words[MAX_WORDS][MAX_LEN];
int  word_count;

char letters[10];
int  letter_count;

int  digit_of[26];
int  used[10];

int       index_of[26];
long long coef[10];
int       is_leading[10];

int parse_rebus(const char* rebus) {
    int len = 0;
    int has_equal = 0;

    word_count = 0;

    for (int i = 0; ; i++) {
        char c = rebus[i];

        if (c >= 'A' && c <= 'Z') {
            if (word_count >= MAX_WORDS || len >= MAX_LEN - 1)
                return 0;
            words[word_count][len] = c;
            len++;
        }
        else {
            if (len > 0) {
                words[word_count][len] = '\0';
                word_count++;
                len = 0;
            }
            if (c == '=')
                has_equal = 1;
            if (c == '\0')
                break;
        }
    }

    return has_equal && word_count >= 3;
}

int collect_letters(void) {
    int seen[26] = { 0 };

    letter_count = 0;
    for (int w = 0; w < word_count; w++) {
        for (int i = 0; words[w][i] != '\0'; i++) {
            int c = words[w][i] - 'A';
            if (!seen[c]) {
                seen[c] = 1;
                if (letter_count == 10)
                    return 0;
                letters[letter_count] = words[w][i];
                index_of[c] = letter_count;
                letter_count++;
            }
        }
    }
    return 1;
}

void compute_coefficients(void) {
    memset(coef, 0, sizeof(coef));
    memset(is_leading, 0, sizeof(is_leading));

    for (int w = 0; w < word_count; w++) {
        int len = (int)strlen(words[w]);
        long long power = 1;

        for (int i = len - 1; i >= 0; i--) {
            int k = index_of[words[w][i] - 'A'];
            if (w < word_count - 1)
                coef[k] += power;
            else
                coef[k] -= power;
            power *= 10;
        }

        if (len > 1)
            is_leading[index_of[words[w][0] - 'A']] = 1;
    }
}

int check_solution(void) {
    long long sum = 0;

    for (int k = 0; k < letter_count; k++)
        sum += coef[k] * digit_of[letters[k] - 'A'];

    return sum == 0;
}

int search(int k) {
    if (k == letter_count)
        return check_solution();

    for (int d = 0; d <= 9; d++) {
        if (used[d])
            continue;
        if (d == 0 && is_leading[k])
            continue;

        used[d] = 1;
        digit_of[letters[k] - 'A'] = d;

        if (search(k + 1))
            return 1;

        used[d] = 0;
    }
    return 0;
}

void build_answer(char* result) {
    int pos = 0;

    for (int w = 0; w < word_count; w++) {
        for (int i = 0; words[w][i] != '\0'; i++)
            result[pos++] = (char)('0' + digit_of[words[w][i] - 'A']);

        if (w < word_count - 2) {
            strcpy(result + pos, " + ");
            pos += 3;
        }
        else if (w == word_count - 2) {
            strcpy(result + pos, " = ");
            pos += 3;
        }
    }
    result[pos] = '\0';
}

int solve_rebus(const char* rebus, char* result) {
    if (!parse_rebus(rebus))
        return 0;
    if (!collect_letters())
        return 0;
    compute_coefficients();

    memset(used, 0, sizeof(used));
    memset(digit_of, 0, sizeof(digit_of));

    if (!search(0))
        return 0;

    build_answer(result);
    return 1;
}

void trim_newline(char* s) {
    int n = (int)strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

int solve_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        printf("Cannot open file: %s\n", filename);
        return 1;
    }

    char line[MAX_LINE];
    char answer[MAX_LINE];
    double total_time = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        trim_newline(line);
        if (line[0] == '\0')
            continue;

        clock_t start = clock();
        int ok = solve_rebus(line, answer);
        double ms = 1000.0 * (double)(clock() - start) / CLOCKS_PER_SEC;
        total_time += ms;

        if (ok)
            printf("%-45s => %-45s (%.3f ms)\n", line, answer, ms);
        else
            printf("%-45s => no solution%-34s (%.3f ms)\n", line, "", ms);
    }

    printf("Total time: %.3f ms\n", total_time);
    fclose(f);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file with rebuses>\n", argv[0]);
        return 1;
    }
    return solve_file(argv[1]);
}