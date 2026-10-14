# Composer MIDICV v1.0 制作资料

[返回 README](../README.md) · [使用与校准手册](USER_MANUAL.md)

## 当前版本

当前主板为 v1.0，搭配 `hardware/ComposerPanel` 中的当前面板。
主板 BOM 共 38 组、86 个元件，其位号、数值和数量已与当前 PCB 核对。
当前面板生产包的各层 Gerber 图形已与当前 KiCad 文件重新导出的结果核对。

## 文件选择

| 文件 | 用途 |
| --- | --- |
| [主板 BOM](../hardware/ComposerMIDICV/production/COMPOSERv1.0_bom.csv) | 元件清单 |
| [主板 Gerber 与钻孔 ZIP](../hardware/ComposerMIDICV/production/COMPOSERv1.0Gerber.zip) | 主板加工 |
| [主板贴片坐标](../hardware/ComposerMIDICV/production/COMPOSERv1.0_positions.csv) | 贴装位置、角度和所在面 |
| [主板 KiCad 工程](../hardware/ComposerMIDICV/ComposerMIDICV.kicad_pro) | 源设计 |
| [原理图 PDF](Schematic.pdf) | 浏览电路 |
| [面板生产 ZIP](../hardware/ComposerPanel/production/ComposerPanel.zip) | 面板加工 |
| [面板 KiCad 工程](../hardware/ComposerPanel/ComposerPanel.kicad_pro) | 面板源设计 |
| [面板 SVG](../hardware/ComposerPanel/design/Composer_MIDI_CV_Panel_Silkscreen.svg) | 丝印图稿 |
| [面板 AI](../hardware/ComposerPanel/design/Composer_MIDI_CV_Panel_Silkscreen.ai) | 可编辑图稿 |
| [历史硬件](../hardware/legacy/) | 早期版本资料 |

## 采购与加工

主板和面板分别制作。不要将历史 BOM、Gerber 或坐标文件混入 v1.0 的下单资料。
使用工厂预览核对贴装面、元件方向、孔位和板框；板厚、材料及表面处理需按
实际制作要求选择，ZIP 文件没有记录完整的下单选项。

BOM 的 J1–J9 插孔和 RP2354A 需要另行确认采购来源。L3 的供应商字段是
厂家型号 `AOTA-B201610S3R3-101-T`，不是 LCSC 的 `C...` 编号。
L3 在 BOM 中写作 `0805`，在 PCB 中使用 `R_0805_2012Metric` 焊盘，采购时
需结合实际器件尺寸核对。电源排线、固定件等装配附件另行准备。

## 编辑工程所需的库

主板的 `libraries/` 保存当前 PCB 实际使用的 22 个自定义封装和原理图中
15 个 EasyEDA 符号，库表使用项目相对路径。AudioJacks、PCM_JLCPCB 和自定义
标志封装也包含在这些快照中，保留当前项目的修改。

使用 KiCad 10，并安装其标准符号/封装库。部分可选 3D 模型仍引用外部模型库
变量；没有这些模型不影响读取 2D 设计或使用已导出的生产文件。
第三方来源和许可见 [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md)。

## 装配后的检查

刷写当前 `firmware` 工程生成的 UF2，按 [使用手册](USER_MANUAL.md) 检查
四路 CV、四路 Gate、USB/TRS MIDI 和六个模式，再完成四路 CV 两点校准并保存。
历史 bring-up 程序的引脚分配不同，不能代替当前 v1.0 应用固件。
