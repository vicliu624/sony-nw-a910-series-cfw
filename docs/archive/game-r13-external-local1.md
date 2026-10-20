# R13 External iD LOCAL1 — 仅本地候选

当前设备尚在 R12 更新失败状态，本包不是无USB状态的救砖工具，尚未刷入或实机验证。
R11是最后由用户确认能运行的基线；原包保留。R12已标记禁止再次安装。

## 本版内容

Games仅保留 TETRIS、TAMAGOTCHI ID、RETURN TO WALKMAN。
P1初代程序、TamaLIB核心和P1 ROM不再编入本版。
彩色iD ROM和新游戏初始状态从UPG系统分区移除，改由可见存储加载。
Sony原九宫格、奇卡哇正常/选中资源、英语文字、主程序、驱动和音频保持原样。
保留R12的彩色下键菜单、确认页、短按队列和128×128显示；不承诺提速。

## 文件位置

将本包 WALKMAN-files 内的 GAMES 文件夹复制到播放器可见磁盘根目录：

```
WALKMAN:/GAMES/TAMAGOTCHI/ID/rom.bin     彩色iD ROM，4194304字节
WALKMAN:/GAMES/TAMAGOTCHI/ID/new.sav     已执行原游戏RESET后的初始状态
```

程序会在相同目录建立 state.sav（普通存档）、state.bak（上次重新开始前的备份）
和 state.tmp（保存临时文件）。ROM与new.sav始终只读打开，不写回原ROM。
当前仅接受这一份已核验的日文iD dump（CRC32 0c2e4116），不是通用彩色机型加载器。
ROM SHA256：1f8202c77f952524fec841e978b142bcb6c627225b1e761f00058c2be6901d48。
new.sav SHA256：d742d6938b95ab27ab0c62e2d2f1c6539bdfb423e8c4e9e47dcf23b54449c7ea。
两者在交付ZIP内作为独立磁盘文件，均不在firmware/NW_A91X.UPG中。

首次没有state.sav时按原ROM启动，公开dump可能自带旧宠物状态。要全新开始，
按下键打开菜单，选择NEW GAME，再选择YES - CONFIRM；new.sav只在确认后加载，
不自动覆盖已有进度。

旧的内部/var存档不自动迁移。恢复通信后若能备份旧color-id.sav，可另行核验并
复制为外部state.sav；本轮没有读取或改动设备存档。

## 操作

开机选择Walkman或Games；USB已连接/状态读不出时直接进入Sony。
Tetris仅方向键和中间键，中间键结束本局，没有暂停/存档。
彩色iD左=A、中间=B、右=C；下键打开Continue、Save and Exit、New Game、
Restore Backup菜单。危险操作默认Cancel。菜单期间游戏暂停，清空待处理按键。
缺少new.sav时，New Game拒绝操作，不先挪走旧存档。
当前模拟速度慢仍会导致响应延迟；短按队列修复不是速度或满帧保证。

## 减少分区风险的约束

1. 安装包必须先通过真实ext2分配检查，且48MiB系统分区至少留4MiB空闲。
   使用原厂BusyBox mke2fs 1.37创建普通镜像文件，再分配所有tar文件的数据块、
   间接块及目录。没有将压缩包大小当作系统占用，也没有只用宿主机大目录测试。
2. 原R12作为负对照，必须被该检查器拒绝。检查失败时不会生成新的UPG或release。
3. 独立Rockbox解包，逐项比较元数据和文件，确认只替换Games程序/启动包装脚本，
   并删除两份内置ROM。原厂升级脚本、内核、initrd保持不变。
4. 只有明确选择Games后才挂载数据区；默认或选择Walkman时不进行这次挂载。
   游戏包装脚本只挂载/dev/mtdblock/2（可见FAT数据区）到私有临时目录。
   不挂载系统分区或旧/var，不执行格式化、fsck修复或原始分区写入。
5. 原厂更新请求fWUp存在或更新标志读取失败时直接交回Sony，标志只读不修改。
   数据区已被别的组件挂载、USB连接、USB探测失败、/proc/mounts不可读时直接
   交回Sony，不抢占现有挂载。卸载失败时不让Sony同时使用同一分区。
6. 保存先检查空间并预留64KiB，临时文件写入、fsync、rename、目录fsync。
   无空间/只读/写入/同步/重命名失败时处理失败并保留可用的旧存档路径。
   使用ARM正确的O_DIRECTORY/O_NOFOLLOW常量，防止跟随最终文件符号链接。

这能拦截已复现的R12超容量问题，不能保证突然断电、FAT自身损坏、硬件坏块或
未测的USB/内核时序绝不造成数据损坏。保存仍然是向可见FAT分区的普通文件写入。
刷机仍由Sony升级程序按原厂脚本改写系统分区；不是完全不写分区。
实际设备几何、真实FAT驱动和USB交接还没有在本版实测。

## 本地验证

Host ASan/UBSan与实际共用的磁盘平台代码：缺ROM、坏ROM、无存储/内存、只读、
满盘、写入/fsync/rename失败、缺初始状态、菜单取消、重开备份、恢复、保存退出再进、
坏存档拒绝。ARM926/QEMU：俄罗斯方块200局、磁盘ROM/存档、新游戏与备份恢复、
输入锁存及进程监护。模拟系统调用工作进程23场景、原Sony内核调用分派24项、
假挂载服务17场景，以及打包启动脚本6场景。它们不能替代A918实机验证。

## 源码与历史

game-source.zip不包含P1/TamaLIB；彩色核心GPL-3.0，来源见vendor/color-core。
历史P1源码和旧release为回退/研究保留，不属于本固件。
打包入口 scripts/build-external-game-firmware.py，容量检查在UPG写入前强制执行。
此包只交付UPG和外部磁盘文件，没有自动刷写脚本或自动启动的Windows更新程序。
