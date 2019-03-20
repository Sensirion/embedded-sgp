# Changelog

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

 * [`changed]` Rename factory baseline to inceptive baseline
 * [`changed`] Prefix all methods with `sgp30_` / `sgpc3_` instead of `sgp_`
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

[Unreleased]: https://github.com/Sensirion/embedded-sgp/compare/3.1.4...master
[3.1.4]: https://github.com/Sensirion/embedded-sgp/compare/3.1.3...3.1.4
[3.1.3]: https://github.com/Sensirion/embedded-sgp/compare/3.1.2...3.1.3
[3.1.2]: https://github.com/Sensirion/embedded-sgp/compare/3.1.1...3.1.2
[3.1.1]: https://github.com/Sensirion/embedded-sgp/compare/3.1.0...3.1.1
[3.1.0]: https://github.com/Sensirion/embedded-sgp/compare/3.0.0...3.1.0
[3.0.0]: https://github.com/Sensirion/embedded-sgp/compare/2.5.0...3.0.0
[2.5.0]: https://github.com/Sensirion/embedded-sgp/releases/tag/2.5.0
