#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 1024
#define MAX_LEN 1024
#define MAX_PARAMS 16

char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    return s;
}

char *replaceAll(const char *src, const char *from, const char *to) {
    static char buffer[MAX_LEN];
    buffer[0] = '\0';
    const char *p = src;
    size_t fromLen = strlen(from), toLen = strlen(to);
    while (*p) {
        if (strncmp(p, from, fromLen) == 0) {
            strncat(buffer, to, sizeof(buffer) - strlen(buffer) - 1);
            p += fromLen;
        } else {
            strncat(buffer, p, 1);
            p++;
        }
    }
    return buffer;
}

void replaceMacroParam(char *line, const char *param, const char *value) {
    char temp[MAX_LEN];
    strcpy(temp, line);
    strcpy(line, replaceAll(temp, param, value));

    char withSpace[MAX_LEN], withComma[MAX_LEN];
    sprintf(withSpace, " %s", param);
    sprintf(withComma, ",%s", param);

    strcpy(temp, line);
    strcpy(line, replaceAll(temp, withSpace, value));

    strcpy(temp, line);
    strcpy(line, replaceAll(temp, withComma, value));
}

char *formatAssemblyLine(const char *line) {
    static char formatted[MAX_LEN];
    formatted[0] = '\0';

    if (strlen(line) == 0) return "";

    if (isspace((unsigned char)line[0])) {
        snprintf(formatted, sizeof(formatted), "%s", line);
        return formatted;
    }

    char label[MAX_LEN] = "", rest[MAX_LEN] = "";
    sscanf(line, "%s %[^\n]", label, rest);
    int labelWidth = 9;
    sprintf(formatted, "%-*s%s", labelWidth, label, rest);
    return formatted;
}

int main() {
    char *lines[MAX_LINES];
    int lineCount = 0;
    char buffer[MAX_LEN];

    while (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        lines[lineCount] = strdup(buffer);
        lineCount++;
    }

    char macroName[MAX_LEN] = "";
    char *params[MAX_PARAMS];
    int paramCount = 0;
    char *body[MAX_LINES];
    int bodyCount = 0;
    int inMacro = 0;

    for (int i = 0; i < lineCount; i++) {
        char tmp[MAX_LEN];
        strcpy(tmp, lines[i]);
        char *t = trim(tmp);

        if (!inMacro) {
            char w1[MAX_LEN] = "", w2[MAX_LEN] = "";
            sscanf(t, "%s %s", w1, w2);
            if (strlen(w2) > 0 && strcasecmp(w2, "MACRO") == 0) {
                strcpy(macroName, w1);
                inMacro = 1;

                char *p = strstr(t, "MACRO");
                if (p) {
                    p += 5;
                    p = trim(p);
                    char *token = strtok(p, ",");
                    while (token) {
                        token = trim(token);
                        if (token[0] != '&') {
                            char *withAmp = malloc(strlen(token) + 2);
                            sprintf(withAmp, "&%s", token);
                            params[paramCount++] = withAmp;
                        } else {
                            params[paramCount++] = strdup(token);
                        }
                        token = strtok(NULL, ",");
                    }
                }
            }
        } else {
            if (strcasecmp(t, "MEND") == 0)
                inMacro = 0;
            else
                body[bodyCount++] = strdup(lines[i]);
        }
    }

    int skippingMacro = 0;
    for (int i = 0; i < lineCount; i++) {
        char tmp[MAX_LEN];
        strcpy(tmp, lines[i]);
        char *t = trim(tmp);

        if (strstr(t, "MACRO")) { skippingMacro = 1; continue; }
        if (strcasecmp(t, "MEND") == 0) { skippingMacro = 0; continue; }
        if (skippingMacro) continue;

        if (strlen(macroName) > 0 &&
            strncmp(t, macroName, strlen(macroName)) == 0 &&
            isspace((unsigned char)t[strlen(macroName)])) {

            char *args = t + strlen(macroName);
            args = trim(args);
            char *actuals[MAX_PARAMS];
            int actualCount = 0;
            char *token = strtok(args, ",");
            while (token) {
                actuals[actualCount++] = trim(token);
                token = strtok(NULL, ",");
            }

            for (int b = 0; b < bodyCount; b++) {
                char lineOut[MAX_LEN];
                strcpy(lineOut, body[b]);
                for (int p = 0; p < paramCount; p++)
                    replaceMacroParam(lineOut, params[p], actuals[p]);
                printf("%s\n", formatAssemblyLine(lineOut));
            }
        } else {
            printf("%s\n", lines[i]);
        }
    }

    return 0;
}