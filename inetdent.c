#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "inetdent.h"

struct inetdent *inetd_conf_parse(const char *fname) {
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
    printf("%s", line);
  }
  fclose(fp);
  return NULL;
}
