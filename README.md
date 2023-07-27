# rmapi4

This works with [Royal Mail API v4](https://royalmail.proshipping.net/), providing command line options to create shipments and manifests as needed.

Note that the documentation on the RM site is crap.

At present this is not set up for all of the international options, so let me know if needed.

## --help

The `--help` explains the options for the command. You will need to call with `--client-id=` and `--client-key=` to initially create credentials in an authentication file. But after that it uses the file.

## --type

One of the options that needs more explanation is `--type=` as it covers several other options in one go. This is mainly to allow for a pull down of postage service settings in one string for easy of integration on to a web form.

It starts with the `--service-code`, e.g. `CRL1` and then a number of `/` separated values.

|Value|Meaning|
|-----|-------|
|`NDX` `DOX` `HV`|The `--content-type` (non document, document, high value)|
|`Letter` `LargeLetter` `Parcel` `PrintedPapers`|The `--package-type` (It is a good idea to set this as the default is biggest allowed which adds to cost)|
|`Signed`|Set `--signed`|
|`LocalCollect`|Set `--local-collect`|
|`SMS`|Set `--sms-update`|
|`Email`|Set `--email-update`|
|*number*|Maximum `--weight` (grammes), can also set `--weight` if same or lower|
|`£`*number*|Maximum `--insurance`, can also set `--insurance` if same or lower|

## Creating shipments

The `--outprefix=` option is useful as it ensures a file is created when the `--create-shipment` is done, using the shipment tracking number, e.g. `TT000350284GB.PDF` but prefixed. Typically you might used `--outprefix=/tmp/` for example.

The *stdout* from `--creat-shipment` provides one (or more, in theory, but unlikely) tracking numbers.

You can use `--print-shipment=` to get a file for an individual tracking number later if needed.

## Creating manifests

Again, the `--outprefix=` option is useful as it is quite possible for more than one nmanifest to be created in one go, depending on the services used. This would create file for each manifest. The manifest IDs are written to *stdout*.

You can used `--print-manifest=` later to print specific manifests.
