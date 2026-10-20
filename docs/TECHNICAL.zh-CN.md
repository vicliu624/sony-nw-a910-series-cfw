# Sony NW 自制固件技术文档

[English](TECHNICAL.md) · [简体中文](TECHNICAL.zh-CN.md) · [日本語](TECHNICAL.ja.md)

## 1. 项目介绍与适用范围

本工程以日版 A910 系列的简体中文、英文本地化为主线。奇卡哇主题展示资源替换，自制程序入口和游戏展示运行自定义代码的可能性；它们是案例，不是项目固定主题或应用平台承诺。本工程旨在抛砖引玉，提供 Sony NW 自制固件的研究方法。

理论上可以用这套方法研究其他 Sony NW 型号，但必须重新确认固件封装、密钥、CPU／指令集、操作系统、资源、分区和升级流程。不能假定所有 NW 都使用 Linux 或相同 UPG，也不能直接移植 A910 的地址、补丁和驱动调用。

原版升级程序列出 A916／A918／A919；只有 A918 有实机测试记录。早期中英文本地化、主题及部分游戏版本获得过成功反馈；R12 安装失败。当前外部程序候选及新重建的中、英文包仍需分别实机验证，不能继承早期版本的成功结论。

开发前请先读本文，并核对当前源码。`docs/archive/` 是按时间保留的调查，可能含后来被推翻的假设；`archive/` 中旧工具和私有输入不进 Git，不能作为干净构建的依赖。下列命令在工程根目录的 Linux／WSL 中运行，只处理普通文件。禁止在电脑上执行从固件解出的升级脚本或设备挂载脚本。

## 2. 源码位置与构建方法

| 需要解决的问题 | 当前入口 |
| --- | --- |
| UPG 解密、校验、重封装 | `scripts/upg.py`：unpack、pack、replace_member |
| 导出二进制及文字证据 | `scripts/inspect_firmware.py` |
| DWR 解析、补译与 DPK 重建 | `scripts/wording.py`：parse_dwr、fill_dwr、complete_pack |
| 中英补译原文 | `translations/missing.tsv` |
| 五个原厂文件的已知差分 | `assets/patches/manifest.json` 与 .xor.zlib |
| 更新一个已审核的资源差分 | `scripts/update_resource_patch.py`，默认仅预览 |
| 完整构建与打包 | `scripts/build_release.py` |
| 安装后完整块分配检查 | `scripts/check-rootfs-capacity.py` |
| ARM 编译与底层适配 | `games/build-apps-linux.sh`、`games/src/boot_arm.c`、`sony_oabi.S` |
| 实验入口、扫描与监控 | `games/boot-dispatch-apps.sh`、`launcher_platform.inc`、`launcher_watchdog.inc` |
| 独立 UPG 解包校验 | `third_party/rockbox-upgtools/` |
| 依赖、版本、CI | `requirements.txt`、`VERSION`、`.github/workflows/ci.yml`、[BUILD](BUILD.md) |

按 BUILD 安装依赖、编译 Rockbox 工具后：

```sh
python scripts/check_project.py
python -m unittest discover -s tests -v
python scripts/build_release.py --language en
python scripts/build_release.py --language zh-CN
```

它们生成两份独立固件，不是机器内切换语言。当前构建仍包含示例主题与程序入口，没有纯本地化／原厂主题配置开关。本地两次构建要串行，因为共用 work；CI 用隔离的矩阵任务。不得启用 Python -O／PYTHONOPTIMIZE，继承的解析与测试含 assert。

## 3. 原版输入和 Rockbox 做了什么

索尼原始地址：`https://walkman.update.sony.net/fw/pc/A910/NW-A910_V1_03.exe`。

| 输入 | 字节数 | SHA-256 |
| --- | ---: | --- |
| 原版 EXE | 13,628,920 | `6f745a8ff3c90b7abcd5ea185e6f51e4598492f6de5bdb3256599c6f43e39701` |
| NW_A91X.UPG | 13,734,432 | `cfbcc372e2a18f11c12a080c9e5f039333e09175f5db03ac841a4227d3896b32` |

原 EXE 曾在 Windows 验证数字签名有效，签名者为 SONY Corporation；它不能替修改后的 UPG 背书。构建校验 EXE 哈希，用 7z 解开、不运行 EXE，并核对恰好 15 个文件：更新器、两个 DLL、INI、UPG 和十份许可。原版缓存位于 `firmware/original/`，解包在 `work/ci/updater/`。

Rockbox 已研究 Sony UPG 的格式，并实现解包、打包和密钥搜索。本工程复用该工具，在预期的十六进制字符密钥空间中找到能正确解密的候选，再验证头、四个组件范围和 MD5。不是 AI 突破非对称加密。已知旧机型密钥不能直接解本包。

这份固件的 DES 等效密钥是 **ASCII 八字节** `b8442c44`，不是按十六进制转成四字节。包内标识为 ASCII `5dfeff3c`；Rockbox 推导的 KAS 为 `ba49adfd57caba10b771b9bef805b2f9`。DES 校验位会造成文本不同但加解密等效的密钥。

原版四组件不改动重封装，SHA-256 与原包完全一致；独立 Rockbox 重打包也一致。当前构建再次用 Rockbox 解开最终包，逐组件比对。它证明封装处理正确，不证明设备安装安全。仓库保留的 Rockbox 工具历史副本没有记录上游提交号，不要编造提交溯源。

## 4. UPG 的确切布局

所有整数为小端，DES-ECB 块长 8 字节。

| 文件位置／解密头位置 | 含义 |
| --- | --- |
| 文件 0..15 | 文件 16..EOF 的原始 MD5 摘要 |
| 文件 16..63 | 加密后的 48 字节头 |
| 解密头 +0 | 八字节 `5dfeff3c` |
| +8、+12 | u32 组件数 4；u32 保留值 0 |
| +16+8*i | u32 组件绝对偏移、u32 未补齐长度 |
| 组件绝对偏移处 | DES 加密数据，密文长度向上补齐 8 字节 |

必须核对标识、MD5、偏移对齐、顺序、不重叠和范围，不能只看解出可读文字。这里的标识不是公钥签名，MD5 也不是来源认证。

| 组件 | 原版偏移 | 原版字节数 | 已确认用途 |
| ---: | ---: | ---: | --- |
| 0 | 64 | 1,578 | 设备端升级脚本 |
| 1 | 1,648 | 2,523,136 | Linux 内核，已用于系统调用解码重放 |
| 2 | 2,524,784 | 786,432 | ext2 initrd 镜像 |
| 3 | 3,311,216 | 10,423,215 | gzip 压缩的 tar 根文件系统 |

早期笔记曾将组件 1 标为未知，后来分析已确认用途。当前只修改组件 3，保留前三个加密组件，更新头中组件 3 的长度并重算 MD5。`upg.py` 专用于这份格式，不是通用索尼解包库。

构建准备好原版输入后，可导出证据：

```sh
python scripts/inspect_firmware.py --upg work/ci/updater/NW_A91X.UPG --output work/inspect-original
```

输出目录须为 work 下的新目录。工具导出四个组件、主程序、启动脚本、文字／纹理包、12 个 DWR 和 inspection.json，不把任意归档路径直接释放到主机，也不执行 ARM 程序。组件 0 只读，不运行。

## 5. 系统与语言选择指令

原始 rootfs 有 613 个归档条目。界面主程序 `/usr/local/bin/gemini-ICX1101` 是 ARM Linux ELF，5,691,632 字节。文字包为 `/usr/local/share/gemini/res/wording_res/ICX-COMMON_wording.dpk`。纹理位于同级 `dui_tex/`。字体包包括 `dui_font.dpk`、`ICX1101_dui_font.dpk`；菜单用英文不等于曲名不再需要中日文字形。

原读取流程是加载语言资源→读取配置语言编号→选对应 DWR→按 ID 取文字。此原版主程序的关键位置：

| 项目 | 值 |
| --- | --- |
| 程序虚拟地址 | `0x96294` |
| ELF 文件偏移 | `0x8e294` |
| 原指令与小端字节 | `ldr r3, [r0, #0x14]`；`14 30 90 e5` |
| 固定英文 | `mov r3, #0`；`00 30 a0 e3` |
| 固定简体中文 | `mov r3, #6`；`06 30 a0 e3` |

只替换一条四字节指令，后续取字逻辑不动。它不是增加语言设置页，也不写地区／NVP，不负责独立的开机与升级文字。其他 ELF 必须先按 PT_LOAD 定位：文件偏移=`p_offset+VA-p_vaddr`，且地址在文件支持的段内。不能把虚拟地址当文件偏移，也不能把这个补丁用于其他固件版本而不核对原字节。

## 6. 全部 11 个语言段

文字 DPK 有一个公共段和 11 个语言段。**DPK 段索引等于语言编号+1**，索引 0 是公共段。

| 语言编号 | DPK 段 | 语言 |
| ---: | ---: | --- |
| 0 | 1 | 英语 |
| 1 | 2 | 日语 |
| 2 | 3 | 法语 |
| 3 | 4 | 德语 |
| 4 | 5 | 意大利语 |
| 5 | 6 | 西班牙语 |
| 6 | 7 | 简体中文 |
| 7 | 8 | 繁体中文 |
| 8 | 9 | 俄语 |
| 9 | 10 | 韩语 |
| 10 | 11 | 葡萄牙语 |

每个语言段有 826 个编号；公共段有 34 项，观察到的程序对共享 ID 826..859 不经过语言选择。日文有字 648 项、空／保留 178 项；原英文有字 360 项，较日文缺 288 项；原简中有字 359 项，缺 289 项。其他语言必须单独统计，不套用英文数量。

这为其他语言开发者提供了直接入口：选择目标段，按日文同 ID 比较，保留已有翻译，仅补缺失，核对字体与布局，再修改选择编号。**有资源不等于翻译完整、字体完备、页面可达或已经实测。** 不能把 11 个内置段称为 11 个已完成语言包，也不能从中推断索尼当年的分支开发历史。

当前 CLI 只接受 en 和 zh-CN。增加其他语言时，需要新增 TSV 列、语言／段映射、适合该语言的换行规则、实测缺项数量、选择指令编号和最终包检查。wrap_text 的非英文分支按 CJK 宽度切行，不能原样用于法语／德语的单词换行；已有中英文测试应继续保留。

## 7. DPK／DWR 与补译重建

文字 DPK 头 `<4I`：负载字节数、段数、两个保留零；索引 `<QQ`：key、段偏移。每段到下一段起点为止。修改长度后必须重建后续段偏移和负载长度，保留各 key 及非目标段。

DWR 头 `<4I`：语言编号、条目数、行长度池字节偏移、UTF-16 文字池字节偏移。每条记录 `<bbHHH` 共八字节：有符号行数、secondary、u16 长度池索引、u16 文字池索引、u16 元数据。池索引单位是 **16 位字**，不是字节。每行长度包含 UTF-16 NUL；解析必须检查范围与终止符。secondary 与动态内容插入有关，不能只复制可见字符串。

`parse_dwr()` 解码；`fill_dwr()` 只填日文有字而目标为空的记录，保留目标元数据，为缺项沿用日文 secondary，把新行长度和文字追加到池尾。重算 text_pool 偏移，并按 16 字节对齐池／段；索引必须小于等于 65535。保留动态字段位置和必要空行。不要用长译文覆盖原字符串而破坏后续内容。

`translations/missing.tsv` 列为 id、english、simplified_chinese；字面 \n 表示换行。新增译文约限每行 20 个显示宽度单位，ASCII 算 1，宽汉字算 2，行数不超过原日文。它是长度启发式，不是像素排版证明。

```sh
python scripts/wording.py --upg work/ci/updater/NW_A91X.UPG --language en --output work/localization/en.dpk
python scripts/wording.py --upg work/ci/updater/NW_A91X.UPG --language zh-CN --output work/localization/zh-CN.dpk
```

当前 CI 根据 TSV 为选中语言生成完整资源，检查补齐 288／289 项、较日文无空缺和原记录保持不变，再将选择指令设为 0／6。两份固件分别选择语言；实验启动器自己的文字目前仍为英文。

## 8. 图标、动画、背景的格式与布局

纹理 DPK 也有 16 字节头，索引可按 `<4I` 读取：key、category、offset、reserved。存在嵌套包，需用头、计数、负载大小及偏移范围识别，不能只看扩展名。已观察到的文件名哈希：初值 0，对每个字节执行 `h=(37*h+byte)&0xffffffff`，最后 `h|0x80000000`。保留 category、reserved 和父子层级。

本例修改的 DTX 为格式 5：头部 `<HHI` 是宽、高、格式，完整头长 16 字节，其后为小端 RGB565；长度应为 `16+2*宽*高`。编码 `((R>>3)<<11)|((G>>2)<<5)|(B>>3)`。这只适用于已观察的格式 5 叶节点，不代表全部索尼纹理格式。

九项名称词干：i_shuf／oneseg／search／photo／music／video／setup／rec／now。包括 `home_<stem>.dtx`、`_hot.dtx` 和编号动画帧。不是简单的两套通用图标，要检查静态、选中和过渡各帧及引用。静态预览不能证明实机状态机；更换角色只是此案例的美术选择。

已确认 Home 布局：屏幕 240×320；顶栏 y=0..27；九格从 y=28 开始，每格 80×72，到 y=243；标题底图 240×44 位于 y=244..287；状态底图 240×32 位于 y=288..319。有些叶节点高 70，必须按节点本身尺寸编码。

Home 标题底图在 ICX1101_home_anim.dpk，叶 key `0xe60ccd8f`、父 key `0xaa48c2d0`，程序加载到纹理槽 21；状态栏背景还供其他页面共用。第一次背景底部露出原色，是只替换了九格，没有替换两个独立底图。应从同一画布按 y=28／244／288／320 切片，并检查白色前景文字对比度及其他页面。包中其他 240×320 图可能是关机／开机图，不可认作 Home 壁纸。

优先保持 DTX 头、尺寸、长度和父 DPK 偏移不变；长度变动时应完整重建所有受影响索引。

## 9. 修改资源并更新补丁

当前五个差分对应 Gemini 主程序、文字包、两个纹理包、`/etc/init.d/gemini`。manifest 记录原文件／目标文件／差分的 SHA-256 和结果长度。差分解压后与补零的原文件逐字节 XOR，再截为目标长度。CI 消费它们，不会从美术源文件重绘主题。

修改已审核的纹理包：

```sh
python scripts/update_resource_patch.py --original-upg work/ci/updater/NW_A91X.UPG --path ./usr/local/share/gemini/res/dui_tex/dui_tex.dpk --replacement work/my-theme/dui_tex.dpk
# 检查预览结果后，再加 --apply 写入这个差分和 manifest。
```

它只更新已有白名单中的一个差分，不生成／刷写固件，也不证明新图包语义正确。先检查变化节点、头和尺寸，再重建固件、独立解包、检查文件差异和容量。增加系统文件或新型号应显式扩展构建器及验证，不能为了通过而删掉哈希检查。

注意：每个语言的主程序指令和文字包在基线差分之后重新生成。只改这两项基线差分不能覆盖 `complete_pack()` 与选择指令；翻译应改 TSV／文字工具，选语言应改明确的构建逻辑。纯本地化或原厂主题配置是可做的后续工作，不是已经存在的命令行开关。

## 10. 自制程序如何启动

rootfs 启动脚本在索尼程序之前调用 `/usr/local/bin/a918-boot-dispatch`，用 A918_BOOT_BYPASS 防止递归。调度脚本对应 `games/boot-dispatch-apps.sh`，小型界面为 a918-boot，保留原紧急启动与索尼路径。不改九宫格／OPTION；早期 R6／R7 直接挂主菜单的实验曾卡死，已放弃，不要重用。

仅 `nvpflag -x fup` 明确返回 `0x00000000` 才允许入口；更新标志未知／待更新、USB 接入／状态不可读时转索尼。读标志不等于可以写 NVP。默认 Walkman 超时路径不挂载用户盘。显式选择 APPS 后，确认没有已有数据盘挂载、/tmp 确为 ramfs/tmpfs，再把 `/dev/mtdblock/2` 以 rw,nosuid,nodev,noexec 私有挂载到 `/tmp/a918-game-disk`，并重复检查 USB。

应用清单位于 APPS/<id>/app.ini，检查长度／CRC 和基础 ARM ELF 后复制到 `/tmp/a918-app-runtime/program`，以自身目录为 cwd。心跳进程组监控处理普通退出和卡住，返回索尼前卸载。卸载三次失败则 exit 91，不启动索尼，避免双重访问；用户态超时救不了不可中断内核 I/O。程序没有安全沙箱。完整接口见 [APPS](APPS.md)。

App、ROM、存档放在用户盘，避免系统包增长和反复刷写。iD 例子把 4MiB ROM 读入 RAM，校验 CRC，模拟闪存修改存入 state 而不覆盖 rom.bin。保存用临时文件、fsync、rename、目录 fsync；最后同步失败时正式文件可能已替换。并非每次保存都生成备份，FAT 断电耐久性未实测，不应保证存档不丢或应用永不拖死系统。

## 11. ARM 指令、旧内核 ABI、显示和按键

编译参数为 `-march=armv5te -marm -mfloat-abi=soft`，静态／freestanding，以 `_start` 进入，不依赖现代 glibc。索尼内核为旧 ARM Linux 2.6.10；ARM 模式下现代 `svc 0`＋r7 传调用号的方式不能正确调度，应使用立即数 `SWI 0x900000+nr`。

`sony_oabi.S` 保持 C 层 AAPCS 参数／寄存器保存规则，通过 288 项 SWI 桩发起调用。它是系统调用适配，不是把所有 C 结构都变成旧 OABI。信号返回也需要正确立即数。QEMU user-mode 跑通并不证明旧内核兼容。

`check-apps-kernel-abi.py` 用 Unicorn 重放原内核解码：vector_swi=`0xc0022ae0`，sys_call_table=`0xc0022ba4`，arm_syscall=`0xc0028388`。跳过无关 CP15 设置，停在选出的处理入口，并不执行真实硬件调用；检查 28 个调用号。

`boot_arm.c` 打开 `/dev/fb/0`、`/dev/input/event0`、`/dev/usbconn`。QFB 设置 ioctl `0x80044e04` 参数 `{0,33}` 参考原程序，会改显示时序，不应因 ioctl 方向位把它叫只读查询；映射 `0x80000` 字节，用 `0x40044e01`、`{1,0x800,255,page,0,0}` 呈现 RGB565。USB 状态调用 `0x80046701`，失败转回索尼。按键独占 EVIOCGRAB=`0x40044590`；观察到原始键码 0=中、1=右、2=左、3=上、4=下。必须处理按下与松开、交接时释放独占／映射／描述符。这些都不能直接套用其他型号。

ARM 同族也不等于指令兼容。本目标使用 iD 解释器，不能直接运行上游针对 Cortex-M3 的 Thumb-2 动态生成代码；换设备不能只比较 MHz。

## 12. R12 失败与容量硬门槛

原升级脚本先 `mkfs.ext2 -b 4096 /dev/mtdblock/1`，挂载后用 fwpup 取组件 3 并输送给 tar。**先格式化，后解压，没有 A/B 回滚。** 当前保持该脚本、内核和 initrd 原样；改电脑端升级器界面不能消除这条风险。

原 NAND 驱动默认 root 为 `0x03000000`＝48MiB，尚未读回实机几何。原 BusyBox mke2fs 1.37 创建 12288 块，开始分配文件前空闲 11895 块。完整分配 R11 需 11704 块，R12 需 11921 块，至少超 26 块＝106496 字节。已复现确定的容量错误且与设备现象一致；没有设备日志，不能声称唯一原因已经排除。R12 禁止安装。

当前门槛只在 work 下普通镜像中执行：用原 mke2fs＋QEMU 建 ext2，debugfs 为全部文件实际分配数据块和间接块，包含零数据，计算目录／链接等占用，要求至少 4MiB 余量；不挂载真实盘。压缩包大小、文件长度求和、稀疏写入成功、现代 mkfs 默认值都不能替代它。

同模型原版剩 5,312,512 字节，R14 英文剩 5,267,456 字节，安装占用较原版增加 45,056 字节＝44KiB。新中文／英文构建要看各自 capacity.json；这些不是实机 df 或坏块测量。外置存储解决系统包增长，不等于解决供电、写入失败或救援问题。

## 13. 构建检查与发布

顺序：核对原 EXE→准确解包→编译→ARM 测试→模拟调度→原内核调用解码→五差分→生成选定语言→校对 tar 内容与元数据→容量完整分配→封装→Rockbox 独立解包→ZIP 与 SHA-256。新增两个 rootfs 文件后共 615 条。检查原条目顺序、所有者、权限、时间、链接等元数据；检查通过后才发布产物。

公开测试包含 3 个容器回归、5 个文字保留／边界回归、17 个不带 ROM 的 ARM 目录夹具、14 个模拟调度、28 个内核解码，另有编译脚本自检。真实 ROM 的开机／保存／新游戏测试需要私有输入，公开 CI 明确跳过，不算通过。它们不能证明实机 USB 交接、掉电、休眠、NAND／FAT 和音质不变；保留音频驱动减少改动范围，但不是音质测量。

[BUILD](BUILD.md) 包含环境、输出与发布命令。VERSION 是工程 SemVer，设备仍为索尼 1.03。CI 矩阵分别生成 en、zh-CN，全部通过后，匹配标签才创建实验性预发布。没有自动刷设备或写 NVP。托管构建记录见 [GitHub Actions](https://github.com/vicliu624/sony-nw-a910-series-cfw/actions)，本地与托管结果分别记录。

## 14. 已知故障与恢复限制

故障设备反复尝试 R12，用户确认没有 USB 窗口。目前没有已验证的强制 USB 按键或同故障救砖路径。正常 rootfs 的紧急 USB 入口，不能保证在 rootfs 不可用时生效。原厂注释中的播放键救援分支未启用；主界面的工厂按键解析不能证明升级页也能调用。没有确认的 UART 焊点、电平或 shell，不提供猜测短接、他机按键套用、盲目 RESET 或无通信／备份的启动标志修改。

保留 R11、原版及失败证据。详见[容量／故障调查](archive/r12-recovery-investigation.md)、[公开救援证据](archive/recovery-public-evidence-2026-10-19.md)、[原版占用比较](archive/rootfs-usage-original-vs-r14.md)。早期文档状态可能过时，应以时间和后续证据判断。

## 15. 新型号／新语言的开发流程

1. 固定型号、板号、固件版本与哈希，分别记录事实、推断与未知。
2. 先完成离线解包及不修改的完全往返校验；不盲用 A910 密钥和标识。
3. 先读设备端安装器，明确分区、写入顺序、容量、更新标志与恢复条件。
4. 查明目标 CPU／OS／ABI、资源、字库和语言映射，不转用本机地址和 ioctl。
5. 从一个受控的资源／语言选择改动开始，列出允许变化的文件，保留原翻译和元数据。
6. 最终包独立解码、完整安装占用检查、保留足够余量与经过核对的回退固件，记录跳过的测试。
7. 实机验证需可工作的设备及明确的恢复条件，不能用离线通过宣称全部型号安全。

建议按 README、本文、BUILD、相关源码的顺序阅读。已有 UPG 解密、DWR 结构、OABI 结论不必重新推导；其他语言须自行统计空缺和字形。每个新结论记录输入哈希、命令、输出、型号适用范围和限制，不能把“编译通过”写成“功能已完成”。
