# Changelog

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

* [`changed`] Move the i2c init call out of `probe()` and into the examples
* [`changed`] Faster `sgp30_measure_raw_blocking_read` for FS1.0+. Now 25ms
              instead of 200ms.
* [`changed`] CFLAGS: Enable strict aliasing warnings by default, add `-Os` to
              SGPC3.
* [`changed`] Avoid the use of a 64b temporary when converting absolute humidity
* [`changed`] Split out `default_config.inc` from Makefile to configure paths
              and CFLAGS for SGP30, SGPC3 and SVM30 drivers
* [`changed`] Only one example with either `hw_i2c` or `sw_i2c` is built,
              depending on `CONFIG_I2C_TYPE`. Defaults to `hw_i2c`.
* [`removed`] No longer support SGP30 with feature set < 1.0
* [`changed`] Update submodule to increase timeout while clock stretching in
              software I2C mode
* [`fixed`]   Run `make prepare` in the `embedded-sht` directory when makeing
              the `release` target in order for the `release/svm30` target to
              succeed.
* [`removed`] Remove the `AUTHORS` file from the driver and the
              `embedded-common` submodule, as it adds more noise than benefit.
              The contributors can be found in the git log.
* [`fixed`]   Copy correct `CHANGELOG.md` and `LICENSE` files to target
              locations when running the `release` target of the driver's root
              Makefile.
* [`fixed`]   Fix `cpp` build of SVM30 due to missing header file
* [`changed`] Don't use variable-length arrays (improves portability)
* [`changed`] Change the quite complicated feature set handling code to easy to
              understand constants for SGP30 and SGPC3
* [`changed`] Change the SGP30 error code `SGP30_ERR_UNKNOWN_FEATURE_SET` to
              `SGP30_ERR_INSUFFICIENT_FEATURE_SET`
* [`added`]   Add `SGPC3_ERR_INSUFFICIENT_FEATURE_SET` as error code for SGPC3

## [5.0.0] - 2019-05-17

* `[added]` `.clang-format` Style-file for clang-format and helper targets
            `make style-fix` and `make style-check` (#36)
* `[changed]` Use stdint types (e.g. `uint16_t` instead of `u16`)

## [4.0.0] - 2019-03-27

 * [`changed`] Rename factory baseline to inceptive baseline
 * [`changed`] Prefix all methods with `sgp30_` / `sgpc3_` instead of `sgp_`
 * [`changed`] Unify naming of measurement, initialization and baseline methods
 * [`fixed`] Feature set check for SVM30
 * [`fixed`] Rebuild when example source code changed

## [3.1.4] - 2018-11-28

 * [`fixed`] Store basline every hour in SGPC3 example
 * [`fixed`] Potential issues caused by strict aliasing violations

## [3.1.3] - 2018-08-16

 * [`added`] Driver for SVM30 modules

## [3.1.2] - 2018-08-09

 * [`fixed`] `sgp_set_absolute_humidity` on SGP30

## [3.1.1] - 2018-07-20

 * [`changed`] Clarify factory baseline documentation

## [3.1.0] - 2018-07-19

 * [`fixed`] Potential alignment issues
 * [`fixed`] Readout of the serial ID
 * [`added`] Support for factory baseline

## [3.0.0] - 2018-04-30

 * [`changed`] Increase `set_baseline_wait` duration to 10ms
 * [`changed`] Do not scale ethanol and h2 raw signals

## [2.5.0] - 2018-03-21

 * First public release

[Unreleased]: https://github.com/Sensirion/embedded-sgp/compare/5.0.0...master
[5.0.0]: https://github.com/Sensirion/embedded-sgp/compare/4.0.0...5.0.0
[4.0.0]: https://github.com/Sensirion/embedded-sgp/compare/3.1.4...4.0.0
[3.1.4]: https://github.com/Sensirion/embedded-sgp/compare/3.1.3...3.1.4
[3.1.3]: https://github.com/Sensirion/embedded-sgp/compare/3.1.2...3.1.3
[3.1.2]: https://github.com/Sensirion/embedded-sgp/compare/3.1.1...3.1.2
[3.1.1]: https://github.com/Sensirion/embedded-sgp/compare/3.1.0...3.1.1
[3.1.0]: https://github.com/Sensirion/embedded-sgp/compare/3.0.0...3.1.0
[3.0.0]: https://github.com/Sensirion/embedded-sgp/compare/2.5.0...3.0.0
[2.5.0]: https://github.com/Sensirion/embedded-sgp/releases/tag/2.5.0
