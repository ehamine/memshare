/* Tlog, syslog wrapper with IPC control.                                          */
/* Copyright (C) 2012  Tommy Wiklund                                               */
/* This file is part of Tlog.                                                      */
/*                                                                                 */
/* Tlog is free software: you can redistribute it and/or modify                    */
/* it under the terms of the GNU Lesser General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or               */
/* (at your option) any later version.                                             */
/*                                                                                 */
/* Tlog is distributed in the hope that it will be useful,                         */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   */
/* GNU Lesser General Public License for more details.                             */
/*                                                                                 */
/* You should have received a copy of the GNU Lesser General Public License        */
/* along with Tlog.  If not, see <http://www.gnu.org/licenses/>.                   */

#include "tlog_api.h"
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
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
