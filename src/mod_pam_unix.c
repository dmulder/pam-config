/* Copyright (C) 2007, 2008 Sven Schober
   Author: Sven Schober <sschober@suse.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pam-config.h"


static int
write_config_unix (pam_module_t *this, enum write_type op, FILE *fp)
{
  option_set_t *opt_set = this->get_opt_set (this, op);
  int with_krb5, with_ldap, with_lum, with_winbind, with_pwcheck,
    with_cracklib, with_mount;

  if (debug)
    debug_write_call (this, op);

  if (!opt_set->is_enabled (opt_set, "is_enabled"))
    return 0;

  with_krb5	= is_module_enabled (common_module_list, "pam_krb5.so"	  , op);
  with_ldap	= is_module_enabled (common_module_list, "pam_ldap.so"	  , op);
  with_lum	= is_module_enabled (common_module_list, "pam_lum.so"	  , op);
  with_winbind	= is_module_enabled (common_module_list, "pam_winbind.so" , op);
  with_pwcheck	= is_module_enabled (common_module_list, "pam_pwcheck.so" , op);
  with_cracklib = is_module_enabled (common_module_list, "pam_cracklib.so", op);
  with_mount	= is_module_enabled (common_module_list, "pam_mount.so"	  , op);

  switch (op)
  {
    case AUTH:
      if (with_krb5 || with_ldap || with_lum || with_winbind)
	/* Only sufficient if other modules follow */
	fprintf (fp, "auth\tsufficient\tpam_unix.so\t");
      else
	fprintf (fp, "auth\trequired\tpam_unix.so\t");
      if (with_mount)
	/* if pam_mount is enabled it asks for a pw so we use that
	 * one.
	 * */
	fprintf (fp, "use_first_pass ");
      break;
    case ACCOUNT:
      if (with_krb5 || with_ldap || with_lum || with_winbind)
	fprintf (fp, "account\trequisite\tpam_unix.so\t");
      else
	fprintf (fp, "account\trequired\tpam_unix.so\t");
      break;
    case PASSWORD:
		if (with_krb5)
		{
			fprintf (fp, "password\t[default=ignore success=1]\tpam_succeed_if.so\tuid > 999 ");
			if (opt_set->is_enabled (opt_set, "debug"))
				fprintf (fp, "debug \n");
			else
				fprintf (fp, "quiet \n");
		}
		if (with_krb5 || with_ldap || with_lum)
			fprintf (fp, "password\tsufficient\tpam_unix.so\t");
		else
			fprintf (fp, "password\trequired\tpam_unix.so\t");
		if (with_pwcheck || with_cracklib)
			fprintf (fp, "use_authtok ");
		break;
    case SESSION:
      fprintf (fp, "session\trequired\tpam_unix.so\t");
      break;
  }

  WRITE_CONFIG_OPTIONS

  return 0;
}

GETOPT_START_ALL
GETOPT_END_ALL

PRINT_ARGS("unix")

/* ---- contruct module object ---- */
DECLARE_BOOL_OPTS_11(is_enabled, debug, audit, nodelay, nullok, shadow, md5, bigcrypt, not_set_pass, nis, broken_shadow);
DECLARE_STRING_OPTS_1(remember);
DECLARE_OPT_SETS;
/* at last construct the complete module object */
pam_module_t mod_pam_unix = { "pam_unix.so", opt_sets,
			      &def_parse_config,
			      &def_print_module,
			      &write_config_unix,
			      &get_opt_set,
			      &getopt,
			      &print_args};
