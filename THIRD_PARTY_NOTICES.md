# Third-party notices

The project author's original firmware, design source and documentation use
AGPL-3.0-only; see [LICENSE](LICENSE). Third-party material retains its own licence.

## Raspberry Pi Pico SDK and TinyUSB

The SDK is a Git submodule pinned to Pico SDK 2.2.0. Its BSD 3-Clause notice is
retained in [docs/licenses/Pico-SDK-BSD-3-Clause.txt](docs/licenses/Pico-SDK-BSD-3-Clause.txt).
The SDK's upstream dependency notices remain in the submodule. TinyUSB uses MIT;
see [docs/licenses/TinyUSB-MIT.txt](docs/licenses/TinyUSB-MIT.txt).

## AudioJacks

The AudioJacks footprint snapshots are derived from
[clacktronics/AudioJacks](https://github.com/clacktronics/AudioJacks), upstream
revision `14a88866e93b8ce4a31ad376b0c6eb85cd4d2cf3`, with the project-specific
changes present in the v1.0 PCB. The MIT licence and copyright notice are retained
in [docs/licenses/AudioJacks-MIT.txt](docs/licenses/AudioJacks-MIT.txt).

## JLCPCB KiCad library

The PCM_JLCPCB footprints originate from
[CDFER/JLCPCB-Kicad-Library](https://github.com/CDFER/JLCPCB-Kicad-Library).
Current-board snapshots retain the MIT notice in
[docs/licenses/JLCPCB-library-MIT.txt](docs/licenses/JLCPCB-library-MIT.txt).
Supplier data is retained for component identification; stock and prices in
footprint metadata are historical, not current purchasing information.

## KiCad libraries and imported component data

Standard symbols and footprints use the installed KiCad 10 libraries. Their
[CC-BY-SA 4.0 licence and electronic-design exception](https://www.kicad.org/libraries/license/)
remain applicable; the standard libraries are not vendored here.
The EasyEDA library snapshots retain the symbols and footprints used in this
project's design, including original part identifiers and datasheet references.
Optional 3D models are referenced by their original external library variables;
they are not included in the snapshots.
