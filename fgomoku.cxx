/*
    A quick conversion of a gomoku program written in the year 2000
    with GraphApp2 to FLTK in the year 2018.

    I didn't want to touch the code more than necessary, so
    I built a lot of wrapper functions that translate GraphApp
    to FLKT. Only some too complicted things (like menus) have
    been adapted.
*/
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <string.h>
#include <time.h>
#include <cassert>

struct rect
{
	rect() : x(0),y(0),width(0),height(0) {}
	rect(int x_, int y_, int w_, int h_) : x(x_), y(y_), width(w_), height(h_)	{}
	int x;
	int y;
	int width;
	int height;
};

struct pt
{
	pt() : x(0),y(0) {}
	pt(int x_, int y_) : x(x_), y(y_) {}
	int x;
	int y;
};

class FWindow : public Fl_Double_Window
{
	typedef Fl_Double_Window Inherited;
public:
	FWindow(int x_, int y_, int w_, int h_, const char *l_ = 0) :
		Inherited( x_, y_, w_, h_, l_ ),
		_drawFunc( 0 ),
		_mouseFunc( 0 )
		{}
	void draw()
	{
		Inherited::draw();
		if ( _drawFunc )
			(*_drawFunc)();
	}
	int handle( int e_ )
	{
		int ret = Inherited::handle( e_ );
		if ( e_ == FL_PUSH )
		{
			pt p(Fl::event_x(), Fl::event_y());
			if ( _mouseFunc )
				(*_mouseFunc)(this, Fl::event_button(), p);
		}
		return ret;
	}
	void setDraw( void (*d_)(void) )
	{
		_drawFunc = d_;
	}
	void setMouse( void (*d_)(Fl_Widget *, int, const pt& ) )
	{
		_mouseFunc = d_;
	}
private:
	void (*_drawFunc)(void);
	void (*_mouseFunc)(Fl_Widget *, int, const pt&);
};

typedef Fl_Widget *control;
typedef FWindow * window;
typedef Fl_Box * label;
typedef Fl_Menu_Item * menuitem;
#define point pt
#define beep fl_beep


void (*TimerFunc)(void *);

#define BLACKCOLOR   FL_BLACK
#define WHITECOLOR   FL_WHITE
#define DarkGrey		fl_darker(FL_GRAY)
#define LightGrey		fl_lighter(FL_GRAY)
#define Black			FL_BLACK
#define DarkRed		fl_darker(FL_RED)
#define WatchCursor	FL_CURSOR_WAIT
#define ArrowCursor	FL_CURSOR_ARROW
//#define ArrowCursor	FL_CURSOR_HAND
#define White FL_WHITE


#define Centered 0x1
#define Modal 0x2
#define Titlebar 0x4
#define Closebox 0x8

#define StandardWindow 0x0
#define UsePalette 0x0

#define Center FL_ALIGN_TOP
#define VCenter FL_ALIGN_CENTER


int GRIDSIZE = 40;
int MENU_HEIGHT = 30;
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
	pos dir[4];
} dirpos;

dirpos gstat[19][19];

window MainWindow = 0;
char board[19][19];
int colors[2] = { 0, 1 };
int anz_pieces[2];
move LastMove;

int anz_moves = 0;
move movelist[19 * 19];
int ComputerMoves;
int FirstMove;
label InfoText, InfoTextBack;
int gameover;
menuitem test_eval_flag;


// foreward decl.
int eval(int c);
int evalmove(int x, int y, int c);
void move_to(int x, int y, int color);
int checkfive(int c);
void prompt_move();
void getstatall(void);



void resize(Fl_Widget *wgt_, const rect &r_)
{
	wgt_->size(r_.width, r_.height);
	wgt_->position(r_.x, r_.y);
}

void redraw(Fl_Widget *wgt_)
{
	wgt_->redraw();
}

void redim(int gridsize)
{
	GRIDSIZE = gridsize;
	BORDERSIZE = GRIDSIZE;

	OX = (2 * BORDERSIZE);
	OY = (2 * BORDERSIZE) + MENU_HEIGHT;

	BOARDSIZE = (18 * GRIDSIZE);
	RX = (OX + BOARDSIZE + BORDERSIZE);
	UY = (OY + BOARDSIZE + BORDERSIZE);
	UC = (BOARDSIZE / 2 + OY);

	PIECESIZE = (GRIDSIZE - GRIDSIZE / 6);

	PBOXSIZE = (PIECESIZE * 8);
	if ( MainWindow )
	{
		resize(MainWindow, rect(0, 0, RX + 2 * PBOXSIZE, UY + OY));
		resize(InfoTextBack, rect(RX + PBOXSIZE / 2 - 2, UC - 100 / 2 - 2, PBOXSIZE + 4, 104));
		resize(InfoText, rect(RX + PBOXSIZE / 2, UC - 100 / 2, PBOXSIZE, 100));
		redraw(MainWindow);
	}
} // redim


void fillarc(const rect& r_, double s_, double e_)
{
	fl_pie( r_.x, r_.y, r_.width, r_.height, s_, e_ );
}

void drawarc(const rect& r_, double s_, double e_)
{
	fl_arc( r_.x, r_.y, r_.width, r_.height, s_, e_ );
}

void fcircle(int x, int y, int r)
{
	/* draws a filled circle */
	rect rc;
	rc.x = x - r;
	rc.y = y - r;
	rc.width = 2 * r;
	rc.height = 2 * r;
	fillarc(rc, 0, 360);
}

void circle(int x, int y, int r)
{
	/* draws a filled circle */
	rect rc;
	rc.x = x - r;
	rc.y = y - r;
	rc.width = 2 * r;
	rc.height = 2 * r;
	drawarc(rc, 0, 360);
}

void setcolor(int c_)
{
	fl_color(c_);
}

void drawdot(int x, int y, int color)
{
//	color = colors[color];
	if ( x < 19 )
		x = OX + x * GRIDSIZE;
	if ( y < 19 )
		y = OY + y * GRIDSIZE;

	setcolor(DarkGrey);
	fcircle(x, y, 2);
}

void drawpiece(int x, int y, int color)
{
	color = colors[color];
	if ( x < 19 )
		x = OX + x * GRIDSIZE;
	if ( y < 19 )
		y = OY + y * GRIDSIZE;

	setcolor(DarkGrey);
	fcircle(x + 1, y + 1, PIECESIZE / 2);

	if (color)
		setcolor(BLACKCOLOR);
	else
		setcolor(WHITECOLOR);
	fcircle(x, y, PIECESIZE / 2);

	if (color)
		setcolor(WHITECOLOR);
	else
		setcolor(BLACKCOLOR);
	circle(x, y, PIECESIZE / 2 + 1);

} // drawpiece


void drawtable(int c)
{
	/* 0=player, 1=computer */
	int i, o;
	if ( c )
		o = -1;
	else
		o = 1;
	setcolor(Black);
	fcircle(RX + PBOXSIZE + 5, UC + PBOXSIZE * o + 5, PBOXSIZE / 2);
	setcolor(DarkRed);
	fcircle(RX + PBOXSIZE, UC + PBOXSIZE * o, PBOXSIZE / 2);
	setcolor(Black);
	fcircle(RX + PBOXSIZE, UC + PBOXSIZE * o, PBOXSIZE / 2 - PBOXSIZE / 7);
	for (i = 0; i < anz_pieces[c]; i++)
	{
		int ox = rand() % ((PBOXSIZE - PIECESIZE) / 4);
		int oy = rand() % ((PBOXSIZE - PIECESIZE) / 4);
		if ( rand() % 2 )
			ox = -ox;
		if ( rand() % 2 )
			oy = -oy;
		drawpiece(RX + PBOXSIZE + ox, UC + PBOXSIZE * o + oy, c);
	}
} // drawtable

void fillrect(const rect& r_)
{
	fl_rectf(r_.x, r_.y, r_.width, r_.height);
}

void drawline(const pt& s_, const pt& e_)
{
	fl_line(s_.x, s_.y, e_.x, e_.y);
}

void drawboard(int ox, int oy)
{
	int i;
	setcolor(Black);
	fillrect(rect(ox - BORDERSIZE + 5, oy - BORDERSIZE + 5, 18 * GRIDSIZE + 2 * BORDERSIZE, 18 * GRIDSIZE + 2 * BORDERSIZE));
//	setcolor(0x00AA8D6EL);
	setcolor(0xAA8D6E00L);
	fillrect(rect(ox - BORDERSIZE, oy - BORDERSIZE, 18 * GRIDSIZE + 2 * BORDERSIZE, 18 * GRIDSIZE + 2 * BORDERSIZE));
	setcolor(Black);
	for (i = ox; i <= ox + 18 * GRIDSIZE; i += GRIDSIZE)
		drawline(pt(i, oy), pt(i, oy + 18 * GRIDSIZE ));
	for (i = oy; i <= oy + 18 * GRIDSIZE; i += GRIDSIZE)
		drawline(pt(ox, i), pt(ox + 18 * GRIDSIZE, i));
}

static void cb_to(void *d_)
{
	if ( TimerFunc )
		(*TimerFunc)(d_);
}

void settimer(double delay_)
{
	Fl::remove_timeout( cb_to );
	if ( TimerFunc && delay_ )
		Fl::add_timeout(delay_/1000, cb_to);
}

void settext(Fl_Widget *wgt_, const char *l_=0)
{
	wgt_->copy_label(l_);
	wgt_->redraw();
}

void setcursor(Fl_Cursor c_)
{
	fl_cursor(c_);
}

void find_best_move(move *best_move, int c)
{
	int x, y;
	int max = -1000000;
	int e;

	for (x = 0; x < 19; x++)
	{
		for (y = 0; y < 19; y++)
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

				else if ( e == max && rand() % 2 )
				{
					max = e;
					best_move->x = x;
					best_move->y = y;
				}
			}
		}
	}
} // find_best_move

void askok(const char *l_)
{
	fl_alert("%s", l_);
}

void make_move(void *data)
{
	move best_move;
	settimer(0);
	settext(InfoText, "I'm thinking...");
	setcursor(WatchCursor);
	find_best_move(&best_move, 2);

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
} // make_move


void prompt_move()
{
	settext(InfoText, "Your move please!");
}


int free_pos(int x, int y)
{
	return board[x][y] == 0;
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

	c = board[x0][y0]; /* 0=empty, 1=player, 2=computer */
	switch ( dir )
	{
	case HORI:  dx = 1; dy = 0; break;
	case VERTI: dx = 0; dy = 1; break;
	case LU_RO: dx = 1; dy = 1; break;
	case LO_RU: dx = 1; dy = -1; break;
	default:
		assert(false);
	}

	anz = 1;
	x = x0 + dx;
	y = y0 + dy;
	while ( x >= 0 && x < 19 && y >= 0 && y < 19 && board[x][y] == c && anz < 6 )
	{
		x += dx; y += dy; anz++;
	}
	x1 = x; y1 = y;
	dx = -dx;
	dy = -dy;
	x = x0 + dx;
	y = y0 + dy;
	while ( x >= 0 && x < 19 && y >= 0 && y < 19 && board[x][y] == c && anz < 6 )
	{
		x += dx; y += dy; anz++;
	}
	x2 = x; y2 = y;
	gstat[x0][y0].dir[dir].is_anz = anz;

	x = x1; y = y1;
	dx = -dx;
	dy = -dy;
	f1 = 0;
	while ( x >= 0 && x < 19 && y >= 0 && y < 19 && board[x][y] == 0 && anz < 6 )
	{
		x += dx; y += dy; if ( c )
			anz++;
		f1++;
	}
	x = x2; y = y2;
	f2 = 0;
	dx = -dx;
	dy = -dy;
	while ( x >= 0 && x < 19 && y >= 0 && y < 19 && board[x][y] == 0 && anz < 6 )
	{
		x += dx; y += dy; if ( c )
			anz++;
		f2++;
	}

	gstat[x0][y0].dir[dir].poss_anz = anz;
	gstat[x0][y0].dir[dir].freedom1 = f1;
	gstat[x0][y0].dir[dir].freedom2 = f2;

} // getstatpos


int checkfive(int c)
{
	int x, y, d;
	getstatall();
	for (x = 0; x < 19; x++)
	{
		for (y = 0; y < 19; y++)
		{
			if ( board[x][y] == c )
			{
				for (d = 0; d < 4; d++)
					if  ( gstat[x][y].dir[d].is_anz == 5 )
						return 1;
			}
		}
	}
	return 0;
}

void getstatall(void)
{
	int x, y;

	for (x = 0; x < 19; x++)
	{
		for (y = 0; y < 19; y++)
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
	int e = 0, d;
	for (x = 0; x < 19; x++)
	{
		for (y = 0; y < 19; y++)
		{
			if ( board[x][y] == c )
			{
				for (d = 0; d < 4; d++)
				{
					e += 50 * ( gstat[x][y].dir[d].is_anz == 2 && gstat[x][y].dir[d].poss_anz >= 5 );
					e += 200 * ( gstat[x][y].dir[d].is_anz == 3 && gstat[x][y].dir[d].poss_anz >= 5 && (gstat[x][y].dir[d].freedom1 && gstat[x][y].dir[d].freedom2));
					e += 700 * ( gstat[x][y].dir[d].is_anz == 4  && (gstat[x][y].dir[d].freedom1 || gstat[x][y].dir[d].freedom2));
					e += 5000 * ( gstat[x][y].dir[d].is_anz == 5 );
					e += 20 * ( gstat[x][y].dir[d].poss_anz >= 5 );
					e += 5 * ( (gstat[x][y].dir[d].freedom1 != 0) + (gstat[x][y].dir[d].freedom2 != 0) );
					e += 3 * (gstat[x][y].dir[d].freedom1 >= 4 && gstat[x][y].dir[d].freedom2 >= 4);
					e += 70 * (x > 6 && x < 12 && y > 6 && y < 12);
				}
			}
		}
	}
	return e;
} // eval

int ischecked(Fl_Menu_Item *m_)
{
	if ( !m_ )
		return false;
	return m_->checked();
}

int evalmove(int x, int y, int c)
{
	int my_strength, opp_strength;
	int co = (c & 1) + 1;

	board[x][y] = c;
	getstatall();

	my_strength = eval(c);
	opp_strength = eval(co);

	int my_future_strength, opp_future_strength;
	board[x][y] = co;
	getstatall();

	my_future_strength = eval(co);
	opp_future_strength = eval(c);

	board[x][y] = 0;

	if ( my_future_strength > my_strength )
		my_strength = my_future_strength;
	if ( opp_future_strength > opp_strength )
		opp_strength = opp_future_strength;

	if ( ischecked(test_eval_flag) )
	{
		char str[100];
		sprintf(str, "%d/%d:\n %d : %d -> %d\n", x, y, my_strength, opp_strength, my_strength - opp_strength);
		settext(InfoText, str);
	}

	return my_strength - 2 * opp_strength;
}

void move_to(int x, int y, int color)
{
	LastMove.x = x;
	LastMove.y = y;
	LastMove.color = color;
	movelist[anz_moves].x = x;
	movelist[anz_moves].y = y;
	movelist[anz_moves].color = color;
	anz_moves++;
	anz_pieces[color]--;
	board[x][y] = color + 1; /* 0=empty, 1=player, 2=computer */
   Fl::first_window()->redraw();
}

void mousebutton(control c, int buttons, const point& xy)
{
	if ( gameover )
		return;

	if ( xy.x >= OX - PIECESIZE / 2 && xy.y >= OY - PIECESIZE / 2 )
	{
		int x = (xy.x - OX + PIECESIZE / 2) / GRIDSIZE;
		int y = (xy.y - OY + PIECESIZE / 2) / GRIDSIZE;
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
					evalmove(x, y, 2);
				}
			}
		}
	}
} // mousebutton


void drawpiecelist(void)
{
	int i;
	for (i = 0; i < anz_moves; i++)
		drawpiece(movelist[i].x, movelist[i].y, movelist[i].color);
	if ( LastMove.x > 0 )
	{
		drawdot(LastMove.x, LastMove.y, LastMove.color);
	}
}

//void redraw_window(window w, rect r)
void redraw_window(void)
{
	drawboard(OX, OY);
	drawpiecelist();
	drawtable(0);
	drawtable(1);
}

void show( FWindow *w_)
{
	w_->end();
	w_->show();
}

window newwindow(const char *title_, const rect &r_, int flags_ = 0 )
{
	FWindow *win = new FWindow ( r_.x, r_.y, r_.width, r_.height, title_ );
	if ( flags_ & Modal )
		win->set_modal();
	if ( ( flags_ & Centered ) && Fl::first_window() )
		win->position( Fl::first_window()->x() + ( Fl::first_window()->w() - r_.width ) / 2,
	                  Fl::first_window()->y() + ( Fl::first_window()->h() - r_.height ) / 2 );
	return win;
}

label newlabel(const char *title_, const rect &r_, int flags_ = 0 )
{
	Fl_Box *win = new Fl_Box ( r_.x, r_.y, r_.width, r_.height, title_ );
	win->box(FL_FLAT_BOX);
	return win;
}


void setbackground(Fl_Widget *wgt_, int color_)
{
	wgt_->color(color_);
}

void info(const char *title, const char *text)
{
	window about;

	about = newwindow(title, rect(10, 10, 300, 200), Centered + Modal + Titlebar + Closebox);
	setbackground(about, LightGrey);
	newlabel(text, rect(10, 10, 280, 180), Center + VCenter);
	show(about);

// askok(text);
}

//void about(menuitem m)
void about(Fl_Widget *, void *)
{
	info("About Gomoku", "***** G O M O K U *****\nFive in a row\n\n(c) 2000\nChristian Grabner\n"
	     "\nConvered to FLTK\n(c) 2018 wcout <wcout@@gmx.net>");
}

void exitapp()
{
	while ( Fl::first_window() )
		Fl::first_window()->hide();
}

//void quit(menuitem m)
void quit(Fl_Widget *, void *)
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

	memset(gstat, 0, sizeof(gstat));
	ComputerMoves = FirstMove;
	FirstMove = !FirstMove;
	LastMove.x = -1;

	if ( ComputerMoves )
		settimer(100);  /* make_move() is called */
	else
		prompt_move();
}

time_t currenttime()
{
	return time(0);
}

//void new_game(menuitem m)
void new_game(Fl_Widget *, void *)
{
	srand(currenttime());

	newgame();
}

//void change_colors(menuitem m)
void change_colors(Fl_Widget *, void *)
{
	int temp;
	if ( gameover )
		return;

	temp = colors[0];
	colors[0] = colors[1];
	colors[1] = temp;
	redraw(MainWindow);
}

//void take_back(menuitem m)
void take_back(Fl_Widget *, void *)
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

//void hint(menuitem m)
void hint(Fl_Widget *, void *)
{
	move best_move;

	if ( gameover )
		return;

	find_best_move(&best_move, 1);
	move_to(best_move.x, best_move.y, 0);
	redraw(MainWindow);
	ComputerMoves = 1;
	if ( !ischecked(test_eval_flag) )
		settimer(100);
}

void uncheck(Fl_Menu_Item *m_)
{
	m_->clear();
}

void check(Fl_Menu_Item *m_)
{
	m_->set();
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

//void smaller(menuitem m)
void smaller(Fl_Widget *, void *)
{
	if ( GRIDSIZE > 12 )
		redim(GRIDSIZE - 1);
}

//void bigger(menuitem m)
void bigger(Fl_Widget *, void *)
{
	if ( GRIDSIZE < 40 )
		redim(GRIDSIZE + 1);
}

void setredraw(FWindow *win_, void (*d_)(void) )
{
	win_->setDraw( d_ );
}

void setmousedown(FWindow *win_, void (*d_)(Fl_Widget *, int, const pt&) )
{
	win_->setMouse( d_ );
}

void settimerfn(void (*d_)(void *), void *p_ = 0)
{
	TimerFunc = d_;
}

void mainloop()
{
	Fl::run();
}

int main(void)
{
	redim(GRIDSIZE);

	MainWindow = newwindow("GOMOKU", rect(30, 30, RX + 2 * PBOXSIZE, UY + OY), StandardWindow + UsePalette);
	setbackground(MainWindow, 0xA0CACA00L);
//	setbackground(MainWindow, 0x00A0CACAL);

	setredraw(MainWindow, redraw_window);

	Fl_Menu_Bar *menubar = new Fl_Menu_Bar(0,0,MainWindow->w(),MENU_HEIGHT);
//	newmenubar(NULL);
	Fl_Menu_Item menutable[] = {
		{"File", 0, 0, 0, FL_SUBMENU },
			{"&About...", FL_ALT+'a', about},
			{"&Quit", FL_ALT+'q', quit},
			{0},

			{"Display", 0, 0, 0, FL_SUBMENU },
				{"Bigger", FL_ALT+'l', bigger},
				{"Smaller", FL_ALT+'s', smaller},
			{0},

			{"Game", 0, 0, 0, FL_SUBMENU },
				{"New game", FL_ALT+'n', new_game},
				{"Change colors", FL_ALT+'c', change_colors},
			{0},

			{"Move",0, 0, 0, FL_SUBMENU },
				{"Take back", FL_ALT+'b', take_back},
				{"Hint...", FL_ALT+'h', hint},
			{0}
		};
	menubar->menu(menutable);
#if 0
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
#endif

	setbackground((InfoTextBack = newlabel("", rect(RX + PBOXSIZE / 2 - 2, UC - 100 / 2 - 2, PBOXSIZE + 4, 104), Center + VCenter)), Black);
	InfoText = newlabel("", rect(RX + PBOXSIZE / 2, UC - 100 / 2, PBOXSIZE, 100), Center + VCenter);
	setbackground(InfoText, White);

	show(MainWindow);

	srand(currenttime());

	FirstMove = rand() % 2;
	settimerfn(make_move, NULL);

	newgame();

	setmousedown(MainWindow, mousebutton);

	mainloop();
} // main
