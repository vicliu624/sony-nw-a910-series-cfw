# R12 安装失败与 A918 恢复调查

当前任务仅为恢复设备；不开发新功能、不生成新功能升级包。

## 设备现象与边界

用户报告 R12 更新失败，RESET 后重新进入更新进度，最终再次失败。
照片证实重新更新界面而不是普通开机卡住。用户确认重启期间没有 USB 窗口。
Windows 的当前 PnP/磁盘枚举未找到 Sony VID 054C 或 WALKMAN。
120秒、每200ms的 SetupAPI USB枚举记录只有初始清单，无Sony或新设备。
记录在 work/recovery-r12/usb-watch.jsonl；它不是USB电气抓包，不能排除所有
亚200ms电气变化，但不应再要求用户反复RESET寻找瞬时窗口。

尚未恢复设备。没有读取实际设备 NVP、没有清标志、没有实际刷写恢复包。
R12 ALPHA1 release 已添加 DO-NOT-INSTALL.txt。R11原包哈希保持不变。

## 已复现的系统分区容量问题

原版initrd中的 icx1087-nand-p.ko，icx1087_nand_partitions 默认第一分区
大小为0x03000000（48MiB），名称为 ICX1087 NAND RootFS。实际设备几何信息
尚未读回，因此这里明确是原厂驱动默认配置，不冒充实机测量。

原厂升级脚本：mkfs.ext2 -b 4096 /dev/mtdblock/1，随后解压rootfs。
使用原固件BusyBox内的mke2fs 1.37，经QEMU ARM926在普通文件中创建48MiB
ext2镜像（没有格式化任何真实磁盘或挂载任何设备）：12288块，128字节inode，
12288个inode；新建后空闲11895个4096字节块。

按真实tar条目创建目录和inode，并用debugfs为普通文件分配全部数据块和间接块，
避免稀疏零块优化掩盖容量需求：

| 固件 | 文件数据块 | 间接块 | 新目录 | 所需块 | 结果 |
|---|---:|---:|---:|---:|---|
| R11 |11560|84|60|11704|成功，剩191块|
| R12 |11776|85|60|11921|耗尽，在最后初始存档文件分配失败|

R12至少超出26块，即106496字节；还未把坏块等实际设备差异作为原因。
完整可重复脚本与日志：work/recovery-r12/check-allocation.py、capacity-r11.log、
capacity-r12.log、capacity-allocation-test.json。初始存档877403字节的系统分区
占用是本次新增的主要容量压力。这个离线结果足以证明该48MiB配置装不下R12，
与设备症状高度一致；实际错误日志仍未获得。

## 原厂恢复路径

1. /etc/init.d/gemini 的 run_emergency 存在。主程序不存在或异常返回时，
   调用 apl_conn -a -s 1。AAD/MSC路径还调用mgscsi_test_ioctl。
   MTP分支会设置ICX_RESCUE并重新加载MSC驱动。
2. load_usb_epson 支持救援PID 0x0324；正常ICX1101 PID为0x032A。
   不能宣称A918的每种紧急路径一定使用0x0324，AAD分支不同。
3. nvpflag工具将 fup 映射到 /dev/icx1087_nvp/9；原厂APCSpecificService
   的固件更新请求在VA 0x1a4ffc向它写入fWUp。清除/替换该状态可能与恢复有关，
   但没有直接执行通道，未对真实NVP进行写入，也不在未备份时盲目改写。
4. /etc/init.d/consstart 标明115200串口、gty开关0x67747479。
   普通终端登录需相应NVP开关；MPT会占用UART。因此不能保证接UART就得到shell。
5. mptapp实际存在（2798392字节），不是仅有占位脚本。

## A918按键入口的证据与限制

使用官方Ghidra 12.1.4仅分析本地原始gemini，不执行设备代码。
项目/反编译输出在work/recovery-r12/ghidra-project和decompiled*。

原ARM按键识别函数VA0x1ae290，逻辑键映射表VA0x488390。
三个特殊序列分别指向事件10、11、12。两条12键序列的数据表在0x488408与
0x4883d8；与A800系列服务手册中常见序列不同。
scripts/check-a918-factory-key-decoder.py用原ARM指令重放，stub日志和事件分配，
验证三种事件及其他机型序列不匹配。没有注入设备按键或写NVP。

关键限制：识别器仅在special-key读取开启时生效；上层键处理还有当前页面/运行
状态条件。MPT请求VA0x748e4还检查对象offset0xC为3、offset0x2C为0。
不能把这些条件擅自等同于错误页的实际状态，更不能说更新过程中一定能触发。
暂没有验证可在当前自动更新阶段接管的强制USB组合，不让用户套用其他机型组合。

## 维修接口下一步

用户有万用表和CH340模块。尚不知芯片后缀、模块电路和设备信号电平。
先不连接CH340 TX/VCC到播放器，不套用A800/A820板上的焊点编号。
需要实际A918主板两面、板号和模块照片，先识别候选GND/TX/RX，再决定只接收日志。
115200来自本固件串口启动脚本，不是已经实测成功的连接。

同系列实际拆解资料可用于结构核对，但不提供UART点位：
https://www.youtube.com/watch?v=dUfMS91fFgU
https://postrepair.net/repair-results/nw-a910_chargeconnecter/

未来ROM/游戏数据将放到可见存储区，固件仅保留程序和加载器。当前不实施迁移。
