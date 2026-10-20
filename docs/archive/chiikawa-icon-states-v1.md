# 奇卡哇双状态图标 v1（2026-10-18）

用户指出选中图标应有独立动作/表情。核对原资源替换脚本可见，原厂有无后缀、
hot及编号帧资源。旧R5 draw_tile对所有这些状态复用同一个characters[key]，
选中变化主要为大小；不符合本次要求。

本轮使用内置image_gen，分别以原A1–A9作为编辑目标生成9张新的透明选中姿势，
普通态保留原图，合计18张素材。没有重绘背景、替换角色身份或写入设备。
提示集：`assets/chiikawa/icon-states-v1/PROMPTS.md`。

九组分别是：乌萨奇举手跳起、栗子馒头双手贴脸、古本屋睁眼招手、飞鼠眨眼展开双手、
吉伊举手闭眼笑、挎包铠甲人抬腿舞动、海獭抬手眨眼、狮萨抬手大笑、小八歪身眨眼。
铠甲人的面孔被头盔遮住，保留头盔，通过身体动作表达状态，不杜撰可见面孔。

## 文件

- 普通态：`assets/chiikawa/icon-states-v1/normal/`
- 选中态：`assets/chiikawa/icon-states-v1/selected/`
- 每个角色的源图SHA256及对应：`assets/chiikawa/icon-states-v1/manifest.json`
- 评审页及9个状态对比GIF：`assets/chiikawa/icon-states-v1/review/index.html`
- 对比总图：`assets/chiikawa/icon-states-v1/review/normal-selected-contact-sheet.png`
- DTX、DPK和菜单设计预览：`work/chiikawa-theme/pose-states-v1/`

GIF只演示两态，不是实机录屏，不代表原厂动画时长。

## 资源转换

`make-chiikawa-theme.py --footer --pose-states`：保持普通图的角色，hot使用新姿势；
编号帧早期使用原姿势，后期切入新姿势，仍保留尺寸变化，没有用双图交叉淡化制造
小尺寸重影。生成81个编号帧和14个静态纹理资源，并非81幅独立绘制的动作。
保持80×72/80×70尺寸和RGB565编码。转换脚本的输出目录已修正到工程work内。

`verify-chiikawa-icon-states.py`检查通过：40个DPK头和表、473个DTX叶节点；
相对R5，61个图标叶节点发生变化，378个非图标叶节点完全一致；所有原有
文件尺寸、表、偏移、纹理头保持不变。两份资源包位于上面的resource-packs目录。

这些是资源层验证。索尼真实普通/选中/动画状态的使用时序未在本轮运行或实机验证，
不能根据设计预览宣称所有设备状态已经正确切换。

## 固件状态

没有生成或刷写新UPG。R8候选仍保留原文件，SHA256为
`01cd1835b751f6c930ede95205adf010deeca49988ca8d53fd83dfd5565650e6`。
P1模拟器集成与本次图标资源准备是两项尚未合并到新固件的工作。
