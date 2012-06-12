/* Memshare, quick and easy IPC.                                                   */
/* Copyright (C) 2012  Tommy Wiklund                                               */

/* This program is free software; you can redistribute it and/or                   */
/* modify it under the terms of the GNU General Public License                     */
/* as published by the Free Software Foundation; either version 2                  */
/* of the License, or (at your option) any later version.                          */

/* This program is distributed in the hope that it will be useful,                 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                  */

/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   */
/* GNU General Public License for more details.                                    */

/* You should have received a copy of the GNU General Public License               */
/* along with this program; if not, write to the Free Software                     */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. */

#include "tlog_api.h"
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int val, temp1, temp2;

	tsyslog_init("logtest");

	while (1) {
		tsyslog(LOG_DEBUG, "Detta är en debug utskrift\n");
		tsyslog(LOG_INFO, "Detta är en info utskrift\n");
		tsyslog(LOG_NOTICE, "Detta är en notice utskrift\n");
		tsyslog(LOG_WARNING, "Detta är en warning utskrift\n");
		tsyslog(LOG_ERR, "Detta är en error utskrift\n");
		tsyslog(LOG_CRIT, "Detta är en critical utskrift\n");
		tsyslog(LOG_ALERT, "Detta är en alert utskrift\n");
		tsyslog(LOG_EMERG, "Detta är en emergency utskrift\n");
		sleep(2);
	}
}
