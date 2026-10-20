  text(12,9,"TETRIS",0xffff,1);rectangle(10,42,102,202,0xffff);rectangle(11,43,100,200,0x0000);
  for(int y=0;y<BH;y++)for(int x=0;x<BW;x++)if(game.board[y][x])block(11+x*10,43+y*10,game.board[y][x],10);
  for(int y=0;y<4;y++)for(int x=0;x<4;x++)if(blocks_cell(game.piece,game.rotation,x,y)&&game.y+y>=0)block(11+(game.x+x)*10,43+(game.y+y)*10,game.piece+1,10);
  text(128,44,"NEXT",0xbdf7,1);for(int y=0;y<4;y++)for(int x=0;x<4;x++)if(blocks_cell(game.next,0,x,y))block(130+x*12,65+y*12,game.next+1,12);
  text(128,125,"SCORE",0xbdf7,1);number(128,140,game.score);text(128,165,"LINES",0xbdf7,1);number(128,180,game.lines);text(128,205,"LEVEL",0xbdf7,1);number(128,220,game.lines/10+1);
  if(game.over){rectangle(16,125,208,40,0x19a8);text(25,138,"GAME OVER",0xffff,2);}
  text(12,263,"LEFT RIGHT MOVE  UP ROTATE",0xbdf7,1);text(12,280,"DOWN - SOFT DROP",0xbdf7,1);text(12,300,"CENTER - END ROUND",0x07ff,1);
