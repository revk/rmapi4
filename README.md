# rmapi4

This works with [Royal Mail API v4](https://royalmail.proshipping.net/), providing command line options to create shipments and manifests as needed.

Note that the documentation on the RM site is crap.

At present this is not set up for all of the international options, so let me know if needed.

## --help

The `--help` explains the options for the command. You will need to call with `--client-id=` and `--client-key=` to initially create credentials in an authentication file. But after that it uses the file.

## --type

One of the options that needs more explanation is `--type=` as it covers several other options in one go. This is mainly to allow for a pull down of postage service settings in one string for easy of integration on to a web form.

It starts with the `--service-code`, e.g. `CRL1` and then a number of `/` separated values. The service codes are listed in [V4 Reference Data spreadsheet](V4ReferenceDatav1.xlsx).

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

For example `--type=TPN/1000/Parcel` means `ROYAL MAIL TRACKED 24` weight default/max 1kg, and a `Parcel`. `--type=STL1/100/Letter` means a 1st class letter up to 100g.

Notable service codes (from above spreadsheet)

|Code(s)|Meaning|
|-------|-------|
|`STL1` `STL2`|Normal 1st/2nd class account postage|
|`SD1` `SD2` `SD3`|Special delivery by 1pm (£750/£1000/£2500)|
|`SD4` `SD5` `SD6`|Special delivery by 9am (£750/£1000/£2500)|
|`SDA` `SDB` `SDC`|Special delivery by 1pm with ID check (£750/£1000/£2500)|
|`SDE` `SDF` `SDG`|Special delivery by 9am with ID check (£750/£1000/£2500)|
|`SDH` `SDJ` `SDK`|Special delivery by 1pm with age check (£750/£1000/£2500)|
|`SDM` `SDN` `SDQ`|Special delivery by 9am with age check (£750/£1000/£2500)|
|`TPM` `TPL`|Tracked 24/48 high value|
|`TPN` `TPS`|Tracked 24/48|
|`TSN` `TSS`|Tracked Return 24/48|

## Creating shipments

The `--outprefix=` option is useful as it ensures a file is created when the `--create-shipment` is done, using the shipment tracking number, e.g. `TT000350284GB.PDF` but prefixed. Typically you might used `--outprefix=/tmp/` for example.

The *stdout* from `--creat-shipment` provides one (or more, in theory, but unlikely) tracking numbers.

You can use `--print-shipment=` to get a file for an individual tracking number later if needed.

## Creating manifests

Again, the `--outprefix=` option is useful as it is quite possible for more than one manifest to be created in one go, depending on the services used. This would create file for each manifest. The manifest IDs are written to *stdout*.

You can used `--print-manifest=` later to print specific manifests.
