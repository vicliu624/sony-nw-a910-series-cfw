# A918 拓麻歌子可行性复评（2026-10-18）

范围：源码研究及隔离的本地编译测试。没有更改 R8 候选固件，没有访问播放器，
没有下载商业游戏 ROM。R8 固件 SHA-256 仍为
`01cd1835b751f6c930ede95205adf010deeca49988ca8d53fd83dfd5565650e6`。

## 区分三个对象

1. Paramix Studio 是针对 Paradise 的 ROM 修改器，可把 S1C33 彩色旧机型模拟器、
   T7Emu、T18Emu 等整合到 Paradise 的固件中。不是可直接给 A918 安装的 Linux 游戏。
2. 移植这些模拟器的核心，让 A918 运行旧型号拓麻歌子，属于可研究路径。
3. 在 A918 上模拟 Paradise 本身，是另一项任务，需 ARMv7-M CPU 和 SNC73410
   外设模拟；不能由“Paradise 能运行旧机型模拟器”推导它也很容易。

## 本轮原始资料及版本

源码存于工程 tools/ 下，均为浅克隆，没有修改上游文件：

| 仓库 | 固定提交 |
| --- | --- |
| mameise/Paramix-Studio | 3676ecd5c49e9e40b118d59dde43cd0bc30fc4e6 |
| maragotchi/tamaemu | 79e19b65a1c970b9e1cd0b58ee6e929507257234 |
| jcrona/tamalib | ce304d55f9a73c60232ce3f552e7983db3fa399c |
| infinition/capybara | e80354eccba2124fdbfcf37d323a39a2104f3832 |

参考：
- https://github.com/mameise/Paramix-Studio/tree/3676ecd5c49e9e40b118d59dde43cd0bc30fc4e6/emu
- https://github.com/maragotchi/tamaemu
- https://github.com/jcrona/tamalib
- https://github.com/infinition/capybara

## 关键新证据：彩色核心可编译并运行于 ARMv5 模拟环境

Paramix `emu/src/emu/build.sh` 指定 Cortex-M3、Thumbv7-M、TC_JIT；`tc_jit.c`
产生 Thumb-2 并有 Cortex-M 专属汇编。A918 的 ARMv5 不能执行这些现成二进制。
但 `tc_all.c` 将内存、外设、LCD、核心控制和 C 快速解释器独立组织。
未定义 TC_JIT 时不包含 JIT，TC_HOST 可避免裸机专用初始化段设置。

本轮新增 `games/research/tama-core-smoke.c` 和 `build-tama-smoke.sh`，直接编译
上游未修改核心。测试数据为人工构造的4MiB零数组和复位向量，零指令是 NOP，
不是商业 ROM。执行了：

- 10万个合成 NOP 周期，并检查 PC 按预期前进。
- 模拟 RAM 的32位读写。
- 模拟按钮状态的按下/释放。
- LCD RGB565命令写入，检查红色像素结果。
- Host ASan/UBSan 运行通过。
- ARMv5TE soft-float、静态、无动态 libc 构建通过，QEMU ARM926 执行通过。

ARM ELF text 23732字节，data为0，BSS约4.16MiB（主要为合成ROM数组）。
GCC 对上游 minsize 属性发出忽略警告；未改上游代码压制它。

这比仅凭“C语言应该能移植”更强，但不能证明完整指令集正确、真实 ROM 能启动、
外设/存档兼容、屏幕适配完成或达到实时速度。QEMU 在电脑上的执行耗时不是 A918
性能证据，也不能据此给帧率或成功概率。

原始结果：`work/game-research/tama-smoke/result.json`；ARM ELF 哈希
`f311317acd0309ba19594deb919a624c49bfe155240f58cacf6abe3992f92d59`。

## 路线比较

| 目标 | 源码依据 | A918 当前评估 |
| --- | --- | --- |
| 初代黑白 P1/P2 | TamaLIB，独立HAL，32×16点阵和图标 | 最低难度，适合验证真实养宠运行路径；本轮没有编译或运行它 |
| Tama-Go / Friends | Paramix T7 核心，6502族，标称4MHz；实际显示宏48×31灰度 | 值得作为中间路线；Friends上游仍标为测试支持，本轮未编译 |
| Plus Color / iD L / P's / 4U等 | tamaemu及Paramix S1C33解释器 | 有实质移植依据；本轮只对Plus Color配置跑合成测试，真实兼容和性能未知 |
| Paradise本身 | Capybara SNC73410/ARMv7-M模拟器 | 难度明显更高，不宜作为首个目标；不是已证明做不到 |

TamaLIB有设备无关的按钮、时钟、屏幕、声音HAL；上游列出MCU移植。
T7文件头的物理64×32与实际显示宏48×31不是同一口径，不应随意混用。
Capybara当前Cargo依赖含eframe/egui/wgpu、桌面窗口、rodio等，不能把现成桌面程序
搬到A918。必须剥离核心并重新适配。源码还包含16MiB的guest flash模型。
具体模拟计算量和A918可用内存仍需实测。

## 彩色版本还缺什么

### 显示与按键

S1C33核心输出128×128 RGB565，A918布局为240×320，可原尺寸显示，或做192×192
等非整数缩放；2倍256×256无法完整放入240像素宽度，不能直接承诺2倍整数放大。
实际QFB输出仍依赖R8选择器的设备验证，不能用核心LCD数组测试替代。
可映射左=A、中间=B、右=C，其他方向用于宿主菜单；这只是方案，未改现有游戏。
俄罗斯方块的“中间结束本局”只属于俄罗斯方块，不适合原样套给拓麻歌子B键。

### 内存与速度

本轮实际读取ARM32构建的sizeof：Emu=65160、GRAM=32768、overlay=65536字节。
4MiB ROM加这些约4.16MiB，8MiB ROM约8.16MiB；不包括宿主帧缓冲、栈、代码、
文件缓存或额外存档副本。ROM按需映射可研究，但不能把这些估算直接当峰值RSS。
A918当前实际空闲RAM及频率没有测量，本报告不假定64MiB或某个MHz数。
Plus Color配置osc3_hz=20000000，iD L=18432000；这是guest某时钟参数，不是说它
任何时刻都持续执行相同数量指令，低功耗/睡眠会改变工作量。解释器无JIT时，
菜单、购物、动画等忙场景的实时速度是必须测试的重点。

### 存档、时间及外围

宠物游戏需要持久状态，Tetris不存档的要求不能直接推广为宠物也不存档。
ROM原文件应保持只读，guest flash的写入及erase需要可写overlay和文件后端，
保存后也要有退出重进验证。当前核心只有8个8KiB overlay槽；必须实现槽满时
的evict及64KiB erase回调，否则可能丢写入或停住，不能仅接好画面就算完成。

退出到播放器后宠物进程不再运行；“冻结养成”或“按离开时间追赶”需要明确策略。
RTC校正不等于把期间所有游戏逻辑都正确执行。首版可先冻结，但尚未替用户选定。
声效应作为后续单独适配；红外/NFC、联机与硬件附件不能因模拟CPU成功就宣称可用。
独立程序不修改索尼音频库有助隔离风险，但不等于音质、驱动交接已实测。

## 推荐验证次序

1. 先把R8启动选择、游戏退出、USB交接在设备上验证，仍未安装或实测。
2. 本地独立宿主加载用户自己的目标机型ROM，先验证孵化、按钮、动画、flash写入
   和退出重进。上游不附带ROM，本轮没有下载ROM。
3. 静态ARMv5构建，用A918真实场景计时测忙场景速度、内存和退出恢复。
4. 达到可用标准后才接入Games列表、再单独构建候选固件。现阶段不改R8。

结论：彩色旧机型从“理论上可移植”推进到“核心能以ARMv5程序执行合成检查”；
完整游戏和实机速度仍未证明。Paradise本体保持独立的高难度研究路线。
