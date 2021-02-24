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

#define DELIMS " \t\n\v"

typedef struct inetdent {
  struct inetdent *next;
  struct servent serv;
  int style;
  struct protoent proto;
  bool wait;
  uid_t user;
  char *command;
  char *args;
} inetdent_t;

// Parse the entire inetd.conf file
inetdent_t *inetd_conf_parse(const char *fname);

// Represents the state of construction of an `inetdent_t`
#define PARSE_SERVICE 0
#define PARSE_STYLE 1
#define PARSE_PROTO 2
#define PARSE_WAIT 3
#define PARSE_USER 4
#define PARSE_COMMAND 5

// Parse a single inetd entry
inetdent_t *inetdent_parse(char *line);

// Functions used with `register_printf_function` from printf.h
int print_inetdent(FILE *stream, const struct printf_info *info, const void *const *args);
int print_inetdent_info(const struct printf_info *info, size_t n, int *argtypes, int *size);

#endif
