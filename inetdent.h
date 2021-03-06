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

#ifndef INETD_ENT_H
#define INETD_ENT_H
#include <netdb.h>
#include <printf.h>
#include <stdbool.h>
#include <sys/socket.h>

#define DELIMS " \f\n\r\t\v"
// Size of the queue of incoming, but yet to be accepted connections
#define CONN_QUEUE_SIZE 32

typedef struct inetdent {
  struct inetdent *next;
  int port;
  int style;
  int proto;
  bool wait;
  uid_t user;
  char *command;
  char *argv[];
} inetdent_t;

// Parse the entire inetd.conf file
inetdent_t *inetd_conf_parse(const char *fname);

// Represents the state of construction of an `inetdent_t`
enum inetdent_parse_state {
  PARSE_SERVICE,
  PARSE_STYLE,
  PARSE_PROTO,
  PARSE_WAIT,
  PARSE_USER,
  PARSE_COMMAND,
  PARSE_ARGS,
};

// Parse a single inetd entry
inetdent_t *inetdent_parse(char *line);

// Functions used with `register_printf_specifier` from printf.h
int print_inetdent(FILE *stream, const struct printf_info *info,
                   const void *const *args);
int print_inetdent_arginfo(const struct printf_info *info, size_t n,
                           int *argtypes, int *size);

// Create and bind the server socket for an inetd entry
int inetdent_socket(inetdent_t *ent);

#endif
