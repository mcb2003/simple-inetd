#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inetdent.h"
#include "util.h"

inetdent_t *inetd_conf_parse(const char *fname) {
  inetdent_t *head = NULL;
  inetdent_t *tail = NULL;
  FILE *fp = fopen(fname, "r");
  ssize_t result = 0;
  while (result >= 0) {
    char *line = NULL;
    size_t line_size = 0;
    result = getline(&line, &line_size, fp);
    if (result < 0 && errno != 0) {
      perror("getline");
      exit(EXIT_FAILURE);
    } else if (result < 0)
      break; // EOF

    // Parse this line
    inetdent_t *ent = inetdent_parse(line);
    if (tail) {
      tail->next = ent;
      tail = ent;
    } else {
      head = ent;
      tail = ent;
    }
  }
  fclose(fp);
  return head;
}

inetdent_t *inetdent_parse(char *line) {
  inetdent_t *ent = xmalloc(sizeof(inetdent_t));
  for (char *tok = strtok(line, DELIMS); tok; tok = strtok(NULL, DELIMS))
    printf("%s\n", tok);
  return ent;
}
