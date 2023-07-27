# rmapi4

This works with [Royal Mail API v4](https://royalmail.proshipping.net/), providing command line options to create shipments and manifests as needed.

Note that the documentation on the RM site is crap.

At present this is not set up for all of the international options, so let me know if needed.

## --help

The `--help` explains the options for the command. You will need to call with `--client-id=` and `--client-key=` to initially create credentials in an authentication file. But after that it uses the file.

## --type

One of the options that needs more explanation is `--type=` as it covers several other options in one go. This is mainly to allow for a pull down of postage service settings in one string for easy of integration on to a web form.

It starts with the
