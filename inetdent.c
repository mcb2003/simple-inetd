#define _POSIX_C_SOURCE 200809L
// For rawmemchr
#define _GNU_SOURCE

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "inetdent.h"
#include "util.h"

inetdent_t *inetd_conf_parse(const char *fname) {
  inetdent_t *head = NULL;
  inetdent_t *tail = NULL;
  FILE *fp = fopen(fname, "r");
  ssize_t result = 0;
  int lineno = 0;
  while (result >= 0) {
    char *line = NULL;
    size_t line_size = 0;
    result = getline(&line, &line_size, fp);
    if (result < 0 && errno != 0) {
      perror("getline");
      exit(EXIT_FAILURE);
    } else if (result < 0)
      break; // EOF

    // Increment line number
    ++lineno;

    // Parse this line
    inetdent_t *ent = inetdent_parse(line);
    if (!ent) {
      // Parsing failed
      fprintf(stderr, "Warning: Failed to parse line %d of %s (skipping)\n",
              lineno, fname);
      continue;
    }
    // Insert `ent` into the list
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
  int state = PARSE_SERVICE;

  // Service name (required because getservbyname requires the protocol as well)
  const char *serv = NULL;
  struct servent *service;
  struct protoent *proto;
  struct passwd *user;
  for (char *tok = strtok(line, DELIMS); tok; tok = strtok(NULL, DELIMS)) {
    switch (state) {
    case PARSE_SERVICE:
      serv = tok;
      break;
    case PARSE_STYLE:
      if (strcoll(tok, "stream") == 0)
        ent->style = SOCK_STREAM;
      else if (strcoll(tok, "dgram") == 0)
        ent->style = SOCK_DGRAM;
      else {
        // Invalid
        free(ent);
        return NULL;
      }
      break;
    case PARSE_PROTO:
      proto = getprotobyname(tok);
      if (!proto) {
        // Protocol not found
        free(ent);
        return NULL;
      }
      memcpy(&ent->proto, proto, sizeof(struct protoent));

      // Also retrieve the service info now we have the protocol string
      service = getservbyname(serv, tok);
      if (!service) {
        // Service not known
        free(ent);
        return NULL;
      }
      memcpy(&ent->serv, service, sizeof(struct servent));
      break;
    case PARSE_WAIT:
      if (strcoll(tok, "nowait") == 0)
        ent->wait = 0;
      else if (strcoll(tok, "wait") == 0)
        ent->wait = 1;
      else {
        // Invalid value
        free(ent);
        return NULL;
      }
      break;
    case PARSE_USER:
      // Try to get the user from the passwd db
      user = getpwnam(tok);
      if (!user) {
        // User doesn't exist
        free(ent);
        return NULL;
      }
      ent->user = user->pw_uid;
      break;
    case PARSE_COMMAND:
      ent->command = tok;

      // Also parse the arguments, as we don't want to tokenise them
      ent->args = rawmemchr(tok, '\0') + 1;
      // Stop at the next '\n'
      // Is it safe to assume there *will* be another '\n' in `ent->args`?
      *((char *)rawmemchr(ent->args, '\n')) = '\0';
      goto end; // Done!
    }
    ++state;
  }
end:
  return ent;
}
