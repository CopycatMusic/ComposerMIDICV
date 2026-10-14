# Composer MIDICV v1.0 使用手册

本手册适用于 `hardware/ComposerMIDICV` 中的 **v1.0 主板**、当前面板，以及默认配置为 `midi1–3`、`sequencer1–3` 的固件。作者已实机验证 USB MIDI、TRS MIDI、四路 CV、四路 Gate、六个模式及电压校准均可使用。

[返回项目首页](../README.md)

## 1. 接口与供电

| 接口 | 用途 |
| --- | --- |
| USB-C | 接收电脑/USB 主机发送的 MIDI；提供 CDC 串口调试和校准接口 |
| TRS MIDI IN | 接收外部设备的 MIDI，3.5 mm TRS Type A 接线 |
| CV1–4 | 0–8 V 单极性控制电压，功能随模式变化 |
| Gate1–4 | 标称 0/5 V 的 Gate、Trigger、Run 或 Clock 输出 |
| 两个模式开关 | 选择六个演奏模式 |
| RGB LED | 显示音符/声部、模式切换和超范围状态 |
| P1：10-pin 排线接口 | 模块电源输入 |

### 1.1 模块电源

v1.0 从 P1 的 **+12 V** 输入产生板上电源。当前 PCB 的 P1 引脚连接如下，编号以 PCB/连接器的 Pin 1 标记为准：

| P1 引脚 | v1.0 连接 |
| --- | --- |
| 1、2 | 未连接，对应标准 Eurorack 排线的 −12 V 一端 |
| 3–8 | GND |
| 9、10 | +12 V 输入，经 D1 接入板上电源 |

连接或拆除电源排线前关闭机箱电源，核对插座的 Pin 1 和排线方向。标准 Eurorack 排线红边对应 −12 V 一端；本板该端未使用，仍需按正确方向安装。标准接口背景可参考 [Doepfer A-100 技术说明](https://doepfer.de/a100_man/a100t_e.htm)。

**USB-C 是数据接口，不能替代 v1.0 的模块电源。** 不要根据历史版本的 USB 供电说明连接当前主板。

### 1.2 MIDI 输入

USB 连接需要支持数据传输的 USB-C 线。系统会识别 **Composer MIDICV**；MIDI 接口字符串为 **MIDICV MIDI**，调试接口字符串为 **MIDICV Debug**，不同系统可能显示其中之一或组合名称。

模块是 **USB Device**：电脑或独立 USB Host 向模块发送 MIDI。USB-only 键盘不能直接作为主机驱动模块；可经电脑路由，或使用键盘的传统 MIDI OUT 接入 TRS 输入。

当前 PCB 中，J9 的 Ring 经 R18 接光耦阳极，Tip 接光耦阴极。这与 **TRS Type A** 极性对应；Type A 转接线映射如下，可参考 [ESI 官方接线说明](https://kb.esi-audio.com/Default.asp?goto=KB00315EN)：

| 3.5 mm TRS | 5-pin DIN MIDI |
| --- | --- |
| Tip | Pin 5 |
| Ring | Pin 4 |
| Sleeve | Pin 2 |

连接的是 MIDI 信号，不是音频。使用 DIN MIDI 源时选 Type A 转接线；Type B 的 Tip/Ring 顺序相反。

USB 和 TRS 事件都进入当前模式。首次使用建议只启用一个 MIDI 来源，便于确认通道和音符路由；两个来源同时工作时避免向同一声部重复发送同一组音符。时钟同步使用一个主时钟来源。

## 2. 第一次使用

1. 关闭机箱电源，连接模块电源排线并核对方向。
2. 接通电源，再通过 USB-C 连接电脑，或通过 TRS MIDI IN 连接 MIDI 源。
3. 选择模式 1 `midi1`。
4. 将 **CV1** 接到振荡器的 **1 V/oct 音高输入**，将 **Gate1** 接到包络发生器的 **Gate 输入**。
5. 在 DAW 的 MIDI 轨选择 Composer 对应的 MIDI 输出端口，使用 MIDI 通道 1。
6. 发送 MIDI note **36、48、60**。对应音高电压约为 **0、1、2 V**；包络随按键开关。

模块输出控制信号。声音由接入的振荡器、包络、VCA 等合成器模块产生。不同 DAW 对八度的文字命名不同，核对音高时以 **MIDI note number** 为准。

## 3. 选择模式与分配 MIDI 通道

### 3.1 六个开关组合

SW3 提供两组模式，SW4 在每组中选择 A / 中间 / B。下表中的 SW3 状态指电气上的开路/闭合，SW4 A/B 指对应接点闭合；不要仅凭开关手柄的左右方向猜测档位。

| 模式 | 名称 | SW3 | SW4 |
| --- | --- | --- | --- |
| 1 | `midi1` | 开路 | A |
| 2 | `midi2` | 开路 | 中间 |
| 3 | `midi3` | 开路 | B |
| 4 | `sequencer1` | 闭合 | A |
| 5 | `sequencer2` | 闭合 | 中间 |
| 6 | `sequencer3` | 闭合 | B |

切换模式后，固件清空输出并重置该模式的音符状态，LED 短暂闪白。需要重新发送音符。调试串口的 `status` 命令可查看当前模式名称；`index` 和 `selector` 使用从 **0** 开始的编号，对应面板模式 1–6 的值为 0–5。

### 3.2 通道决定声部

改变 MIDI 通道不会切换模式，而是选择当前模式中的声部：

| 模式 | 声部 | MIDI 通道 |
| --- | --- | --- |
| `midi1` | 单声部 | 1–16 全部进入同一声部 |
| `midi2`、`sequencer1–3` | Voice 1 | 1–8 |
| `midi2`、`sequencer1–3` | Voice 2 | 9–16 |
| `midi3` | Voice 1 | 1–4 |
| `midi3` | Voice 2 | 5–8 |
| `midi3` | Voice 3 | 9–12 |
| `midi3` | Voice 4 | 13–16 |

双声部和四声部模式中，每个通道组共用一个声部，同组新的 Note On 替换该声部当前音符。要同时输出四个音高，可分别向通道 **1、5、9、13** 发送音符；在单一 MIDI 通道弹和弦不会自动分配到四路输出。

## 4. 六个模式的输出

### 4.1 模式 1：midi1

| 输出 | 功能 |
| --- | --- |
| CV1 | 单声部音高，1 V/oct，支持 Pitch Bend |
| CV2 | 当前音符的 Velocity |
| CV3 | CC1 调制电压 |
| CV4 | 每次 Note On 更新一次的随机 Sample & Hold 电压 |
| Gate1、Gate2 | 音符持续期间为高电平 |
| Gate3、Gate4 | 每次 Note On 产生 10 ms 触发脉冲 |

音符采用最后按下优先：按住第一个音再按第二个音，CV1 转到第二个音；松开第二个音会返回仍按住的前一个音。所有音符松开后 Gate1/2 拉低，CV1 保留最后的音高电压。

示例：CV1 → VCO 1 V/oct，Gate1 → 包络 Gate，CV2 → 力度控制的 VCA/调制输入，CV3 → 带衰减的滤波器调制输入，CV4 → 随机调制目标。

### 4.2 模式 2：midi2

| 输出 | 功能 |
| --- | --- |
| CV1、CV2 | Voice 1 的 Pitch / Velocity |
| CV3、CV4 | Voice 2 的 Pitch / Velocity |
| Gate1、Gate2 | Voice 1 的持续 Gate / 10 ms Note On Trigger |
| Gate3、Gate4 | Voice 2 的持续 Gate / 10 ms Note On Trigger |

DAW 轨道 A 使用通道 **1**，轨道 B 使用通道 **9**，即可独立控制两个音高/力度声部。对应 Note Off 关闭该声部的 Gate 并将 Velocity 归零；Pitch 保持最后值。每个声部的 Pitch Bend 由其通道组的消息控制。

### 4.3 模式 3：midi3

| 输出 | 功能 |
| --- | --- |
| CV1、Gate1 | Voice 1 Pitch / Gate，通道 1–4 |
| CV2、Gate2 | Voice 2 Pitch / Gate，通道 5–8 |
| CV3、Gate3 | Voice 3 Pitch / Gate，通道 9–12 |
| CV4、Gate4 | Voice 4 Pitch / Gate，通道 13–16 |

可设置四条 DAW MIDI 轨，分别使用通道 **1、5、9、13**，将四路 CV/Gate 接入四套声音链。此模式不单独输出 Velocity，也不自动对同一通道的和弦做声部分配。

### 4.4 模式 4–6：sequencer1–3

三个模式的 CV 分配相同，均由 MIDI 音符直接控制两个 Pitch/Velocity 声部；它们不在模块内生成音符序列。

| 输出 | 功能 |
| --- | --- |
| CV1、CV2 | Voice 1 Pitch / Velocity，通道 1–8 |
| CV3、CV4 | Voice 2 Pitch / Velocity，通道 9–16 |
| Gate1 | Run：收到 Start/Continue 后为高，收到 Stop 后为低 |
| Gate2 | Reset：收到 Start 时产生 10 ms 脉冲 |

Gate3/4 的时钟分频随模式变化：

| 模式 | Gate3 | Gate4 |
| --- | --- | --- |
| 4：`sequencer1` | 1 PPQN | 8 PPQN |
| 5：`sequencer2` | 2 PPQN | 12 PPQN |
| 6：`sequencer3` | 4 PPQN | 24 PPQN |

PPQN 表示每四分音符的脉冲数。MIDI Clock 为 24 PPQN：1/2/4/8/12/24 PPQN 分别每 24/12/6/3/2/1 个 MIDI Clock tick 触发一次。脉冲宽度为 10 ms，极快时钟下相邻脉冲可能连接为持续高电平。

在 DAW 或 MIDI 源中启用发往 Composer 的 **MIDI Clock 和 Transport**：

- **Start**：Run 拉高，产生 Reset，时钟计数从头开始。
- **Continue**：Run 拉高，保留当前计数，不产生 Reset。
- **Stop**：Run 拉低，清除时钟和 Reset 脉冲。

只有 Clock 而没有 Start/Continue 时，时钟 Gate 不输出。音高和力度仍可由 MIDI 音符更新。DAW 播放期间切换回 sequencer 模式会恢复 Run 高电平，但不会补发 Reset；需要重新对齐外部序列器时发送新的 Start。当前固件不处理 Song Position Pointer。

示例：Gate1 → 外部序列器 Run，Gate2 → Reset，Gate3 或 Gate4 → Clock；CV1/CV3 控制两个 VCO，CV2/CV4 作为力度调制。与接收模块匹配脉冲频率和输入功能。

## 5. 电压与 LED

### 5.1 电压规则

- CV 为单极性，固件输出范围 **0–8000 mV**。
- Pitch 以 **MIDI note 36 = 0 V** 为基准，每升高 12 个半音增加 1 V；note 48/60/72 分别对应 1/2/3 V。
- Pitch Bend 固定约 **±2 半音**，不通过 RPN 改变范围。
- Velocity 和 CC1 的 0–127 线性映射为 0–8 V；Velocity 为 0 的 Note On 按 Note Off 处理。
- 超出电压范围的音高被限制在输出边界。低于 note 36 的音符会出现低端限制和 LED 超范围提示。
- Note Off 后保留音高电压属于正常行为，是否继续发声由 Gate/包络和声音链决定。

Velocity、CC1 和随机 CV 可能达到 8 V。接入目标模块时按其控制输入范围使用衰减器。

### 5.2 LED 状态

| 状态 | 显示 |
| --- | --- |
| `midi1` 有音符 | 按音级显示颜色 |
| 双声部 / sequencer | Voice 1 为红粉色，Voice 2 为绿色；同时激活时混色 |
| `midi3` | 四个声部分别为红粉、绿、蓝、黄；同时激活时混色 |
| 无活动音符 | 常态熄灭 |
| 模式切换 | 短暂闪白，约 240 ms |
| sequencer 的 Start/Continue/Stop | 短暂蓝色提示，约 240 ms |
| 超范围音符 | 对应颜色以约 120 ms 亮 / 120 ms 灭闪烁，直到相关音符状态更新 |

LED 的常态主要反映 MIDI 音符活动，不是电源灯。无音符时熄灭不表示模块未供电。

## 6. 固件构建与刷写

已经安装可用固件时可直接使用；需要更新或修改固件时再执行本节。

### 6.1 构建

准备 CMake 3.22+、Ninja、Python 3、包含 newlib/`nosys.specs` 的 Arm GNU 嵌入式工具链。当前本地环境使用 **14.2.Rel1**，SDK 为 **Pico SDK 2.2.0**。确保 `arm-none-eabi-gcc` 等工具在 `PATH` 中。

在项目根目录初始化 SDK 子模块：

```sh
git submodule update --init --recursive third_party/pico-sdk
```

macOS / Linux：

```sh
export PICO_SDK_PATH="$PWD/third_party/pico-sdk"
cmake -S firmware -B firmware/build-arm -G Ninja \
  -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build firmware/build-arm --parallel
```

Windows PowerShell：

```powershell
$env:PICO_SDK_PATH = Join-Path $PWD "third_party/pico-sdk"
cmake -S firmware -B firmware/build-arm -G Ninja -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2 -DCMAKE_BUILD_TYPE=Release
cmake --build firmware/build-arm --parallel
```

构建输出为 `firmware/build-arm/composer_midicv.uf2`，同时复制到 `firmware/dist/composer_midicv.uf2`。这里的 `pico2` 是自定义 RP2354A 主板使用的 SDK 构建配置，实际引脚由固件定义。旧版本的 bring-up 测试程序使用不同引脚，应使用当前 `firmware` 工程生成的 UF2。

### 6.2 刷写

1. 停止 MIDI 播放，保持模块电源可用，通过 USB 数据线连接电脑。
2. 按住板上的 **BOOTSEL**，按下并松开 **RUN** 复位键，再松开 BOOTSEL。
3. 电脑出现 RP2350 USB 启动盘，常见名称为 **RP2350**。
4. 将 `composer_midicv.uf2` 复制到该盘。复制完成后启动盘消失，模块自动重启并重新出现 MIDI/CDC 接口。
5. 重新选择 DAW MIDI 端口，发送音符确认输出。

从完全断电状态启动时，也可先按住 BOOTSEL，再接通模块电源并连接 USB。由于 v1.0 由机箱供电，单纯拔插 USB 不一定会复位模块。

## 7. 串口调试

打开模块对应的 USB CDC 串口：macOS 通常为 `/dev/cu.usbmodem...`，Linux 通常为 `/dev/ttyACM...`，Windows 为 `COM...`。使用支持 CDC 的串口终端，启用 DTR，发送以换行结尾的命令。可使用 115200、8N1；USB CDC 固件不以该波特率设置物理 UART。

测试前暂停 DAW，并停止或断开 MIDI 输入，避免新的 MIDI 事件覆盖测试输出。

| 命令 | 用途 |
| --- | --- |
| `status` | 查看当前模式、选择器、CV 目标值、Gate 和校准载入状态 |
| `mute` | 清空当前输出并取消 raw DAC 覆盖；新的 MIDI 事件仍可恢复输出 |
| `test_gate <1-4>` | 指定 Gate 输出约 100 ms 测试脉冲 |
| `test_cv <1-4> <mv>` | 指定通道输出目标电压，单位 mV，经过当前校准 |
| `cal show` | 查看四路校准点，显示目标值/测量值 |
| `cal reset` | 将四路校准恢复为默认值，暂不写入 flash |
| `cal raw <1-4> <code>` | 直接指定 DAC code 0–4095，绕过校准 |
| `cal set <1-4> <target_mv> <measured_mv>` | 录入该通道的一个校准点 |
| `cal save` | 将全部当前校准点保存到 flash |
| `led auto` | 恢复自动 LED 反馈 |
| `led off` | 暂时关闭 LED 自动反馈 |
| `led <r> <g> <b>` | 设置 RGB，各通道 0–255 |
| `led cycle [ms]` | LED 测试循环，可选间隔 20–60000 ms |

`status` 中的 `cv` 是固件请求的电压，不是对输出插孔的实测读数。确认电压需使用万用表或合适的测量设备。

执行 `cal raw` 后，该通道持续使用原始 DAC code。用同通道的 `test_cv`、`mute` 或 `cal reset` 退出 raw 覆盖；使用 `led auto` 退出 LED 测试。

## 8. CV 电压校准

四路 CV 分别进行两点线性校准。准备可靠的电压测量设备，黑表笔接模块 GND，红表笔测对应 CV 输出。校准时停止 MIDI 输入，并让模块电源和测量条件保持稳定。

### 8.1 采集两点

若重新校准全部通道，先执行一次：

```text
mute
cal reset
```

`cal reset` 会重置**全部四路**的当前校准。不要在已经校准通道 1 后，为校准通道 2 再次执行它。

以 CV1 为例，先测量两个目标电压，并记录实际值：

```text
test_cv 1 1000
test_cv 1 6000
```

执行第一条后记录约 1 V 的实际读数；执行第二条后记录约 6 V 的实际读数。两次都测完后再录入校准点，避免中途修改校准改变第二次测量的基准。低点选在 0–4000 mV，高点选在 4000 mV 以上；1 V 和 6 V 可避开输出上下边界。

### 8.2 录入并检查

例如，**假设**两次实际读数分别是 1003 mV 和 5987 mV，则录入：

```text
cal set 1 1000 1003
cal set 1 6000 5987
cal show
```

最后两个测量值必须换成你的实际读数。`target_mv` 是测试时要求的电压，`measured_mv` 是当时测得的电压，不要填反。

用 `test_cv 1 2000`、`test_cv 1 4000` 等命令测量中间点，检查校准后的输出。按相同步骤完成 CV2、CV3、CV4，将通道编号分别改为 2、3、4。

### 8.3 保存

确认四路结果后执行：

```text
cal save
mute
```

校准参数带 CRC 保存到 flash，重启时自动载入。重启后用 `cal show` 核对两点，用 `status` 查看 `cal=flash`，再发送 MIDI 音符检查 1 V/oct 输出。修改校准点但没有执行 `cal save` 时，重启会载入上一次保存的参数。

若需要永久恢复默认值，执行 `cal reset` 后再执行 `cal save`。

## 9. 硬件资料与制作

制作当前模块时，主板使用 **v1.0** 生产文件，面板使用当前 `ComposerPanel/production` 文件：

| 文件 | 作用 |
| --- | --- |
| [主板 KiCad 工程](../hardware/ComposerMIDICV/ComposerMIDICV.kicad_pro) | 查看/编辑主板设计 |
| [主板原理图](../hardware/ComposerMIDICV/ComposerMIDICV.kicad_sch) | 查看电路及连接 |
| [v1.0 BOM](../hardware/ComposerMIDICV/production/COMPOSERv1.0_bom.csv) | 元件位号、封装、数量、型号及供应商字段 |
| [v1.0 Gerber ZIP](../hardware/ComposerMIDICV/production/COMPOSERv1.0Gerber.zip) | 主板各层及钻孔生产文件 |
| [v1.0 贴片坐标](../hardware/ComposerMIDICV/production/COMPOSERv1.0_positions.csv) | 元件坐标、旋转角度与所在面 |
| [面板 KiCad 工程](../hardware/ComposerPanel/ComposerPanel.kicad_pro) | 查看/编辑面板设计 |
| [面板生产 ZIP](../hardware/ComposerPanel/production/ComposerPanel.zip) | 当前面板生产文件 |
| [面板图稿](../hardware/ComposerPanel/design/Composer_MIDI_CV_Panel_Silkscreen.svg) | 面板视觉设计 |
| [历史版本](../hardware/legacy/) | v0.3 及更早导出资料 |

联系作者：Copycat， [tg3299182383@gmail.com](mailto:tg3299182383@gmail.com)。
