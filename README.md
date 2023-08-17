# Breakpoints

**Breakpoints** are where a responsive design adjusts in order to display
correctly on different devices. The Breakpoints module standardizes the use of
breakpoints, and enables modules and themes to expose or use each others'
breakpoints. The Breakpoint module keeps track of the height, width, and
resolution breakpoints.

Breakpoints itself does not natively use the breakpoints entered in order to
provide additional functionality to the site; as a result, some theming or
development knowledge is required in order to harness the power of breakpoints.

## Installation

- Install this module using the [official Backdrop CMS instructions](https://backdropcms.org/guide/modules)

## Configuration

Breakpoints can be configured in two ways: from the module configuration page
or in the theme.info file. Once configured, breakpoints can be exported to
themes or to responsive styles,

Configuring breakpoints from the module configuration page provides several
options that allows them to be used quickly, including exporting breakpoints or
creating image styles to match breakpoints. To configure a breakpoint from the
module configuration page:

1. Navigate to admin/config/media/breakpoints on your Backdrop installation.
2. Enter a human-readible name for your new breakpoint under "Name".
3. Enter a CSS @media breakpoint at which you want the breakpoint to trigger
   under "Breakpoint". For example, "min-width: 750px".
4. Select multipliers if you wish to support Retina displays under "Multipliers".
5. Click "Save".

*Note*: breakpoints made from the user interface will have "customuser"
prepended to the name given to the breakpoint.

Breakpoints should be joined into groups for use with responsive styles or for
organization purposes. To create a group:

1. Click on the "Add new group" from the main Breakpoint page.
2. Enter a name for the group under "Group Name".
3. Select the breakpoints you wish to add to the group from the select list.
4. Click "Save".

A theme can define breakpoints in the theme.info file by adding a name for the
breakpoint and a valid media query, like so:

```
breakpoints[mobile] = (min-width: 0px)
breakpoints[narrow] = (min-width: 560px)
breakpoints[wide] = (min-width: 851px)
breakpoints[tv] = only screen and (min-width: 3456px)
```

*Note*: breakpoints made from a .info file will not be read until the theme has
been reloaded, either on the theme page by clicking "Reload theme" or by
disabling and enabling the theme.

## Usage

- Usage instructions can be [viewed and edited in the Wiki](https://github.com/backdrop-contrib/breakpoints/wiki).

The main usage of Breakpoints comes entirely outside of the module itself.
Because breakpoints defined either in the .info file or in the Breakpoints UI
are available with a call to the breakpoints module, developers and themers can
call on breakpoints in template.php (or in other modules).

Aside from the ability to store breakpoints, the Breakpoints module also can
convert stored Breakpoints to valid theme.info file format, using the following
steps:

1. Navigate to /admin/config/media/breakpoints.
2. Select a group of breakpoints from the tabs at the top of the screen.
3. Click "Export breakpoints to theme".

The results in the textbox on the resulting screen can then be copied to a
theme.info file, where they will be automatically implemented on any profile
running the respective theme.

*Note*: from this same screen, breakpoints may also be fully exported in a
feature-ready format using the "Export nodes" link.

## Recommended Modules

Breakpoints works hand-in-hand with a number of other modules for responsive
solutions, which is recommended. These modules include:

* [Picture](https://www.backdrop.org/project/picture)

## Issues

 - Bugs and Feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/breakpoints/issues).

## Current Maintainers

 - [Laryn Kragt Bakker](https://github.com/laryn).
 - Collaboration and co-maintainers welcome!

## Credits

 - Current development is supported by [Aten Design Group](https://aten.io).
 - Ported to Backdrop CMS by [Laryn Kragt Bakker](https://github.com/laryn).
 - Maintained for Drupal 7 by [Peter Droogmans](https://www.drupal.org/u/attiks)
 and [Jelle Sebreghts](https://www.drupal.org/u/jelle_s).

 ## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
