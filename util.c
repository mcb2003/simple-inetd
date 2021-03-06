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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void *xmalloc(size_t size) {
  void *ptr;
  if ((ptr = malloc(size)))
    return ptr;
  perror(G_PROG_NAME);
  exit(EXIT_FAILURE);
}

void *xcalloc(size_t count, size_t eltsize) {
  void *ptr;
  if ((ptr = calloc(count, eltsize)))
    return ptr;
  perror(G_PROG_NAME);
  exit(EXIT_FAILURE);
}

void *xrealloc(void *ptr, size_t newsize) {
  if ((ptr = realloc(ptr, newsize)))
    return ptr;
  perror(G_PROG_NAME);
  exit(EXIT_FAILURE);
}

char *argv2str(const char *const argv[]) {
  size_t old_len;
  size_t len = 0;
  char *buff = NULL;
  for (size_t i = 0; argv[i]; ++i) {
    size_t arg_len = strlen(argv[i]);
    old_len = len;
    len += arg_len + 1;
    buff = xrealloc(buff, len);
    memcpy(buff + old_len, argv[i], arg_len);
    buff[len - 1] = ' ';
  }
  buff[len - 1] = '\0';
  return buff;
}
