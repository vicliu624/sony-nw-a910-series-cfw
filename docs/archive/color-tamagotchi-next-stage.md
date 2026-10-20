# P1成功后的彩色路线（2026-10-18）

用户反馈：“拓麻歌子P1成功了，继续考虑其他彩色版本”。这是P1实机成功反馈，
不是对Tetris、USB、存档断电及退出所有行为的逐项确认。当前交付版本为R9 ALPHA3，
本轮没有修改或安装它，也没有生成新的UPG。

## 基线

保留releases/R9-P1-Icons-ALPHA3。UPG SHA256：
18d0fde3dd9937b8b31b2bd59e0218b7f7912aeaa306c71689e9f83cfe679a8b。
实机报告单独存user-validation.json，不把离线验证报告的device_tested整体改成true。

P1成功使OABI系统调用、启动选择、已有显示/按键路径有了实际使用依据，但彩色
S1C33模拟计算量和RAM需求明显不同，不能据此推断实时速度足够。

## 目标顺序建议

| 目标 | 已读源码ROM配置 | 理由/限制 |
| --- | --- | --- |
| Plus Color | 4MiB，Paramix有独立profile | 第一候选；减少初期内存占用，不等于已经测得更快 |
| 原版iD、iD Melody | 4MiB，tamaemu有独立profile | 同属较小目标，但flash布局、debug引脚及sleep配置不能照抄Plus Color |
| iD L | 8MiB，Paramix有独立profile | 下一阶段候选，需真实ROM忙场景测试 |
| P's | 8MiB | 配置包含不同sleep标志，不能只按容量套iD L |
| 4U/4U+ | 8MiB，tamaemu配置30MHz OSC3 | 另有NFC外围模拟；初期不承诺实体NFC或联机 |
| Meets/On、Paradise | 另一套CPU/外设核心 | 不属于这条S1C33移植路线，保持后续独立研究 |

容量/配置依据为固定版本源码，不是对硬件频率的实测：
https://github.com/maragotchi/tamaemu/blob/79e19b65a1c970b9e1cd0b58ee6e929507257234/src/device.c
https://github.com/mameise/Paramix-Studio/blob/3676ecd5c49e9e40b118d59dde43cd0bc30fc4e6/emu/src/core/tc_core.c

当前公开源码都支持多个彩色机型，但“上游支持”不是“A918已经支持”。
其Thumb-2 JIT不适用于A918，第一版应继续使用C解释器。

## 本轮实际新增验证

games/research/color-storage-smoke.c直接使用未改动的Paramix解释器，测试数据
全部合成，没有下载或运行彩色商业ROM。覆盖Plus Color的4MiB和iD L的8MiB profile。

1. 两种profile的合成复位向量和10万个NOP周期，RAM及RGB565红/绿/蓝像素、按钮状态。
2. 负对照：八个8KiB overlay槽无回写时，超过容量确实记录丢写入。
3. RAM工作镜像的evict和64KiB erase回调接入后，写20个8KiB区域、触发至少12次
   eviction，再逐个读回一致，flash_lost_writes保持0。
4. NOR按位AND编程、64KiB擦除、8KiB参数区擦除及邻区不受影响检查。
5. Host ASan/UBSan及ARMv5/OABI + QEMU ARM926通过。
6. 实际编译/部分链接证明P1与彩色核心同名cpu_reset冲突；仅给彩色对象添加
   a918_color_cpu_reset等前缀后可共存链接。没有把它直接链接进交付P1程序。

产物在work/game-research/color-storage。ARM text=27504字节，data=0，
BSS=8553120字节；这个测试夹具无论测4MiB还是8MiB都分配最大8MiB工作数组。
ARM32 Emu大小仍为65160，GRAM32768，overlay65536字节；实际4MiB配置若按需分配，
这些合计约4.16MiB，8MiB约8.16MiB，另加前端画面、栈、代码、文件缓存等。
未测A918空闲RAM或CPU速度，不提供FPS或“必能流畅”结论。

GCC13在再次编译时出现CFG内部错误，O0也发生；没有改上游逻辑绕过。
Host检查已改用已安装的GCC12并通过；ARM工具链仍在Os下编译通过。
上游minsize属性被GCC忽略的警告保留。还修正了旧彩色smoke研究入口的svc0用法；
本轮主要执行的是新的color-storage测试，不把历史EABI smoke当设备兼容证据。

## 这还不是什么

RAM工作镜像回写测试不是Linux文件持久化、断电恢复或完整存档实现。
它只补齐核心flash读写的一个关键环节。CPU/定时器/内存等完整状态仍要序列化，
不能直接把含指针的Emu结构体写盘再原样读回。原ROM文件须保持只读。

用户最终指定彩色画面固定原生128×128，不做放大；水平居中，左右各56像素。A/B/C可以延续P1左/中/右映射，但颜色转换、LCD睡眠、
保存状态及逐帧节奏仍需实际ROM和前端适配。

MAME提供原版iD已知dump元数据：4MiB，CRC6272c24d，
SHA1 f4d67409fa15873961bda684c68f3febf501218f：
https://github.com/mamedev/mame/blob/master/src/mame/handheld/tama_id.cpp
该MAME驱动自身标记NOT_WORKING，因此只用于核验dump，不能当可运行证据。
本轮未获得Plus Color或iD L可用ROM，尚无彩色真实启动/孵化结果。

## 下一步

优先Plus Color；若原版iD先得到可核验的ROM，也可先用iD验证4MiB路线。
先在本地验证真实启动、设时、孵化、flash写入及退出再进，再测A918的忙场景速度
和峰值内存，最后决定是否接入Games、构建单独候选。保持已成功P1基线不变。
