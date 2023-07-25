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
#include <fcntl.h>
#include <sys/file.h>
#include <ajlcurl.h>

int debug = 0;

int
main (int argc, const char *argv[])
{
   j_iso8601utc = 1;
   const char *user = getenv ("DB") ? : "default";
   const char *auth_file = "/etc/rmapi4.json";
   const char *site = "proshipping.net";
   const char *clientid = NULL;
   const char *clientkey = NULL;
   const char *account = NULL;
   int createshipment = 0;
   const char *contactname = getenv ("LNAME");
   const char *companyname = getenv ("LCOMPANY");
   const char *contactemail = getenv ("LEMAIL");
   const char *contactphone = getenv ("LSMS") ? : getenv ("LTEL");
   const char *line1 = getenv ("LADDRESS1");
   const char *line2 = getenv ("LADDRESS2");
   const char *line3 = getenv ("LADDRESS3");
   const char *town = getenv ("LPOSTTOWN");
   const char *postcode = getenv ("LPOSTCODE");
   const char *county = getenv ("LCOUNTY");;
   const char *countrycode = getenv ("LCOUNTRY");
   const char *contenttype = getenv ("LCONTENTTYPE");
   const char *servicecode = getenv ("LSERVICECODE");
   const char *description = getenv ("LDESCRIPTION");
   const char *reference1 = getenv ("LREFERENCE1");
   const char *reference2 = getenv ("LREFERENCE2");
   const char *labelpdf = NULL;
   int weight = atoi (getenv ("WEIGHT") ? : getenv ("RMAPIWEIGHT") ? : "");     // Grammes
   int length = atoi (getenv ("LENGTH") ? : "");        // mm
   int width = atoi (getenv ("WIDTH") ? : "");  // mm
   int height = atoi (getenv ("HEIGHT") ? : "");        // mm
   if (!countrycode || !*countrycode)
      countrycode = "GB";
   if (!contenttype || !*contenttype)
      contenttype = "NDX";      // Non document default
   if (!description || !*description)
      description = "Goods";
   poptContext optCon;          // context for parsing command-line options
   {                            // POPT
      const struct poptOption optionsTable[] = {
         {"user", 'd', POPT_ARG_STRING, &user, 0, "User", "user"},
         {"create-shipment", 's', POPT_ARG_NONE, &createshipment, 0, "Create Shipment", NULL},
         {"label-pdf", 'o', POPT_ARG_STRING, &labelpdf, 0, "Label PDF file", "filename"},
         {"contact-name", 0, POPT_ARG_STRING, &contactname, 0, "Contact Name", "Name ($LNAME)"},
         {"company-name", 0, POPT_ARG_STRING, &companyname, 0, "Company Name", "Company ($LCOMPANY)"},
         {"contact-email", 0, POPT_ARG_STRING, &contactemail, 0, "Contact Email", "Email ($LEMAIL)"},
         {"contact-phone", 0, POPT_ARG_STRING, &contactphone, 0, "Contact Phone", "Number ($LSMS/$LTEL)"},
         {"line1", 0, POPT_ARG_STRING, &line1, 0, "Address line 1", "Address ($LADDRESS1)"},
         {"line2", 0, POPT_ARG_STRING, &line2, 0, "Address line 2", "Address ($LADDRESS2)"},
         {"line3", 0, POPT_ARG_STRING, &line3, 0, "Address line 3", "Address ($LADDRESS3)"},
         {"town", 0, POPT_ARG_STRING, &town, 0, "Town", "Post town ($LPOSTTOWN)"},
         {"postcode", 0, POPT_ARG_STRING, &postcode, 0, "Postcode", "Post code ($LPOSTCODE)"},
         {"county", 0, POPT_ARG_STRING, &county, 0, "County", "County ($LCOUNTY)"},
         {"country-code", 0, POPT_ARG_STRING, &countrycode, 0, "Country code", "Country ($LCOUNTRY/GB)"},
         {"content-type", 0, POPT_ARG_STRING, &contenttype, 0, "Content Type", "NDX/DOX/HV ($LCONTENTTYPE/NDX)"},
         {"service-code", 0, POPT_ARG_STRING, &servicecode, 0, "Service code", "Service code ($LSERVICECODE)"},
         {"description", 0, POPT_ARG_STRING, &description, 0, "Description", "Description ($LDESCRIPTION/Goods)"},
         {"reference1", 0, POPT_ARG_STRING, &reference1, 0, "Reference1", "Reference1 ($LREFERENCE1)"},
         {"reference2", 0, POPT_ARG_STRING, &reference2, 0, "Reference2", "Reference2 ($LREFERENCE2)"},
         {"weight", 0, POPT_ARG_INT, &weight, 0, "Weight", "g ($WEIGHT)"},
         {"length", 0, POPT_ARG_INT, &length, 0, "Length", "mm ($LENGTH)"},
         {"width", 0, POPT_ARG_INT, &width, 0, "Width", "mm ($WIDTH)"},
         {"height", 0, POPT_ARG_INT, &height, 0, "Height", "mm ($HEIGHT)"},
         {"auth-file", 'C', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &auth_file, 0, "Auth file", "filename"},
         {"client-id", 0, POPT_ARG_STRING, &clientid, 0, "Client ID (for setup)", "ID"},
         {"client-key", 0, POPT_ARG_STRING, &clientkey, 0, "Client Key (for setup)", "Key"},
         {"account", 0, POPT_ARG_STRING, &account, 0, "Shipping account (for setup)", "Key"},
         {"site", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &site, 0, "Base URL", "hostname"},
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
   const char *accountid = NULL;
   {                            // Config file stuff
      j_t auth = j_create (),
         a = NULL;
      int fd = open (auth_file, O_CREAT | O_RDONLY);
      if (fd < 0)
         errx (1, "Auth file failed: %s", auth_file);
      flock (fd, LOCK_EX);
      if (!access (auth_file, F_OK))
      {
         j_err (j_read_file (auth, auth_file));
         a = j_find (auth, user);
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
         if (account)
            j_store_string (a, "account", account);
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
         char *basic = NULL;
         if (asprintf (&basic, "%s:%s", j_get (a, "client-id"), j_get (a, "client-key")) < 0)
            errx (1, "malloc");
         char *e = j_curl (J_CURL_POST | J_CURL_BASIC, curl, tx, rx, basic, "https://authentication.%s/connect/token", site);
         free (basic);
         if (e)
            errx (1, "Failed: %s", e);
         if (strcasecmp (j_get (rx, "token_type") ? : "", "Bearer"))
            errx (1, "Failed to get authentication");
         bearer = j_get (rx, "access_token");
         expiry = time (0) + atoi (j_get (rx, "expires_in") ? : "");
         j_store_string (a, "bearer", bearer);
         j_store_datetime (a, "expiry", expiry);
         j_delete (&tx);
         j_delete (&rx);
         j_err (j_write_file (auth, auth_file));
         bearer = j_get (a, "bearer");
      }
      account = j_get (a, "account");
      accountid = j_get (a, "account-id");
      if (!accountid)
      {                         // Find account
         j_t rx = j_create ();
         char *e = j_curl (J_CURL_GET, curl, NULL, rx, bearer, "https://api.%s/v4/shippingAccounts", site);
         j_log (debug, "rmapi", "shippingAccounts", NULL, rx);
         if (e)
            errx (1, "Failed: %s", e);
         j_t list = j_find (rx, "ShippingAccounts");
         if (!list)
            errx (1, "No ShippingAccounts");
         j_t s = j_first (list);
         while (s)
         {
            if (!account)
               break;           // Pick first
            const char *ac = j_get (s, "AccountNumber");
            if (ac && !strcmp (ac, account))
               break;
            s = j_next (s);
         }
         if (!s)
            errx (1, "Account not found");
         j_store_string (a, "account-id", j_get (s, "ShippingAccountId"));
         j_delete (&rx);
         j_err (j_write_file (auth, auth_file));
         accountid = j_get (a, "account-id");
      }
      close (fd);
   }
   if (!bearer)
      errx (1, "No authentication bearer");
   if (!accountid)
      errx (1, "No account id");
   if (createshipment)
   {
      if (!servicecode || !*servicecode)
         errx (1, "Must specify --service-code");
      j_t tx = j_create (),
         rx = j_create (),
         j;
      j = j_store_object (tx, "ShipmentInformation");
      j_store_string (j, "ContentType", contenttype);
      j_store_string (j, "ServiceCode", servicecode);
      j_store_string (j, "DescriptionOfGoods", description);
      j_store_string (j, "WeightUnitOfMeasure", "KG");
      j_store_string (j, "DimensionsUnitOfMeasure", "CM");
      // TODO SHIP DATE as parameter
      // TODO SAFE PLACE
      j_store_null (j, "ShipmentDate"); // Today
      j = j_store_object (tx, "Shipper");
      j_store_string (j, "ShippingAccountId", accountid);
      if (reference1 && *reference1)
         j_store_string (j, "Reference1", reference1);
      if (reference2 && *reference2)
         j_store_string (j, "Reference2", reference2);
      j = j_store_object (tx, "Destination");
      j = j_store_object (j, "Address");
      if (contactname && *contactname)
         j_store_string (j, "ContactName", contactname);
      if (companyname && *companyname)
         j_store_string (j, "CompanyName", companyname);
      if (contactemail && *contactemail)
         j_store_string (j, "ContactEmail", contactemail);
      if (contactphone && *contactphone)
         j_store_string (j, "ContactPhone", contactphone);
      if (line1 && *line1)
         j_store_string (j, "Line1", line1);
      if (line2 && *line2)
         j_store_string (j, "Line2", line2);
      if (line3 && *line3)
         j_store_string (j, "Line3", line3);
      if (town && *town)
         j_store_string (j, "Town", town);
      if (postcode && *postcode)
         j_store_string (j, "Postcode", postcode);
      if (county && *county)
         j_store_string (j, "County", county);
      if (countrycode && *countrycode)
         j_store_string (j, "CountryCode", countrycode);
      j = j_store_array (tx, "Packages");
      j = j_append_object (j);
      if (weight)
         j_store_literalf (j, "DeclaredWeight", "%.3f", (float) weight / 1000);
      if (length && width && height)
      {
         j_store_literalf (j, "Length", "%.2f", (float) length / 10);
         j_store_literalf (j, "Width", "%.2f", (float) width / 10);
         j_store_literalf (j, "Height", "%.2f", (float) height / 10);
      }
      char *e = j_curl (J_CURL_SEND, curl, tx, rx, bearer, "https://api.%s/v4/shipments/rm", site);
      j_log (debug, "rmapi", "createShipment", tx, rx);
      if (e)
      {
         j_t errs = j_find (rx, "Errors");
         if (errs)
         {
            j_t e = j_first (errs);
            while (e)
            {
               fprintf (stderr, "Error %s: %s %s\n", j_get (e, "ErrorCode"), j_get (e, "Message"), j_get (e, "Cause"));
               e = j_next (e);
            }
         }
         errx (1, "Failed: %s", e);
      }
      j_t packages = j_find (rx, "Packages");
      j_t p = j_first (packages);
      while (p)
      {                         // Should be one, but...
         printf ("%s", j_get (p, "TrackingNumber"));
         p = j_next (p);
         if (p)
            printf ("\t");
      }
      if (labelpdf)
      {
         const char *label = j_get (rx, "Labels");
         FILE *f = fopen (labelpdf, "w");
         if (label && *label)
            if (!f)
               err (1, "Cannot write %s", labelpdf);
         unsigned char *bin = NULL;
         size_t len = j_base64d (label, &bin);
         if (len)
            fwrite (bin, len, 1, f);
         free (bin);
         fclose (f);
      }
      j_delete (&tx);
      j_delete (&rx);

   }
   poptFreeContext (optCon);
   return 0;
}
