# P1 ROM 获取与启动检查（2026-10-18）

用户要求寻找缺失ROM后，已下载并校验初代P1 ROM。本轮没有制作固件或操作设备。

## 文件

- `work/game-research/p1-rom/p1.bin`：从下载包内 `tama.b` 提取的原始程序。
- `work/game-research/p1-rom/tama.zip`：原下载包，另含 `test.b`，未把test.b作为游戏。
- `work/game-research/p1-rom/provenance.json`：来源与校验值。

来源页面：https://wowroms.com/en/roms/mame/tamagotchi-usa/108517.html
下载使用该页面公开的正常下载流程，未执行下载站提供的软件。

## 核对依据

MAME原始驱动：
https://github.com/mamedev/mame/blob/master/src/mame/handheld/hh_e0c6200.cpp

其中`tama`游戏程序为0x3000字节，当前称为`tama.bin`，记录CRC32及SHA1。
TamaTool规定ROM每条12位指令用大端16位存储：
https://github.com/jcrona/tamatool

| 项目 | 实测 |
| --- | --- |
| 文件长度 | 12288字节 |
| 指令数 | 6144个16位存储单元 |
| 每条指令高4位 | 均为0 |
| CRC32 | 5c864cb1 |
| MD5 | 3fce172403c27274e59723bbb9f6d8e9 |
| SHA1 | 4b4979cf92dc9d2fb6d7295a38f209f3da144f72 |
| SHA256 | 67b6388f26e2e3f15674932baf2fc2fb1c6f388cc0f16ea1aa0f441db1a4f43c |

MAME数据提供CRC32和SHA1依据；MD5另与tamagotchi-p1-web的说明核对一致：
https://github.com/tonypan2/tamagotchi-p1-web

## 本地启动验证

`games/research/p1-rom-smoke.c`将原始ROM解析成TamaLIB指令数组，使用虚拟时钟和
内存LCD回调运行。依赖源码从固定版本TamaLIB复制至work/game-research/p1-runtime/core，
没有改动这些上游文件；HAL类型来自上游模板。电脑编译启用了ASan/UBSan。

输出：

```text
virtual_us=5000071 steps=29168 lcd_updates=8704 lit_pixels=72 PC=1a6 errors=0
```

得到蛋的32×16画面，截图保存为`work/game-research/p1-runtime/p1-boot.png`，
是模拟LCD输出的7倍显示，不是绘制的示意图。

这证明实际ROM可读并完成基本启动；尚未验证设时/孵化、喂食、完整按键流程、存档、
A918上的运行速度与设备接口。P1尚未接入Games菜单，也未进入新固件。
后续任务仍是完成TamaLIB宿主、持久状态及Games集成；不能将本次ROM检查当作已完成。
