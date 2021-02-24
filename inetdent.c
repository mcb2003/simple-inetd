/*
    inetd - The internet daemon
    Copyright (C) 2021 Michael Connor Buchan

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    email: <mikey@blindcomputing.org>
*/

#define _POSIX_C_SOURCE 200809L
// For rawmemchr
#define _GNU_SOURCE

#include <errno.h>
#include <printf.h>
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
  int lineno = 0; // For debug / errors
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
      free(line);
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
  // Insure `ent->next` is NULL so we don't segfault
  ent->next = NULL;

  int state = PARSE_SERVICE; // Initial state

  // Service name (required because getservbyname requires the protocol as well)
  const char *serv = NULL;
  struct servent *service;
  struct protoent *proto;
  struct passwd *user;
  for (char *tok = strtok(line, DELIMS); tok; tok = strtok(NULL, DELIMS)) {
    switch (state) {
    case PARSE_SERVICE:
        // Save the name for lookup when we later have the protocol name
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
      // Save the protocol number
      ent->proto = proto->p_proto;

      // Also retrieve the service info now we have the protocol string
      service = getservbyname(serv, tok);
      if (!service) {
        // Service not known
        free(ent);
        return NULL;
      }
      // Save the port number
      ent->port = service->s_port;
      break;
    case PARSE_WAIT:
      if (strcoll(tok, "nowait") == 0)
        ent->wait = false;
      else if (strcoll(tok, "wait") == 0)
        ent->wait = true;
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

int print_inetdent_arginfo(const struct printf_info *info, size_t n, int *argtypes, int *size) {
    if(n > 0) {
        argtypes[0] = PA_POINTER;
        *size = sizeof(void*);
    }
    return 1;
}

int print_inetdent(FILE *stream, const struct printf_info *info, const void *const *args) {
           const inetdent_t *ent = *((const inetdent_t **) (args[0]));
           // Protocol
struct protoent *proto = getprotobynumber(ent->proto);
// Service
struct servent *serv = getservbyport(ent->port, proto->p_name);
           // Communication style
           const char *style = "unknown";
           switch(ent->style) {
               case SOCK_STREAM: style = "stream"; break;
               case SOCK_DGRAM: style = "dgram";
           }
           // Retrieve the user info
           struct passwd *pw = getpwuid(ent->user);
    return fprintf(stream, "%s\t%s\t%s\t%s\t%s\t%s\t%s", serv->s_name, style, proto->p_name, ent->wait ? "wait": "nowait", pw->pw_name, ent->command, ent->args);
}
