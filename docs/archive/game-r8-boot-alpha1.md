# R8 Boot ALPHA1：启动选择 Walkman / Games

用户已改为启动选择方案，取消把 Game 嵌入索尼九宫格。本版从 R5 英文奇卡哇
主题固件重新构建，未沿用发生过卡死的 R6/R7 主程序补丁。

## 启动与交接

正常启动分支在 `/etc/init.d/gemini` 异步启动 dispatch，允许后续 rc2 服务继续
启动，包括 S90icx1087helper 启动的原厂 watchdog_test。工厂测试分支不改变。
dispatch 运行独立静态 ARMv5TE 程序 a918-boot；其父进程只监护自己 fork 的子进程。
子进程退出（包括异常）后，父进程回收它并退出，dispatch 设置绕过变量，exec
回原启动脚本。此时原来的 run_command、watchman 参数及 emergency USB 路径照常执行。
不存在游戏与 Gemini 同时绘图，也不存在 SIGSTOP/SIGCONT 索尼进程或注入索尼 ELF。

界面：WALKMAN 默认选中，8 秒无五向键输入自动继续；GAMES 内有 TETRIS 和
RETURN TO WALKMAN。TETRIS 仅五向键：左右移动、上旋转、下缓降、中间结束本局。
每次新开局，运行路径不读写存档、不暂停。BACK 和 OPTION 不用作游戏控制。
进入索尼后只有原九项，OPTION 原行为。再次选择游戏需重新执行系统启动，
不是从索尼菜单返回；普通待机唤醒是否重走该入口尚未实机确认。

## 显示、按键与回退

- 显示使用原厂 `/dev/fb/0` QFB 调用序列：0x80044E04 查询，mmap 0x80000 字节，
  两个缓冲间隔 0x40000，0x40044E02 配置、0x40044E01 呈现。源证据为原 Gemini
  0x1BC6E0 和 0x1BCA08，布局 240×320 RGB565。这还不是设备验证。
- 主程序表 0x488390 给出方向键和 CENTER 原始编号：左2、右1、上3、下4、中0。
  原 icx1087_key.ko 的 input_event 调用证实新按下值0、松开值1；HOLD 时编号可
  叠加0x80，当前代码不接受这些编号。无需七键校准，也不写 keys.cfg。
- 退出前尽量排空按键松开事件，解除 EVIOCGRAB、关闭 fd、munmap，才交还启动流程。
- 原 USB 只读状态请求 0x80046701 非零或失败时直接继续 Walkman。游戏中每100ms
  查询，检测到连接则退出，交给 Sony 处理 USB。没有写入 USB 模式 ioctl。
- 监护5秒无心跳会结束自己的子进程；测试覆盖正常结束、SIGKILL异常和停滞。
  内核不可中断睡眠、驱动死锁不能靠这个机制保证恢复；没有声称彻底防卡死。
- 初始选择页无输入8秒退出；其他列表无输入120秒回播放器；游戏局内不设该超时。

## 已执行验证

1. Host ASan/UBSan：方块规则、200次新局和中间键返回、WALKMAN选择。
2. QEMU ARM926：同一逻辑测试，监护正常/异常/停滞，缺少真实 framebuffer 回退。
3. Unicorn 实际 ARM worker：12种模拟硬件系统调用用例，包括分段事件读取、
   游戏进出、USB接入、设备/显示调用失败、双缓冲和全部资源释放。
4. 打包后的 init/dispatch：6种隔离的 shell 测试，覆盖正常、选择程序报错或缺失、
   dispatch缺失、watchman错误后的原USB应急路径、工厂模式绕过。
5. Rockbox 独立解包最终 UPG；逐个核对所有原有 rootfs 成员内容和元数据。
   原有文件只有 init.d/gemini 改动，新增两个可执行文件。原 Gemini 主程序与
   R5 英文版完全一致，前三个UPG组件、内核、驱动、音频库、翻译和主题保持一致。

本地检查不验证真实 LCD、输入设备节点、QFB 驱动行为、USB 时序或音质。
当前没有刷写设备，尚不能确认启动选择、游戏及返回在 A918 上稳定。

## 构建与材料

```text
wsl -d Ubuntu-24.04 -- sh games/build-boot-linux.sh
work/venv/Scripts/python.exe games/test-boot-arm.py
work/venv/Scripts/python.exe scripts/build-boot-game-firmware.py
wsl -d Ubuntu-24.04 -- python3 games/test-boot-startup.py
```

包：`releases/R8-Boot-ALPHA1/`。中间代码、测试及预览：`work/game-boot/`。
回退包：`releases/RECOVERY-1.03/ENGLISH-R2-NO-GAME/`。
实机仍须确认：选择页显示和默认进入、手动进入Sony及USB识别、TETRIS控制、
重复新局、回播放器后的按键/播放及USB连接。目标的实机稳定性验证尚未完成。
