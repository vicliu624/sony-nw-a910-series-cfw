# A918 R9 P1 / 双状态图标 ALPHA2：修正存档挂载

本版替代ALPHA1。ALPHA1的P1存档假定/var已经挂载，但代码和脚本证据显示
Sony本身有准备该分区的初始化流程，不能把这个假定用于Sony启动前的游戏。
ALPHA1不应安装。它仍保留作历史记录，未覆盖旧ZIP。

ALPHA2仍是实机未验证的本地候选，没有自动安装，也不能保证不会卡死。

## 修正内容

游戏启动脚本读取/proc/mounts，在自己的/tmp/a918-p1-state临时挂载点，以ext2
挂载现有/dev/mtdblock/3，绝不运行格式化、修复或修改分区命令，也不提前挂到/var。
只有挂载成功才允许P1运行。失败时Tetris和Walkman仍可用，P1显示STORAGE UNAVAILABLE，
按下键回列表，不会在空的RAM目录下假装存档成功。

存档文件在该分区的a918-games/p1.sav，游戏中的访问路径为
/tmp/a918-p1-state/a918-games/p1.sav；Sony随后将同一分区挂到/var时，对应
/var/a918-games/p1.sav。文件位置不会因为这个临时路径而改变。
写入过程为临时文件、fsync、原子rename、目录fsync。游戏及监护进程结束后，
外层启动脚本卸载自己的挂载点，成功后才进入原Sony启动脚本。
卸载失败最多重试3次；仍失败或私有挂载点本来被占用时，不擅自卸载他人挂载，
也不继续Sony的分区检查，退出并保留错误日志。该异常路径可能需要人工复位，
不能把它描述成绝对可自动恢复。

原始证据：主程序0x1cc6a8先umount，运行分区检查命令，再mount；
0x1cc808后的调用参数关联/var、ext2和/dev/mtdblock/3。
原/etc/init.d/mtp也明确挂载该设备到/var。

## 操作

开机WALKMAN / GAMES；无输入8秒继续播放器。Games有TETRIS、TAMAGOTCHI P1、
RETURN TO WALKMAN。进入Sony后再次进游戏仍需要重新执行系统启动。
Tetris：左右移动、上旋转、下缓降、中间结束本局，无暂停/存档。
P1：左=A，中间=B，右=C，下=保存并返回。已内置校验的初代P1 ROM。
首次B进入设时，A调小时、B调分钟、C确认、B回宠物画面；等待孵化。
P1为无声版本，离开后时间冻结，无周期自动保存。强制终止可能丢失未保存进度。
退出或USB接入会尝试保存；保存失败会提示重试，坏档会拒绝载入。
Sony九项、原OPTION行为及九个角色双状态图标延续ALPHA1；FM不启用。

## 离线验证

- Host ASan/UBSan及ARM926真实ROM测试：设时、孵化、喂食、快照恢复的一致性、
  错档拒绝、存档失败重试、新增“持久存储不可用时拒绝启动P1”。
- 18个实际ARM worker模拟硬件用例，包含存储禁用后不读ROM/存档、不进行文件写入。
- 9个私有挂载脚本隔离用例：正常、mount失败、proc缺失、mkdir失败、worker错误/
  被杀、卸载重试、卸载持续失败、挂载点已占用；mount/umount全部由假的服务代替。
- 6个打包后的完整启动脚本隔离用例；没有在电脑上实际mount、umount或访问播放器。
- Rockbox独立解包最终UPG；原主程序、内核、驱动、音频库与R5一致，未增加菜单钩子。

必须区分：已验证的是脚本顺序、程序逻辑及模拟系统调用，不是A918真实挂载成功、
磁盘状态、驱动行为或断电安全。仍需设备验证，用户当前要求只保留本地。
源码和TamaLIB GPL许可证随包提供；ROM来源记录单列，不属于TamaLIB开源许可。

候选ZIP位于releases/R9-P1-Icons-ALPHA2；原R8及R9 ALPHA1保留。
回退包仍为releases/RECOVERY-1.03/ENGLISH-R2-NO-GAME。
