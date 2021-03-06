/*
   This file is part of QuasselC.

   QuasselC is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   QuasselC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with QuasselC.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <module.h>
#include <channels-setup.h>
#include <channels.h>
#include <chat-protocols.h>
#include <chatnets.h>
#include <commands.h>
#include <net-sendbuffer.h>
#include <network.h>
#include <queries.h>
#include <recode.h>
#include <servers-setup.h>
#include <nicklist.h>
#include <servers.h>
#include <settings.h>
#include <signals.h>
#include <unistd.h>
#include <lib-config/iconfig.h>

#include <printtext.h>
#include <levels.h>
#include "quassel-irssi.h"
#include "connector.h"

static void sig_chatnet_read(Quassel_CHATNET_REC* rec, CONFIG_NODE* node) {
	if(!PROTO_CHECK_CAST(CHATNET(rec), Quassel_CHATNET_REC, chat_type, "Quassel"))
		return;

	int legacy = config_node_get_bool(node, "legacy", FALSE);
	rec->legacy = legacy;

	int load_backlog = config_node_get_int(node, "load_backlog", 0);
	rec->load_backlog = load_backlog;

	int backlog_additional = config_node_get_int(node, "backlog_additional", FALSE);
	rec->backlog_additional = backlog_additional;
}

void quassel_irssi_ready(void* _r) {
	//We have full infos about which buffers are displayed
	//Time to request backlog !
	Quassel_SERVER_REC* r=(Quassel_SERVER_REC*)_r;

	Quassel_CHATNET_REC *chatnet = (Quassel_CHATNET_REC*)chatnet_find(r->connrec->chatnet);
	int policy = chatnet->load_backlog;
	int additional = chatnet->backlog_additional;

	if(policy == 0)
		return;

	int all = 0;
	if(policy == 2)
		all = 1;

	quassel_irssi_request_backlogs(r->handle->handle, all, additional);
}

void quassel_cfg_init() {
	signal_add("chatnet read", (SIGNAL_FUNC) sig_chatnet_read);

	//I'm being loaded AFTER configuration is read
	//So I need to ask for a configuration reread
	settings_reread(NULL);
}

void quassel_cfg_deinit() {
	signal_remove("chatnet read", (SIGNAL_FUNC) sig_chatnet_read);
}
