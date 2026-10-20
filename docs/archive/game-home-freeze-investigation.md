# Home OPTION freeze — device report

Observed by user: short OPTION on original Home freezes and reboots. User wants
directional edge paging, not an OPTION shortcut. ALPHA1/2 must not be described
as usable/stable on-device releases. Do not trigger their Home OPTION launcher.

The original input table at VA 0x488390 independently confirms virtual key 7
maps to raw OPTION code 19. Raw direction mapping is UP3->1, DOWN4->2,
LEFT2->3, RIGHT1->4, CENTER0->5, BACK18->6. The old tests injected already
decoded events and did not reproduce complete device scheduling or graphics.

Ranked hypotheses before changing implementation:
1. Blocking fork/wait/process-stop handoff prevents Sony tasks from progressing.
   Prediction: a cooperative, nonblocking in-process overlay needs no such
   operations and avoids that class of hang. Hardware confirmation still needed.
2. Hook/launch ABI or libc-fork interaction faults. Prediction: retaining the
   Home instruction trampoline but eliminating libc fork and child launch
   separates the trampoline from launch failures.
3. Framebuffer/USB preflight rejects the helper. This can explain no second
   page, but cannot alone explain the reported reboot. The stock UI maps two
   buffers and uses Sony custom QFB ioctls, unlike our strict generic FB probe.

Next implementation: preserve the nine stock functions, use direction keys at
grid edges for page two, update falling blocks cooperatively in Sony's existing
UI ticks, and composite into its existing current draw buffer before its own
present operation. No fork, wait, SIGSTOP/SIGCONT, EVIOCGRAB or new framebuffer
mode. The actual cause is not yet proven; this change removes the suspect
handoff rather than claiming an unmeasured timing/driver cause is established.

Sony graphics evidence: init 0x1bc6e0 maps 0x80000 bytes, buffer pointers are at
0x5d9a38, spaced 0x40000. The existing graphics context at 0x5d151c holds current
draw pointer +24, width +28, height +30 and byte stride +32. Stock init explicitly
passes 240x320, 16bpp. Present 0x1bca08 issues the stock QFB update and flips the
current page. These addresses must be asserted and exercised in a binary replay
test, not simply presumed correct because a browser preview works.
