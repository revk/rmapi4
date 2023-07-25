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

void
fail (const char *e, j_t rx)
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
   const char *msg = j_get (rx, "Message");
   if (msg)
      fprintf (stderr, "Error %s\n", msg);
   errx (1, "Failed: %s", e);
}

int
main (int argc, const char *argv[])
{
   j_iso8601utc = 1;
   int debug = 0;
   int quiet = 0;
   const char *user = getenv ("DB") ? : "default";
   const char *auth_file = "/etc/rmapi4.json";
   const char *site = "proshipping.net";
   const char *clientid = NULL;
   const char *clientkey = NULL;
   const char *account = NULL;
   int createshipment = 0;
   int manifest = 0;
   const char *contactname = NULL;
   const char *companyname = NULL;
   const char *contactemail = NULL;
   const char *contactphone = NULL;
   const char *line1 = NULL;
   const char *line2 = NULL;
   const char *line3 = NULL;
   const char *town = NULL;
   const char *postcode = NULL;
   const char *county = NULL;
   const char *countrycode = NULL;
   const char *contenttype = NULL;
   const char *servicecode = NULL;
   const char *description = NULL;
   const char *reference1 = NULL;
   const char *reference2 = NULL;
   const char *safeplace = NULL;
   const char *outfile = NULL;
   const char *outprefix = NULL;
   int weight = 0;
   int length = 0;
   int width = 0;
   int height = 0;
   int pdf = 0;
   int png = 0;
   int json = 0;
   int zpl203 = 0;
   int zpl300 = 0;
   poptContext optCon;          // context for parsing command-line options
   {                            // POPT
      const struct poptOption optionsTable[] = {
         {"user", 'd', POPT_ARG_STRING, &user, 0, "User", "user"},
         {"create-shipment", 's', POPT_ARG_NONE, &createshipment, 0, "Create Shipment", NULL},
         {"manifest", 's', POPT_ARG_NONE, &manifest, 0, "Create manifest", NULL},
         {"outprefix", 'p', POPT_ARG_STRING, &outprefix, 0, "Output prefix", "file/pathname"},
         {"outfile", 'o', POPT_ARG_STRING, &outfile, 0, "Output file", "filename"},
         {"pdf", 0, POPT_ARG_NONE, &pdf, 0, "PDF format"},
         {"png", 0, POPT_ARG_NONE, &png, 0, "PNG format"},
         {"json", 0, POPT_ARG_NONE, &json, 0, "JSON (datastream) format"},
         {"zpl203", 0, POPT_ARG_NONE, &zpl203, 0, "Zebra 203dpi format"},
         {"zpl300", 0, POPT_ARG_NONE, &zpl300, 0, "Zebra 300dpi format"},
         {"contact-name", 0, POPT_ARG_STRING, &contactname, 0, "Contact Name", "Name"},
         {"company-name", 0, POPT_ARG_STRING, &companyname, 0, "Company Name", "Company"},
         {"contact-email", 0, POPT_ARG_STRING, &contactemail, 0, "Contact Email", "Email"},
         {"contact-phone", 0, POPT_ARG_STRING, &contactphone, 0, "Contact Phone", "Number"},
         {"line1", 0, POPT_ARG_STRING, &line1, 0, "Address line 1", "Address"},
         {"line2", 0, POPT_ARG_STRING, &line2, 0, "Address line 2", "Address"},
         {"line3", 0, POPT_ARG_STRING, &line3, 0, "Address line 3", "Address"},
         {"town", 0, POPT_ARG_STRING, &town, 0, "Town", "Post town"},
         {"postcode", 0, POPT_ARG_STRING, &postcode, 0, "Postcode", "Post code"},
         {"county", 0, POPT_ARG_STRING, &county, 0, "County", "County"},
         {"country-code", 0, POPT_ARG_STRING, &countrycode, 0, "Country code", "Country (GB)"},
         {"content-type", 0, POPT_ARG_STRING, &contenttype, 0, "Content Type", "NDX/DOX/HV (NDX)"},
         {"service-code", 0, POPT_ARG_STRING, &servicecode, 0, "Service code", "Service code"},
         {"description", 0, POPT_ARG_STRING, &description, 0, "Description", "Description (Goods)"},
         {"reference", 0, POPT_ARG_STRING, &reference1, 0, "Reference1", "Reference1"},
         {"reference2", 0, POPT_ARG_STRING, &reference2, 0, "Reference2", "Reference2"},
         {"safe-plave", 0, POPT_ARG_STRING, &safeplace, 0, "Safe place", "Text"},
         {"weight", 0, POPT_ARG_INT, &weight, 0, "Weight", "g ($WEIGHT)"},
         {"length", 0, POPT_ARG_INT, &length, 0, "Length", "mm ($LENGTH)"},
         {"width", 0, POPT_ARG_INT, &width, 0, "Width", "mm ($WIDTH)"},
         {"height", 0, POPT_ARG_INT, &height, 0, "Height", "mm ($HEIGHT)"},
         {"auth-file", 'C', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &auth_file, 0, "Auth file", "filename"},
         {"client-id", 0, POPT_ARG_STRING, &clientid, 0, "Client ID (for setup)", "ID"},
         {"client-key", 0, POPT_ARG_STRING, &clientkey, 0, "Client Key (for setup)", "Key"},
         {"account", 0, POPT_ARG_STRING, &account, 0, "Shipping account (for setup)", "Key"},
         {"site", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &site, 0, "Base URL", "hostname"},
         {"quiet", 'v', POPT_ARG_NONE, &quiet, 0, "Quiet"},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug"},
         // TODO international stuff
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
   if (!countrycode || !*countrycode)
      countrycode = "GB";
   if (servicecode && *servicecode && (!contenttype || !*contenttype))
      contenttype = (!strncmp (servicecode, "CRL", 3) ? "DOX" : "NDX"); // Default, assume document for 1st/2nd post
   if (!description || !*description)
      description = "Goods";
   if (!pdf && !png && !json && !zpl203 && !zpl300)
      pdf = 1;
   if (pdf + png + json + zpl203 + zpl300 != 1)
      errx (1, "Pick one, --pdf, --png, --json, --zpl203, or --zpl300");
   if (outfile && outprefix)
      errx (1, "Use --outfile or --outprefix, not both");
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
            fail (e, rx);
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
            fail (e, rx);
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
      j_store_string (j, "LabelFormat", pdf ? "PDF" : png ? "PNG" : zpl203 ? "ZPL203DPI" : zpl300 ? "ZPL300DPI" : "DATASTREAM");
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
         fail (e, rx);
      j_t packages = j_find (rx, "Packages");
      j_t p = j_first (packages);
      const char *trackingnumber = NULL;
      while (p)
      {                         // Should be one, but...
         trackingnumber = j_get (p, "TrackingNumber");
         const char *shipmentid = j_get (p, "ShipmentId");
         if (!quiet)
            printf ("%s", trackingnumber);
         j_t rx = j_create ();
         e = j_curl (J_CURL_GET, curl, NULL, rx, bearer, "https://api.%s/v4/shipments/printLabel/rm/%s", site, shipmentid);
         // Not bothering to log this
         if (e)
            fail (e, rx);
         j_delete (&rx);
         p = j_next (p);
         if (p)
         {
            if (!quiet)
               printf ("\t");
            warnx ("Multiple packages");
         }
      }
      if (outfile || outprefix)
      {
         char *fn = NULL;
         const char *format = j_get (rx, "LabelFormat");
         if (format && !strcmp (format, "DATASTREAM"))
            format = "JSON";
         if (outfile)
            fn = strdup (outfile);
         else if (!trackingnumber)
            errx (1, "No tracking number");
         else if (asprintf (&fn, "%s%s.%s", outprefix, trackingnumber, format) < 0)
            errx (1, "malloc");
         if (!strcmp (format, "JSON"))
            j_err (j_write_file (rx, fn));
         else
         {
            const char *label = j_get (rx, "Labels");
            if (label && *label)
            {
               FILE *f = fopen (fn, "w");
               if (!f)
                  err (1, "Cannot write %s", fn);
               free (fn);
               unsigned char *bin = NULL;
               size_t len = j_base64d (label, &bin);
               if (len)
                  fwrite (bin, len, 1, f);
               free (bin);
               fclose (f);
            }
         }
      }
      j_delete (&tx);
      j_delete (&rx);

   }
   if (manifest)
   {
      j_t tx = j_create (),
         rx = j_create ();
      j_store_string (tx, "ShippingAccountId", accountid);
      char *e = j_curl (J_CURL_SEND, curl, tx, rx, bearer, "https://api.%s/v4/manifests/rm", site);
      j_log (debug, "rmapi", "Manifest", tx, rx);
      if (e)
         fail (e, rx);
      j_t m = j_first (rx);
      while (m)
      {
         const char *manifestnumber = j_get (m, "ManifestNumber");
         const char *image = j_get (m, "ManifestImage");
         if (image)
         {
            unsigned char *bin = NULL;
            size_t len = j_base64d (image, &bin);
            if (len)
            {
               char *fn = NULL;
               const char *format = "PDF";      // Always PDF
               if (outfile)
                  fn = strdup (outfile);
               else if (outprefix)
               {
                  if (!manifestnumber)
                     errx (1, "No manifest number");
                  if (asprintf (&fn, "%s%s.%s", outprefix, manifestnumber, format) < 0)
                     errx (1, "malloc");
               }
               if (fn)
               {
                  FILE *f = fopen (fn, "w");
                  if (!f)
                     err (1, "Cannot create %s", fn);
                  free (fn);
                  fwrite (bin, len, 1, f);
                  fclose (f);
                  if (!quiet)
                     printf ("%s", manifestnumber);
               } else
                  fwrite (bin, len, 1, stdout);
            }
            free (bin);
         }
         m = j_next (m);
         if (m)
         {
            warnx ("Multiple manifest");
            if ((outfile || outprefix) && !quiet)
               printf ("\t");
         }
      }
      j_delete (&tx);
      j_delete (&rx);
   }
   poptFreeContext (optCon);
   return 0;
}
