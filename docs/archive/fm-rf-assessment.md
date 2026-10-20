# A910 FM / RF assessment — 2026-10-18

## Conclusion

Software contains an FM stack, but available evidence does not establish that A910 boards contain a working FM tuner, the required frequency coverage, demodulation mode and audio route. FM has not been enabled. A local 11-item menu reserves FM and explicitly says hardware unverified. It must not claim reception or emulate a successful station scan.

## Primary evidence

Sony original A916/A918/A919 manual (specifications p84/85):
https://www.sony.jp/ServiceArea/impdf/pdf/3275158041.pdf

It specifies a One Seg tuner with television channels 13–62 and a whip antenna. It does not provide an FM tuning band, an FM operating section or an FM chipset. This establishes the supported product mode, not a complete inventory of hidden silicon capabilities.

Sony official product/press pages:
https://www.sony.jp/walkman/products/archive/NW-A910_series/
https://www.sony.jp/CorporateCruise/Press/200709/07-0927/

The firmware identifies A91x as Daisy/ICX1101 and S71xF as IrisAADNC/ICX1111 in mc.sh. The home program contains FM pages and /dev/v4l/radio0 strings. icx1111_radio_tuner.ko is a separate Sony driver whose strings name Sanyo FM Tuner. The kernel contains icx1111_radio_base code and audio mixer tuner input strings. Startup reads sysinfo fm and only loads that separate driver when ICX_FM_ON=1. The One Seg path independently loads dtv.ko and I2C modules; dtv.ko is a stream/DMA device, not evidence of analog FM demodulation support.

The shared stack also includes other machine capabilities (BT/WiFi checks), so presence of code alone is not a board capability measurement. A UHF digital-TV tuning path does not by itself establish VHF broadcast-FM reception. Both RF coverage and FM demodulation/audio routing must be confirmed. No exact A910 tuner IC/datasheet or usable schematic was found in this search, so do not assert it is physically absent, nor promise a firmware-only unlock.

## Conflicting secondary reports

Some 2007 news reports explicitly claim FM (Newlaunches, Engadget and a Sina/ZOL reprint). They do not identify an RF part or provide an A910 FM procedure, and conflict with the official feature/manual evidence. They cannot establish hardware feasibility. They remain leads rather than primary technical evidence. Do not cite their claim as a verified feature.

## Further evidence required

1. Read existing board capabilities using sysinfo fm/1seg and module/device inventory; games/rf-readonly-probe.sh is prepared but NOT executed.
2. Identify the RF front-end/tuner from a schematic or readable board markings, then inspect its datasheet for supported bands and modes.
3. If FM hardware exists, determine power, control bus, antenna and audio path before loading or tuning it.
4. Only after this, perform a controlled receiver test on the physical device. User currently requested local development, so no live probing is authorized by that scope.

Until then the implementation state is "FM entry reserved; hardware and reception not verified", not "FM enabled".
