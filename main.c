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

#include <stdio.h>

#include "inetdent.h"

const char *G_PROG_NAME = "inetd";

int main(int argc, char *argv[]) {
  if (argc >= 1)
    G_PROG_NAME = argv[0];

  // Register a printf handler for inetd entries
  register_printf_specifier('N', print_inetdent, print_inetdent_arginfo);

  // Parse config file
  inetdent_t *head = inetd_conf_parse("inetd.conf");
  // Print parsed entries
  for (inetdent_t *ent = head; ent; ent = ent->next) {
    printf("%N\n", ent);
  }
  return 0;
}
