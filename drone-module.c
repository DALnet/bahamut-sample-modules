/************************************************************************
 *   Bahamut IRCd - Sample Anti Drone Module
 *   Copyright (C) 2018, Kobi Shmueli
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define BIRCMODULE 1
#include "struct.h"
#include "common.h"
#include "sys.h"
#include "h.h"
#include "hooks.h"
#include "structfunc.h"

#ifndef H_H
#error "Couldn't load h.h!"
#endif
#ifndef USE_HOOKMODULES
#error "Couldn't load setup.h! did you run ./configure ???"
#else

static void *opaque; /* A pointer to the IRCd's opaque */

/* Check if a user is a drone
   Returns: 1 = User is a drone
            0 = User is not a drone
 */
int check_drone(char *nick, char *user, char *host, char *gcos)
{
    if(!strcmp(nick,"kobi-drone")) return 1; /* A drone */

    return 0; /* Not a drone */
}

/* on_connect - POSTACCESS hook handler */
int on_connect(aClient *sptr)
{
    char kill_reason[TOPICLEN + 1];

    if(check_drone(ac_name(sptr), ac_user(sptr)->username, ac_user(sptr)->host, ac_info(sptr)))
    {
        sendto_realops_lev(REJ_LEV, "Rejecting possible drone: %s", get_client_name(sptr, FALSE));
        ircstp->is_ref++;
        ircstp->is_drone++;
        strcpy(kill_reason, "[exp/dm] Compromised host, go to http://kline.dal.net/exploits/akills.htm");
        return exit_client(sptr, sptr, &me, kill_reason);
    }

    /* Automatically umode +CR all users on connect */
    sptr->umode |= (UMODE_C|UMODE_R);

    return 0; /* Let the user connect */
}

/* Called before initialized to make sure the IRCd's and the module interface versions match */
void bircmodule_check(int *ver)
{
    *ver = MODULE_INTERFACE_VERSION;
}

/* Called when a server admin uses the MODULE CMD command */
int bircmodule_command(aClient *sptr, int parc, char *parv[])
{
    return 0;
}

/* Called when a u:lined server uses the MODULE CGLOBAL command */
int bircmodule_globalcommand(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    return 0;
}

/* Called when the module is initialized */
int bircmodule_init(void *real_opaque)
{
   opaque = real_opaque;
   if(!bircmodule_add_hook(CHOOK_POSTACCESS, opaque, on_connect))
      return -1;

   return 0;
}

/* Called when the module is unloaded */
void bircmodule_shutdown()
{
}

/* Module information for MODULE LIST and MODULE info commands */
void bircmodule_getinfo(char **version, char **desc)
{
   *version = "1.0";
   *desc = "Sample Anti Drone Module";
}
#endif
