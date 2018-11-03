/************************************************************************
 *   Bahamut IRCd - Sample UHM (User Host-Masking) Module
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

/******************************************/
/* Note: Please change the following key: */
/******************************************/
#define SECRET_UHM_KEY "changeme"

#ifndef H_H
#error "Couldn't load h.h!"
#endif
#ifndef USE_HOOKMODULES
#error "Couldn't load setup.h! did you run ./configure ???"
#else

static void *opaque; /* A pointer to the IRCd's opaque */

/* maskhost_handler - MASKHOST hook handler
   Returns: UHM_SUCCESS      = Success (1)
            UHM_SOFT_FAILURE = Soft failure (0)
            UHM_HARD_FAILURE = Hard failure (-2)
 */
int maskhost_handler(char *orghost, char **newhostptr, int type)
{
    char key[] = SECRET_UHM_KEY;
    char newhost[HOSTLEN + 1];
    int len;
    unsigned char *digest;
    int digest_pos;
    int orghostlen = strlen(orghost); /* save strlen() result for orghost as we'll use it a lot */
    unsigned int hmaclen;
    unsigned char hmacraw[EVP_MAX_MD_SIZE];

    if(type == 1)
    {
        len = 0; /* Init the len of the new (masked) hostname */
        HMAC(EVP_sha256(), key, strlen(key), (const unsigned char *)orghost, orghostlen, hmacraw, &hmaclen);
        digest_pos = 0;
        while(digest_pos < hmaclen)
        {
            if(digest_pos!=0 && digest_pos%2 == 0) newhost[len++] = '-';
            len += sprintf(&newhost[len], "%02x", (unsigned int)digest[++digest_pos]);
            if(len >= HOSTLEN)
            {
                /* This really really really shouldn't happen! */
                return FLUSH_BUFFER; /* Masked hostname is too long! */
            }
        }
        /* Suffix it with .fake */
        if(len+5 >= HOSTLEN)
        {
            return FLUSH_BUFFER; /* Masked hostname is too long! */
        }
        newhost[len++] = '.';
        newhost[len++] = 'f';
        newhost[len++] = 'a';
        newhost[len++] = 'k';
        newhost[len++] = 'e';
        newhost[len] = '\0';
        strcpy(*newhostptr, newhost);
        return UHM_SUCCESS; /* Success */
    }

    if(type == 2)
    {
        /* Temporary for debugging! */
        strcpy(*newhostptr, "fake.kobi.co.il");
        return UHM_SUCCESS; /* Success */
    }

    sendto_realops_lev(DEBUG_LEV, "UHM Error (unknown uhm type %d) for %s", type, orghost);

    return UHM_SOFT_FAILURE; /* Unknown error (will try other modules if available) */
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
   if(!bircmodule_add_hook(CHOOK_MASKHOST, opaque, maskhost_handler))
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
   *desc = "Sample UHM Module";
}
#endif
