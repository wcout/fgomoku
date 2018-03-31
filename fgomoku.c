#include <graphapp.h>
#include <string.h>

#define BLACKCOLOR   Black
#define WHITECOLOR   White

int GRIDSIZE = 20;
int BORDERSIZE;

int OX;
int OY;

int BOARDSIZE;
int RX;
int UY;
int UC;

int PIECESIZE;
int PBOXSIZE;
 

typedef struct
   {
   int x;
   int y;
   int color;
   } move;

typedef struct
   {
   char is_anz;
   char poss_anz;
   char freedom1;
   char freedom2;
   } pos;

typedef struct
   {
   pos   dir[4];
   } dirpos;

dirpos stat[19][19];


window MainWindow=0;
char board[19][19];   
int colors[2] = {0, 1};
int anz_pieces[2];
move LastMove;

int  anz_moves=0;
move movelist[19*19];
int ComputerMoves;
int FirstMove;
label InfoText, InfoTextBack;
int gameover;
menuitem test_eval_flag;



void redim(gridsize)
   {
   GRIDSIZE = gridsize;
   BORDERSIZE = GRIDSIZE;

   OX = (2*BORDERSIZE);
   OY = (2*BORDERSIZE);

   BOARDSIZE = (18*GRIDSIZE);
   RX = (OX + BOARDSIZE + BORDERSIZE);
   UY = (OY + BOARDSIZE + BORDERSIZE);
   UC = (BOARDSIZE/2 + OY);

   PIECESIZE = (GRIDSIZE-GRIDSIZE/6);

   PBOXSIZE = (PIECESIZE*8);
   if ( MainWindow )
      {
      resize(MainWindow, rect(0, 0, RX + 2*PBOXSIZE, UY + OY));
      resize(InfoTextBack, rect(RX+PBOXSIZE/2-2, UC-100/2-2, PBOXSIZE+4, 104));
      resize(InfoText, rect(RX+PBOXSIZE/2, UC-100/2, PBOXSIZE, 100));
      redraw(MainWindow);
      }
   }


void fcircle(x, y, r)
   {
   /* draws a filled circle */ 
   rect rc;
   rc.x = x - r;
   rc.y = y - r;
   rc.width = 2 * r;
   rc.height = 2 * r;
   fillarc(rc, 0, 360);
   }

void circle(x, y, r)
   {
   /* draws a filled circle */ 
   rect rc;
   rc.x = x - r;
   rc.y = y - r;
   rc.width = 2 * r;
   rc.height = 2 * r;
   drawarc(rc, 0, 360);
   }


void drawdot(int x, int y, int color)
   {
   color = colors[color];
   if ( x < 19 )
      x = OX+x*GRIDSIZE;
   if ( y < 19 )
      y = OY+y*GRIDSIZE;
   
   setcolor(DarkGrey);
   fcircle(x, y, 2);
   }


void drawpiece(int x, int y, int color)
   {
   color = colors[color];
   if ( x < 19 )
      x = OX+x*GRIDSIZE;
   if ( y < 19 )
      y = OY+y*GRIDSIZE;
   
   setcolor(DarkGrey);
   fcircle(x+1,y+1,PIECESIZE/2);

   if (color)
      setcolor(BLACKCOLOR);
   else
      setcolor(WHITECOLOR);
   fcircle(x,y,PIECESIZE/2);

   if (color)
      setcolor(WHITECOLOR);
   else
      setcolor(BLACKCOLOR);
   circle(x,y,PIECESIZE/2 + 1);

   }


void drawtable(int c)
   {
   /* 0=player, 1=computer */
   int i, o, ox, oy;
   if ( c )
      o = -1;
   else
      o = 1;
   setcolor(Black);
   fcircle(RX+PBOXSIZE+5,UC+PBOXSIZE*o+5,PBOXSIZE/2);
   setcolor(DarkRed);
   fcircle(RX+PBOXSIZE,UC+PBOXSIZE*o,PBOXSIZE/2);
   setcolor(Black);
   fcircle(RX+PBOXSIZE,UC+PBOXSIZE*o,PBOXSIZE/2-PBOXSIZE/7);
   for (i=0; i<anz_pieces[c]; i++)
      {
      ox=rand()%((PBOXSIZE-PIECESIZE)/4);
      oy=rand()%((PBOXSIZE-PIECESIZE)/4);
      if ( rand()%2 ) ox=-ox;
      if ( rand()%2 ) oy=-oy;
      drawpiece(RX+PBOXSIZE+ox,UC+PBOXSIZE*o+oy, c);
      }
   }


void drawboard(ox, oy)
   {
   int i;
   setcolor(Black);
   fillrect(rect(ox-BORDERSIZE+5, oy-BORDERSIZE+5, 18*GRIDSIZE+2*BORDERSIZE, 18*GRIDSIZE+2*BORDERSIZE));
   setcolor(0x00AA8D6EL);
   fillrect(rect(ox-BORDERSIZE, oy-BORDERSIZE, 18*GRIDSIZE+2*BORDERSIZE, 18*GRIDSIZE+2*BORDERSIZE));
   setcolor(Black);
   for (i=ox; i<=ox+18*GRIDSIZE; i+=GRIDSIZE)
      drawline(pt(i,oy), pt(i, oy+18*GRIDSIZE+1));
   for (i=oy; i<=oy+18*GRIDSIZE; i+=GRIDSIZE)
      drawline(pt(ox,i), pt(ox+18*GRIDSIZE+1,i));
   }


void make_move(void *data)
   {
   move best_move;
   settimer(0);
   settext(InfoText, "I'm thinking...");
   setcursor(WatchCursor);
   find_best_move(&best_move, 2);
#if 0
   getstatall();
   while ( 1 )
      {
      int x, y;

      x = rand()%19;
      y = rand()%19;
      if ( free_pos(x,y) )
         {
         delay(rand()%3000 + 1000);
         move_to(x, y, 1);
         break;
         }
      }
#endif
   move_to(best_move.x, best_move.y, 1);      
   setcursor(ArrowCursor);
   if ( checkfive(2) )
      {
      askok("I win !");
      settext(InfoText, "Game over - I win");
      gameover = 1;
      return;
      }
      
   ComputerMoves = 0;
   prompt_move();
   }


void prompt_move()
   {
   settext(InfoText, "Your move please!");
   }


int free_pos(int x, int y)
   {
   return ( board[x][y]==0 );
   }


#define HORI         0
#define VERTI        1
#define LO_RU        2   
#define LU_RO        3

void getstatpos(int x0, int y0, int dir)
   {
   int c;
   int x1, x2;
   int y1, y2;
   int f1, f2;
   int x, y;
   int dx, dy;
   int anz;
   
   c = board[x0][y0];  /* 0=empty, 1=player, 2=computer */
   switch ( dir )
      {
      case HORI:  dx = 1; dy = 0; break;
      case VERTI: dx = 0; dy = 1; break;
      case LU_RO: dx = 1; dy = 1; break;
      case LO_RU: dx = 1; dy = -1; break;
      }
        
   anz = 1;
   x = x0 + dx;
   y = y0 + dy;
   while ( x >= 0 && x < 19 && y >= 0 && y < 19 && board[x][y] == c && anz < 6 )
      { x += dx; y += dy; anz++; }
   x1 = x; y1 = y;
   dx = -dx;
   dy = -dy;
   x = x0 + dx;
   y = y0 + dy;
   while ( x >= 0 && x < 19 && y >= 0 && y < 19 && board[x][y] == c && anz < 6 )
      { x += dx; y += dy; anz++; }
   x2 = x; y2 = y;
   stat[x0][y0].dir[dir].is_anz = anz;

   x = x1; y = y1;
   dx = -dx;
   dy = -dy;
   f1 = 0;
   while ( x >= 0 && x < 19 && y >= 0 && y < 19 && board[x][y] == 0 && anz < 6 )
      { x += dx; y += dy; if ( c ) anz++; f1++; }
   x = x2; y = y2;
   f2 = 0;
   dx = -dx;
   dy = -dy;
   while ( x >= 0 && x < 19 && y >= 0 && y < 19 && board[x][y] == 0 && anz < 6 )
      { x += dx; y += dy; if ( c ) anz++; f2++; }

   stat[x0][y0].dir[dir].poss_anz = anz;
   stat[x0][y0].dir[dir].freedom1 = f1;
   stat[x0][y0].dir[dir].freedom2 = f2;
   
   }


int checkfive(int c)
   {
   int x, y, d;
   getstatall();
   for (x=0; x<19; x++)
      {
      for (y=0; y<19; y++)
         {
         if ( board[x][y] == c )
            {
            for (d=0; d<4; d++)
               if  ( stat[x][y].dir[d].is_anz == 5 ) return 1;  
            }
         }
      }
   return 0;
   }


void getstatall(void)
   {
   int x, y;

   for (x=0; x<19; x++)
      {
      for (y=0; y<19; y++)
         {
         getstatpos(x, y, HORI);
         getstatpos(x, y, VERTI);
         getstatpos(x, y, LU_RO);
         getstatpos(x, y, LO_RU);
         }
      }
   }   



int eval(int c)
   {
   int x, y;
   int e=0, d;
   for (x=0; x<19; x++)
      {
      for (y=0; y<19; y++)
         {
         if ( board[x][y] == c )
            {
            for (d=0; d<4; d++)
               {
               e += 50 * ( stat[x][y].dir[d].is_anz == 2 && stat[x][y].dir[d].poss_anz >= 5 );
               e += 500 * ( stat[x][y].dir[d].is_anz == 3 && stat[x][y].dir[d].poss_anz >= 5 && (stat[x][y].dir[d].freedom1 || stat[x][y].dir[d].freedom1));
               e += 700 * ( stat[x][y].dir[d].is_anz == 4  && (stat[x][y].dir[d].freedom1 || stat[x][y].dir[d].freedom2));
               e += 5000 * ( stat[x][y].dir[d].is_anz == 5 );
               e += 20 * ( stat[x][y].dir[d].poss_anz >= 5 );
               e += 5 * ( (stat[x][y].dir[d].freedom1 != 0) + (stat[x][y].dir[d].freedom2 != 0) );
               e += 3 * (stat[x][y].dir[d].freedom1 >= 4 && stat[x][y].dir[d].freedom2 >= 4);
               e += 70 * (x>6 && x<12 && y>6 && y<12);
               }
            }
         }
      }
   return e;   
   }


int evalmove(int x, int y, int c)
   {
   int my_strength, opp_strength;
   board[x][y] = c;
   getstatall();
   
   my_strength = eval(c);
   opp_strength= eval((c & 1) + 1);
   board[x][y] = 0;

   if ( ischecked(test_eval_flag) )
      {
      char str[100]; 
      sprintf(str, "%d/%d:\n %d : %d -> %d\n", x,y,my_strength, opp_strength, my_strength - opp_strength);
      settext(InfoText, str);
      }

   return (my_strength - 2 * opp_strength);
   }


void find_best_move(move *best_move, int c)
   {
   int x,y;
   int max = -1000000;
   int e;

   for (x=0; x<19; x++)
      {
      for (y=0; y<19; y++)
         {
         if ( board[x][y] == 0 )
            {
            e = evalmove(x, y, c);
            if ( e > max )
               {
               max = e;
               best_move->x = x;
               best_move->y = y;
               }

            else if ( e == max && rand()%2 )
               {
               max = e;
               best_move->x = x;
               best_move->y = y;
               }
            }
         }
      }
   }
   
   
void move_to(int x, int y, int color)
   {
   if ( LastMove.x > 0 )
      drawpiece(LastMove.x, LastMove.y, LastMove.color);
   drawpiece(x, y, color);
   drawdot(x, y, color);
   LastMove.x = x;
   LastMove.y = y;
   LastMove.color = color;
   movelist[anz_moves].x = x;
   movelist[anz_moves].y = y;
   movelist[anz_moves].color = color;
   anz_moves++;
   anz_pieces[color]--;
   drawtable(color);
   board[x][y] = color + 1;   /* 0=empty, 1=player, 2=computer */
   }


void mousebutton(control c, int buttons, point xy)
   {
   int x, y;

   if ( gameover ) return;
   
   if ( xy.x>=OX-PIECESIZE/2 && xy.y>=OY-PIECESIZE/2 )
      {
      x = (xy.x - OX + PIECESIZE/2) / GRIDSIZE;
      y = (xy.y - OY + PIECESIZE/2) / GRIDSIZE;
      if ( x < 19 && y < 19 )
         {
         if ( !ComputerMoves )
            {
            if ( !free_pos(x, y) )
               {
               beep();
               return;
               }
            move_to(x, y, 0);
            if ( checkfive(1) )
                {
                askok("You win !");
                settext(InfoText, "Game over - you win");
                gameover = 1;
                return;
                }
                
            ComputerMoves = 1;
            if ( !ischecked(test_eval_flag) )
               settimer(100);
            }
         else if ( ischecked(test_eval_flag) )
            {
            if ( board[x][y] == 0 )
               {
               evalmove(x,y,2);
               }
            }
         }
      }
   }


void drawpiecelist(void)
   {
   int i;
   for (i=0; i<anz_moves; i++)
      drawpiece(movelist[i].x, movelist[i].y, movelist[i].color);
   }


void redraw_window(window w, rect r)
   {
   drawboard(OX, OY);
   drawpiecelist();
   drawtable(0);
   drawtable(1);
   }


void info(char *title, char *text)
   {
   window about;
      
   about = newwindow(title, rect(10, 10, 300, 200), Centered + Modal + Titlebar + Closebox);
   setbackground(about, LightGrey);
   newlabel(text, rect(10,10, 280, 180), Center + VCenter);
   show(about);

// askok(text};   
   }

   
void about(menuitem m)
   {
   info("About Gomoku", "***** G O M O K U *****\nFive in a row\n\n(c) 2000\nChristian Grabner\n");
   }


void quit(menuitem m)
   {
   exitapp();
   }


void newgame(void)
   {
   memset(board, 0, sizeof(board));
   anz_moves = 0;
   gameover = 0;
   anz_pieces[0] = 100;
   anz_pieces[1] = 100;
   redraw(MainWindow);

   memset(stat, 0, sizeof(stat));
   ComputerMoves = FirstMove;
   FirstMove = !FirstMove;
   LastMove.x = -1;

   if ( ComputerMoves )
      settimer(100); /* make_move() is called */
   else
      prompt_move();
   }

void new_game(menuitem m)
   {
   srand(currenttime());

   newgame();
   }

void change_colors(menuitem m)
   {
   int temp;
   if ( gameover ) return;
      
   temp = colors[0];
   colors[0] = colors[1];
   colors[1] = temp;
   redraw(MainWindow);
   }


void take_back(menuitem m)
   {
   if ( !gameover && anz_moves > 2 )
      {
      LastMove.x = -1;
      anz_moves--;
      board[movelist[anz_moves].x][movelist[anz_moves].y] = 0;
      anz_moves--;
      board[movelist[anz_moves].x][movelist[anz_moves].y] = 0;
      redraw(MainWindow);
      }
   }


void hint(menuitem m)
   {
   move best_move;

   if ( gameover ) return;
   
   find_best_move(&best_move, 1);
   move_to(best_move.x, best_move.y, 0);
   redraw(MainWindow);
   ComputerMoves = 1;
   if ( !ischecked(test_eval_flag) )
      settimer(100);
   } 


void test_eval(menuitem m)
   {
   if ( ischecked(m) )
      {
      if ( ComputerMoves )
         settimer(100);
      uncheck(m);
      }
   else
      check(m);
   test_eval_flag = m;
   }

void smaller(menuitem m)
   {
   if ( GRIDSIZE > 12 )
      redim(GRIDSIZE -1);
   }
   
void bigger(menuitem m)
   {
   if ( GRIDSIZE < 30 )
      redim(GRIDSIZE + 1);
   }
    

void main(void)
   {
   redim(GRIDSIZE);
   
   MainWindow = newwindow("GOMOKU", rect(30, 30, RX + 2*PBOXSIZE, UY + OY), StandardWindow + UsePalette);
   setbackground(MainWindow, 0x00A0CACAL);

   setredraw(MainWindow, redraw_window);

   newmenubar(NULL);
   newmenu("File");
   newmenuitem("About...", 'A', about);
   newmenuitem("Quit", 'Q', quit);
   newmenu("Display");
   newmenuitem("Bigger", 'L', bigger);
   newmenuitem("Smaller", 'S', smaller);
   newmenu("Game");
   newmenuitem("New game", 'N', new_game);
   newmenuitem("Change colors", 'C', change_colors);
   newmenu("Move");
   newmenuitem("Take back", 'B', take_back);
   newmenuitem("Hint...", 'H', hint);
   newmenu("Test");
   test_eval_flag = newmenuitem("Eval", 'E', test_eval);

   setbackground((InfoTextBack=newlabel("", rect(RX+PBOXSIZE/2-2, UC-100/2-2, PBOXSIZE+4, 104), Center + VCenter)), Black);
   InfoText = newlabel("", rect(RX+PBOXSIZE/2, UC-100/2, PBOXSIZE, 100), Center + VCenter);
   setbackground(InfoText, White);

   show(MainWindow);

   srand(currenttime());

   FirstMove = rand()%2; 
   settimerfn(make_move, NULL);

   newgame();

   setmousedown(MainWindow, mousebutton);
   
   mainloop();
   }
