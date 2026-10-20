# A910 原厂 FM 开关与菜单选择逻辑

本轮范围：只读分析索尼原版 1.03，不加入 Game，不重写 Home，不生成启用固件，
不连接/操作设备，不写 NVP，不加载收音驱动。模型开关实验只发生在 Unicorn
模拟器的私有内存中，原固件和播放器均未改动。

## 当前结论

确实存在原厂 FM 功能位、驱动和原厂 FM 页面；但没有找到“将 FM 开关设为 1，
A910 的 Home 就自动出现 FM”的路径。已分析的 A910 九宫格是机型专用、固定
九项的实现。就当前证据，不能把仅修改开关视作可用的原厂 FM 入口解锁方案。

这不证明硬件一定没有 FM，也不证明无法进一步找到其他原厂入口。尤其是通用
FM 页的全部页面工厂/可达性、原厂其他上下文入口及实机硬件仍有未核实部分。

## 1. 驱动层开关：来自 sysinfo，独立于主程序机型表

`etc/init.d/rc.sysinfo` 执行 `/usr/local/bin/sysinfo fm`，结果导出到 `ICX_FM_ON`。
`etc/init.d/icx1087driver` 仅在其等于 1 时执行
`insmod /lib/modules/icx1111_radio_tuner.ko`。1SEG 使用独立标志及 dtv.ko 路径。

sysinfo 工具包含只读设备 `/dev/icx1111_sysinfo`；`fm` 命令选择的查询 ioctl
为 0x80047305。工具另有 putsysinfo，写 `/dev/icx1087_nvp/24`，修改的是完整
系统配置，不能与无害的只读 fm 查询混为一谈。本轮没有执行它们。

mc.sh 的原厂默认配置：Daisy/A91x 的 sysinfo 为 00133241，IrisAAD/S61xF 为
00010901，IrisAADNC/S71xF 为 00012901。这是厂内表，不是本台设备 NVP 读取值。
不通过对比这几个数字就宣称已经确定每个硬件位的位置。

## 2. 主程序的功能位：硬编码机型资料

原版 `usr/local/bin/gemini-ICX1101` SHA256：
`a5c94ce492fb7ec9cd7a740d30cb79a1c2b43ee782e2da036a72f57dabdcd177`。

VA 0x5632d8 起是 20 条机型资料，每条 20 字节：ModelID、容量类别、名称指针、
功能字和另一配置字。原函数 0x44dd54 用 ModelID 查表并建立 80 字节配置对象。
FM 对应配置对象 +72 中的 0x20 位；这是程序层功能位，不是上面的 sysinfo
32 位字中可直接套用的同一个位置。

| 机型 | ModelID | 主程序功能字 | FM 位 0x20 |
| --- | --- | --- | --- |
| NW-A916 | 0x02000001 | 0x50052 | 0 |
| NW-A918 | 0x02000002 | 0x50052 | 0 |
| NW-A919 | 0x02000004 | 0x50052 | 0 |
| NW-S615F（对照） | 0x01000000 | 0x50032 | 1 |

以上不只是静态解析：分析脚本执行了原 ARM 查表函数，输出与表逐项一致。
ModelID 的正常来源可追到 0x44e690，读取 `/dev/icx1087_nvp/16`。
没有将当前播放器伪装成其他机型，也不建议为得到 FM 位而整体更改机型。

## 3. ICX_FM_ON 环境变量不是无条件覆盖

0x44df04 读取 ICX_FM_ON；严格的字符串 "1" 会置 0x20，"0" 会清除该位。
但初始化 0x44e04c 在 0x44e104 检查先前 GEMINI_MODEL_ID 覆盖校验结果，
只有该分支启用时才调用环境变量覆盖函数。

执行原初始化尾部的两个离线实验（环境变量均为 ICX_FM_ON=1）：

| 原分支状态 | 最终 A918 功能字 | FM |
| --- | --- | --- |
| 未启用 GEMINI_MODEL_ID 覆盖 | 0x50052 | 0 |
| 已进入有效 ModelID 覆盖分支 | 0x50072 | 1 |

实验控制的是原分支的输入，不是给设备写配置；并不表示主程序全部启动或
FM 初始化已经成功。此结果说明单独 export ICX_FM_ON=1 也未必改变应用功能位。

原始 rootfs 中 `/usr/local/bin/gemini` 链接到 `gemini-ICX1101`；init.d/gemini
通过 watchman 直接运行它。`gemini.sh` 是另一套带目标选择/伪装参数的启动脚本，
不在这个正常启动链中。因此只编辑 gemini.sh 的 FM_ON 对正常开机无效。

## 4. A910 的 Home：固定九项，而非按 FM 位追加

二进制保留的源码路径明确区分：

- Home：`app/ICX1101/ui/src/Page/DuiICX1101PageSID011_G_HOMEMENU.cpp`
- FM：`app/common/ui/src/Page/DuiPageSID104_S_PLAYFM_MANUAL.cpp`

Home 构造函数 VA 0x2835c：0x284ec 固定分配 36 字节的图标指针数组，
0x285d8 固定与 9 比较。每个图标 80x72，以三列排列，未发现按 FM 位追加槽位。

固定表 VA 0x4634f8 为九条 16 字节记录：

| 类型编号 | 原始目标页面值 | 原项目 |
| --- | --- | --- |
| 4 | 73 | 随机播放 |
| 12 | 477 | 电视 |
| 5 | 61 | 搜索 |
| 1 | 217 | 照片 |
| 0 | 25 | 音乐 |
| 2 | 166 | 视频 |
| 7 | 112 | 设置 |
| 11 | 401 | 录音 |
| 9 | 0 | 正在播放（动态处理） |

原 Home 分发函数 0x2a028 取该表提供的类型和目标页面；现有表没有 FM 条目。
另有 ICX1101 专用 OID001_L_HOMEMENU 列表类，已定位到固定五项列表资料
0x462814 及计数 getter 0x23f8c/0x23f94。其使用独立的选项编号体系，不能
只因表中未直接出现页面编号 104 就断定它完全没有间接 FM 路由；此处的全部
映射尚未核实。这也不是对全部原厂菜单调用路径已穷尽的证明。

对原 Home 构造函数进行 FM 位 0/1 两组离线回放，均得到：
36 字节指针数组 + 9 次 432 字节图标对象分配 + 9 次图标初始化。
外部 UI 构造/资源服务和内存分配由桩提供；这个实验验证固定创建数量，
不验证真实屏幕、不模拟射频，也不能独自排除其他未分析的页面选择路径。

## 5. 原厂 FM 页面存在，但入口和硬件是另外两件事

可确认原厂代码包含：

- DuiFmComponent
- DuiPageSID104_S_PLAYFM_MANUAL
- DuiPageSID115_L_SEL_SETTING_FM
- DuiPageSID143_LS_SET_FM_SENSE
- DuiPageSID144_LS_SET_FM_MONO_AUTO
- DuiPageOID009_L_FM_PLAY

这提供了进一步分析原厂页面工厂/路由的对象，不能据此宣称 FM 可收音。
在保留原九项、不另写菜单的约束下，当前没有获得能直接挂出 FM 的开关方案。
接下来若继续，应只追这些原厂页面的创建、进入条件及原有调用者；在确认入口
可达和硬件前，不制作“FM 已启用”固件。

## 复核材料

运行：`work/venv/Scripts/python.exe scripts/analyze-stock-fm-menu.py`

详细结果：`work/fm-native-analysis/report.json`。
输入为原版主程序，脚本明确拒绝 Game 改造 ELF；模拟执行遇到任何 Linux syscall
立即失败。输出仅是分析 JSON，未改固件、未构建升级包、未操作设备。
