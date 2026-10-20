# A918 English / Chiikawa R10 Color iD ALPHA1

本地构建的实验候选包，尚未在 A918 上验证本版本的运行速度与稳定性。
保留 R9 ALPHA3（用户已报告 P1 能运行）作为原有基线。本轮未连接或刷写设备。

## 内容和操作

开机选择 Walkman / Games，默认 8 秒无按键进入 Walkman；USB 已连接时直接进入
索尼系统。请拔掉 USB 后完整重启以进入启动选择，待机唤醒不一定重跑启动脚本。

Games 列表：TETRIS、TAMAGOTCHI P1、TAMAGOTCHI ID、RETURN TO WALKMAN。

- Tetris：左右移动，上旋转，下软降，中间键结束本局返回。没有保存或暂停。
- P1：左 A、中间 B、右 C；下键保存返回。沿用原来的黑白画面尺寸。
- 彩色 iD：左 A、中间 B、右 C；下键保存返回。128×128 原尺寸，画面左上角
  位于 (56,80)，没有缩放。静音，无红外/NFC或联机功能。
- 返回列表后选 RETURN TO WALKMAN 启动索尼播放器。

保留英语完整翻译、暗色奇卡哇背景和正常/选中两套表情动作。索尼原九项菜单、
OPTION、音频程序、内核和驱动沿用 R9 基线。没有新加 Sony 菜单 hook 或开启 FM。

## 内置 ROM

P1 延续已验证的 12288 字节 ROM。彩色内置的是日文 Tamagotchi iD，
不是 Plus Color，也不是把游戏内的日文翻译成了英文。Walkman 和 Games 界面为英文。

iD 来源：Internet Archive 的公开 iD/iDL 固件存档。
https://archive.org/details/fw_tg14_jp_v54_2011_09_05_iDL-15

原文件：fw_tg13_jp_v43_2009_10_03_iD.bin，4194304 字节。
SHA-1：d1d04d910914b1dc2c90fc7a6340437bb408ce8c
SHA-256：1f8202c77f952524fec841e978b142bcb6c627225b1e761f00058c2be6901d48

SHA-1 与来源存档元数据一致；这不是与 MAME 另一版 iD dump 的匹配证明。
文件名标注 2009-10-03。原始内容完整内置，没有擅自擦除其中的游戏数据。
本地启动显示设时提示和角色，可能包含公开 dump 自带的游戏状态。

## 存储与退出

沿用私有 ext2 临时挂载，退出后释放再启动 Sony。P1 存档路径不变，彩色另存为
/var/a918-games/color-id.sav（游戏运行时路径为私有 /tmp/a918-p1-state）。
彩色存档包含约 90 KiB 的 CPU/RAM/LCD/定时器状态，以及每个 8 KiB 的已改动闪存
区块；不会每次都写入完整 4 MiB ROM。大小取决于游戏写过多少区块。采用显式
小端格式和完整性校验；绑定原 ROM 的指纹，不直接保存含指针的 Emu 结构体。
按下键、USB 插入或正常退出时保存；没有后台养成或离开期间时间补算。

先写临时文件、fsync、rename、fsync 目录。旧存档损坏时拒绝加载/覆盖，提示后
可用下键返回。普通保存失败留在当前游戏并允许下键重试。核心错误不覆盖旧存档。
每次进入时重新加载内置只读 ROM，存档内的客体闪存不会写回系统 ROM 文件。

模拟器每 4096 客体周期检查一次耗时，约 8ms 后让出前端；计算不及时时客体变慢，
不积累追赶队列。不保证 A918 满速。进入彩色游戏按需申请约 4.3 MiB 内存，退出释放。
独立进程超时监护仍保留，保存文件期间允许较长的有限等待。

## 已做的验证及边界

- Host ASan/UBSan：P1 真实 ROM、Tetris 200 局、彩色加载/保存/校验失败路径。
- 真实 iD ROM：本地显示、按键响应、128×128 逐像素前端验证、保存退出再进 LCD 一致。
- ARMv5/OABI + QEMU ARM926：真实 P1 和 iD ROM；彩色实际文件写入/重载及画面一致。
- ARM 工作进程的 20 个模拟系统调用场景，包括 USB、释放资源、P1、彩色无内存/
  无存储后返回。它们模拟设备，不能代替驱动实测。
- 原始 Sony 内核 SWI 分派指令重放：22 个系统调用的 OABI 路径和入口检查。
- 使用独立 Rockbox 工具重新解包最终 UPG，比较内核等组件和所有系统文件。

没有完成 A918 彩色版实测、长期养成/所有小游戏兼容性、真实闪存断电恢复、全部
USB/退出时序和性能测量。不要将本候选称为已经在 A918 实机稳定的成品。

使用本目录 updater/FWUpdater.exe 配套的 NW_A91X.UPG；不要混用历史候选里的 UPG。
完整升级包和 SHA256SUMS 在同一个 release 目录。恢复包保留在 releases/RECOVERY-1.03。

## 来源和源码

彩色 C 解释器：Paramix Studio / tamaemu，具体提交和许可证见 source ZIP 中
games/vendor/color-core/PROVENANCE.md 和 LICENSE。未编译 Thumb-2 JIT。
P1 TamaLIB 使用 GPL-2.0-or-later；与 GPL-3.0 彩色核心组合的自制程序源码一并提供。
Sony 系统文件、游戏 ROM 和素材是独立内容，不声明为上述开源许可证覆盖。
