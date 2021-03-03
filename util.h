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

#ifndef INETD_UTIL_H
#define INETD_UTIL_H
#define _POSIX_C_SOURCE 200809L

// Suppress unused parameter warnings
#define UNUSED(x) (void)(x)
// Program name
extern char *G_PROG_NAME;

void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t eltsize);
void *xrealloc(void *ptr, size_t newsize);

#endif
