// Royal Mail Shipping API v4 command line tool

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <unistd.h>
#include <ajlcurl.h>

int debug = 0;

int
main (int argc, const char *argv[])
{
   const char *user = getenv ("DB") ? : "default";
   const char *auth_file = "/etc/rmapi4.json";
   const char *baseurl = "https://authentication.proshipping.net";
   const char *clientid = NULL;
   const char *clientkey = NULL;
   poptContext optCon;          // context for parsing command-line options
   {                            // POPT
      const struct poptOption optionsTable[] = {
         {"user", 0, POPT_ARG_STRING, &user, 0, "User", "user"},
         {"auth-file", 'C', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &auth_file, 0, "Auth file", "filename"},
         {"client-id", 0, POPT_ARG_STRING, &clientid, 0, "Client ID (for setup)", "ID"},
         {"client-key", 0, POPT_ARG_STRING, &clientkey, 0, "Client Key (for setup)", "Key"},
         {"base-url", 'U', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &baseurl, 0, "Base URL", "url"},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug"},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      //poptSetOtherOptionHelp (optCon, "");

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

      if (poptPeekArg (optCon))
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
   }
   CURL *curl = curl_easy_init ();
   if (!curl)
      errx (1, "malloc");
   if (debug)
      curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);

   const char *bearer = NULL;
   {                            // Config file stuff
      j_t auth = j_create (),
         a = NULL;
      if (!access (auth_file, F_OK))
      {
         j_err (j_read_file (auth, auth_file));
         a = j_find (auth, "user");
      }
      if (!a && !clientid)
         errx (1, "You need to authenticate user %s with --client-id and --client--key", user);
      if (clientid)
      {                         // Creating new auth
         j_delete (&a);
         a = j_store_object (auth, user);
         j_store_string (a, "client-id", clientid);
         if (clientkey)
            j_store_string (a, "client-key", clientkey);
         j_err (j_write_file (auth, auth_file));
      }

      time_t expiry = j_time (j_get (a, "expiry"));
      if (expiry && expiry > time (0) + 10)
         bearer = j_get (a, "bearer");
      if (!bearer)
      {                         // Get new bearer
         j_t tx = j_create (),
            rx = j_create ();
         j_store_string (tx, "grant_type", "client_credentials");
         char *e = j_curl (J_CURL_POST, curl, tx, rx, NULL, "%s/connect/token", baseurl);
         if (e)
            errx (1, "Failed: %s", e);
         j_err (j_write (tx, stderr));
         j_err (j_write (rx, stderr));
      }
   }

   poptFreeContext (optCon);
   return 0;
}
