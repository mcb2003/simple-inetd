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

// For rawmemchr
#define _GNU_SOURCE

#include <argz.h>
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
  // Abort if the file couldn't be opened
  if (!fp) {
    char *err;
    if (asprintf(&err, "%s: error: %s", G_PROG_NAME, fname) < 0) {
      // We likely couldn't allocate memory
      perror(G_PROG_NAME);
      exit(EXIT_FAILURE);
    }
    perror(err);
    exit(EXIT_FAILURE);
  }
  ssize_t result = 0;
  int lineno = 0; // For debug / errors
  while (1) {
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
      // First entry in list
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

  enum inetdent_parse_state state = PARSE_SERVICE; // Initial state
  size_t nargs = 0;                                // number of arguments
  const char *argz;                                // Argz vector
  size_t argz_size = 0;

  // Service name (required because getservbyname requires the protocol
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
      break;
    case PARSE_ARGS:
      // First argument, beginning of argz vector
      argz = tok;
      // Falls through
      // We need to increment `nargs` and adjust `argz_size` anyway
    default:
      argz_size += strlen(tok) + 1;
      ++nargs;
    }
    ++state;
  }
  // Allocate enough space for the argv vector
  size_t argv_size = (argz_count(argz, argz_size) + 1) * sizeof(char *);
  ent = xrealloc(ent, sizeof(inetdent_t) + argv_size);
  // Fill the argv vector
  argz_extract(argz, argz_size, (char **restrict) & ent->argv);

  return ent;
}

int print_inetdent_arginfo(const struct printf_info *info, size_t n,
                           int *argtypes, int *size) {
  UNUSED(info);
  if (n > 0) {
    argtypes[0] = PA_POINTER;
    // I searched everywhere, is this correct?
    *size = sizeof(void *);
  }
  return 1;
}

int print_inetdent(FILE *stream, const struct printf_info *info,
                   const void *const *args) {
  const inetdent_t *ent = *((const inetdent_t **)(args[0]));
  // Protocol
  struct protoent *proto = getprotobynumber(ent->proto);
  // Service
  struct servent *serv = getservbyport(ent->port, proto->p_name);
  // Communication style
  const char *style = "unknown";
  switch (ent->style) {
  case SOCK_STREAM:
    style = "stream";
    break;
  case SOCK_DGRAM:
    style = "dgram";
  }
  // Wait?
  const char *wait;
  if (info->alt)
    wait = ent->wait ? "wait" : "nowait";
  else
    wait = ent->wait ? "yes" : "no";
  // User
  struct passwd *pw = getpwuid(ent->user);
  char *buff;
  if (asprintf(&buff,
               info->alt ? "%s\t%s\t%s\t%s\t%s\t%s"
                         : "inetd entry {\n\tservice:\t%s,\n\tstyle:\t"
                           "%s,\n\tprotocol:\t%s,\n\twait:\t%s,\n\tuser:\t"
                           "%s,\n\tcommand:\t%s\n}",
               serv->s_name, style, proto->p_name, wait, pw->pw_name,
               ent->command) < 0)
    return -1;
  int len =
      fprintf(stream, "%*s", info->left ? -info->width : info->width, buff);
  free(buff);
  return len;
}
