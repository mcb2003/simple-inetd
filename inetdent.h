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
#include <stdbool.h>
#include <sys/socket.h>

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

inetdent_t *inetd_conf_parse(const char *fname);

#endif
