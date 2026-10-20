# R7 Directional ALPHA2 — five-key Tetris and stock OPTION

User requires that OPTION keep its original behavior, and Tetris accept only
the four directions plus center play/pause.

Found gap in R7 ALPHA1: Home passed OPTION through, but the active overlay still
consumed it, and Tetris assigned hard drop to it. A binary replay regression
first failed at page2, state4 with "OPTION swallowed".

Fix: before custom input processing, every pending virtual OPTION7 or BACK6
event exits an active overlay, pauses/saves the game if needed, restores both
Sony buffers and the original grid selection, then follows the displaced stock
Home instruction. It never clears/mutates the event bytes. On stock Home,
OPTION passes through immediately. Other Sony pages do not use the Home hook.
Only the previous underlying Sony Home context is available when leaving the
custom overlay; this does not invent a Sony options menu for the game.

Tetris only responds to left/right movement, up rotate, down soft drop, center
pause menu. In that menu, directions select Resume or Save and Exit, center
confirms. OPTION/BACK do not move/drop/exit within the game engine; those are
Sony-owned keys at the outer integration boundary. No hard-drop button is mapped.
Game-list Back to Home is selected with directions and confirmed with center,
then a direction at the page-two edge restores original Home. The whole game
entry/play/pause/save/return lifecycle works without OPTION or BACK.

Regression replay covers original Home, second page, game list, running game
and FM, each across OPTION phases 4/6/7/8: exact event bytes retained, stock
continuation reached and both buffers restored. Physical Sony short/long-press
behavior and the prior reboot fix still require A918 confirmation.
