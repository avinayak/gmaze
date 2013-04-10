/*

	gmaze project: a clutter based maze game for linux
	Version:
		1.0.1
	Author:
		gen3
		su.atul.vi@gmail.com
	Bugs:
		1.one cell before winning,the result pops up..
		2.used system(zenity) for win/lose message.. not very neat.
		3.remove all the warnings during compile.
	Todo:
		1.Lots:The algorithms are slow.Made from bits and pieces found from the internet.I never properly profiled them.
		2.GTK-izing it using a clutter embedded widget.That will solve most problems.
		3.Currently,only square mazes can be made.size can be increased using xsize parameter.
			did that to simplify my task.a ysize is easy to include though.
		3.Modularizing:I dont like a big source file.
		4.Provide a make file which does whats written below
		5.Level based challenges.
	Requirements:
		libclutter,gtk+
	Compile:
		$ gcc gmaze.c -o gmaze `pkg-config clutter-1.0 --cflags --libs`
	Run:
		$ ./gmaze
	
*/
#include<clutter/clutter.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<gdk/gdkkeysyms.h>
#include<gtk/gtk.h>

#define WALL_COL {0,0,0,255};		//black	
#define RUNNER_COL {0,0,255,0};		//blue
#define FLOOR_COL {255,255,255,255};	//white
#define MARKER_COL {255,90,0,255};	//orange
#define RUNNER_SPEED 20
#define SOLVER_SPEED 52	//found out after a lot of testing
#define CELL_W 15
#define MAZE_N (xsize*2-3)
#define UP    	0     //-y
#define DOWN  	1     //+y
#define LEFT  	2     //-x
#define RIGHT 	3     //+x
#define WHITE   a[pp++]=1
#define BLACK   a[pp++]=0
#define xsize 	20	//increase or decrease to change size

static int pp;
int won=0;
int runnerx=1;
int runnery=1;
int solverx=MAZE_N-2;
int solvery=MAZE_N-2;
int steps=0;
char  a[MAZE_N*MAZE_N];
char  b[MAZE_N*MAZE_N];
ClutterActor *stage;
ClutterActor *runner;
ClutterActor *solver;
ClutterColor * runner_col;
ClutterColor * solver_col;
long numin=1;
struct cell{
        int in;  
        int up;  
        int left;
        int prevx, prevy; 
};
struct cell MAZE[xsize][xsize];

void initialize()
{

	int x,y;
        for(x=0;x<xsize;x++){
                for(y=0;y<xsize;y++){
                        MAZE[x][y].in   = (x==0||x==xsize-1||y==0||y==xsize-1)?1:0;
                        MAZE[x][y].up   = (x==0||x==xsize-1||y==0)?0:1;
                        MAZE[x][y].left = (x==0||y==0||y==xsize-1)?0:1;
                }
        }
        return;
}
 
void generate()
{
	//backtracking.obtained from a wiki page.
        int xcur=1, ycur=1;
        MAZE[xcur][ycur].in = 1;
        int whichway;
        int success;
        do{


                while( MAZE[xcur][ycur-1].in&&MAZE[xcur][ycur+1].in&&
                           MAZE[xcur-1][ycur].in&&MAZE[xcur+1][ycur].in ){
                                   
                                int xcur2=MAZE[xcur][ycur].prevx;
                                ycur=MAZE[xcur][ycur].prevy;
                                xcur=xcur2;
                }

                do{
                        
                        success=0;
                        whichway=rand()%4;
                        switch(whichway){
                        case UP:
                                if(!MAZE[xcur][ycur-1].in){
                                        success=1;
                                        MAZE[xcur][ycur].up=0;
                                        MAZE[xcur][ycur-1].prevx=xcur;
                                        MAZE[xcur][ycur-1].prevy=ycur;
                                        ycur--;
                                }
                                break;
                        case DOWN:
                                if(!MAZE[xcur][ycur+1].in){

									success=1;
                                        MAZE[xcur][ycur+1].up=0;
                                        MAZE[xcur][ycur+1].prevx=xcur;
                                        MAZE[xcur][ycur+1].prevy=ycur;
                                        ycur++;
                                }
                                break;
                        case LEFT:
                                if(!MAZE[xcur-1][ycur].in){
                                        success=1;
                                        MAZE[xcur][ycur].left=0;
                                        MAZE[xcur-1][ycur].prevx=xcur;
                                        MAZE[xcur-1][ycur].prevy=ycur;
                                        xcur--;
                                }
                                break;
                        case RIGHT:
                                if(!MAZE[xcur+1][ycur].in){
                                        success=1;
                                        MAZE[xcur+1][ycur].left=0;
                                        MAZE[xcur+1][ycur].prevx=xcur;
                                        MAZE[xcur+1][ycur].prevy=ycur;
                                        xcur++;
                                }
                                break;
                        }
                }while(!success);
                MAZE[xcur][ycur].in=1;
                numin++;
        }while(numin<(xsize-2)*(xsize-2));
        return;
}

void create()
{
        int x, y, n;
        int width=(xsize-1)*2-1;
        int height=(xsize-1)*2-1;
	//nicely print to the array!
        for(y = 0; y <= height - 1; y++)
	{
                for(x = 0; x <= width - 1; x++)
		{
                        if(x%2 == 1 && y%2 == 1)
			{                                
                        	if(MAZE[x/2+1][y/2+1].in) WHITE; else BLACK;
                        }
			else if(x%2 == 0 && y%2 == 0)
			{
                                BLACK;
                        }
			else if(x%2 == 0 && y%2 == 1)
			{
                                if(MAZE[x/2+1][y/2+1].left) BLACK; else WHITE;
                        }
			else if(x%2 == 1 && y%2 == 0)
			{
                                if(MAZE[x/2+1][y/2+1].up) BLACK; else WHITE;
                        }
                }
                
        }

        return;
}
void solve()
{		//based on the video dead end filling in wikipedia
		int i,j,f=1;
		for(i=0;i<MAZE_N*MAZE_N;i++)
			b[i]=a[i];
		while(f)
		{
			f=0;//suicide!
			for(i=1;i<MAZE_N-1;i++)
				for(j=1;j<MAZE_N-1;j++)
					if(b[MAZE_N*j+i]==1)
					{						
						if(b[MAZE_N*(j+1)+(i)]+
					 	   b[MAZE_N*(j-1)+(i)]+
					   	   b[MAZE_N*(j)+(i+1)]+
				
	   	   b[MAZE_N*(j)+(i-1)] == 1 && 
					   	     MAZE_N*j+i!= MAZE_N*1+1 && 
					   	     MAZE_N*j+i!= MAZE_N*(MAZE_N-2)+MAZE_N-2)
						{	
							f++;
							b[MAZE_N*j+i]=0;
						}
					}
		}
		for(i=0;i<MAZE_N*MAZE_N;steps+=b[i++]);	//steps counter
}
				
ClutterActor *create_wall(int x,int y)
{
	ClutterColor wall_col=WALL_COL;
	ClutterActor *rect= clutter_rectangle_new_with_color(&wall_col);
	clutter_actor_set_size(rect,CELL_W,CELL_W);
	clutter_actor_set_position(rect,x,y);
	clutter_container_add_actor(CLUTTER_CONTAINER(stage),rect);
	clutter_actor_show(rect);
	return rect;
}

ClutterActor *mark(int x,int y)
{
	//a function to mark any point on the maze
	ClutterColor wall_col=MARKER_COL;
	ClutterActor *rect= clutter_rectangle_new_with_color(&wall_col);
	clutter_actor_set_size(rect,CELL_W,CELL_W);
	clutter_actor_set_position(rect,x,y);
	clutter_container_add_actor(CLUTTER_CONTAINER(stage),rect);
	clutter_actor_show(rect);
	return rect;
}

ClutterActor *draw_runner(int x,int y)
{
	runner_col=clutter_color_new(0,0,255,255);
	runner= clutter_rectangle_new_with_color(runner_col);
	clutter_actor_set_size(runner,CELL_W,CELL_W);
	clutter_actor_set_position(runner,x,y);
	clutter_container_add_actor(CLUTTER_CONTAINER(stage),runner);
	clutter_actor_show(runner);
	return runner;
}

ClutterActor *draw_solver(int x,int y)
{
	solver_col=clutter_color_new(255,0,0,255);
	solver= clutter_rectangle_new_with_color(solver_col);
	clutter_actor_set_size(solver,CELL_W,CELL_W);
	clutter_actor_set_position(solver,x,y);
	clutter_container_add_actor(CLUTTER_CONTAINER(stage),solver);
	clutter_actor_show(solver);
	return solver;
}

static gboolean runner_react(ClutterStage *stage, ClutterKeyEvent *event, gpointer data)
{	
	int j;
	if((event->keyval==GDK_KEY_Down) && a[(runnery+1)*MAZE_N+runnerx])
	{	runnery+=1;
		for(j=0;a[MAZE_N*(runnery+j)+runnerx]!=2;j++);
		runnery+=(j);

		clutter_actor_animate (runner, CLUTTER_LINEAR, RUNNER_SPEED*(j+1),
                         "x", (float)(runnerx*CELL_W),
                         "y", (float)(runnery*CELL_W),
                         NULL);
		
		
	}
	if((event->keyval==GDK_KEY_Up) 	&& a[(runnery-1)*MAZE_N+runnerx])
	{
		runnery-=1;
		for(j=0;a[MAZE_N*(runnery-j)+runnerx]!=2;j++);
		runnery-=(j);
		clutter_actor_animate (runner, CLUTTER_LINEAR, RUNNER_SPEED*(j+1),
                         "x", (float)(runnerx*CELL_W),
                         "y", (float)(runnery*CELL_W),
                         NULL);
		
	}
	if((event->keyval==GDK_KEY_Left) && a[(runnery)*MAZE_N+runnerx-1])
	{
		runnerx-=1;
		for(j=0;a[MAZE_N*(runnery)+runnerx-j]!=2;j++);
		runnerx-=(j);
		clutter_actor_animate (runner, CLUTTER_LINEAR, RUNNER_SPEED*(j+1),
                         "x", (float)(runnerx*CELL_W),
                         "y", (float)(runnery*CELL_W),
                         NULL);
	}
	if((event->keyval==GDK_KEY_Right) && a[(runnery)*MAZE_N+runnerx+1])
	{
		runnerx+=1;
		for(j=0;a[MAZE_N*(runnery)+runnerx+j]!=2;j++);
		runnerx+=(j);
		clutter_actor_animate (runner, CLUTTER_LINEAR, RUNNER_SPEED*(j+1),
                         "x", (float)(runnerx*CELL_W),
                         "y", (float)(runnery*CELL_W),
                         NULL);
	}

	
	if(runnerx==MAZE_N-2 && runnery==MAZE_N-2)
		won=1;
	
	return 0;
}

static void solver_react()
{	
	//mark(solverx*CELL_W,solvery*CELL_W); 
	//uncomment to above to trace solver path
	b[MAZE_N*(solvery)+(solverx)]=0;
	if(b[(solvery)*MAZE_N+solverx+1])
		solverx+=1;
	else if(b[(solvery+1)*MAZE_N+solverx])
		solvery+=1;
	else if(b[(solvery-1)*MAZE_N+solverx])
		solvery-=1;
	else if(b[(solvery)*MAZE_N+solverx-1])
		solverx-=1;
	clutter_actor_animate (solver, CLUTTER_LINEAR, SOLVER_SPEED,
                         "x", (float)(solverx*CELL_W),
                         "y", (float)(solvery*CELL_W),
                         NULL);
	if(solverx==1 && solvery==1)
	{

		if(!won)
			//i'm sorry but i'm no longer able to do all the api references on how to make a pop up notification
			// just tired.. you'll have to make do with zenity for now.. but i promise for a neat one later.. :(
			system("zenity --text \'Computer wins!\' --info");
		else
			system("zenity --text \'You win!\' --info");
	}
	
}
void junction_mark()
{
	int i,j;
	for(i=1;i<MAZE_N-1;i++)
		for(j=1;j<MAZE_N-1;j++)
			if      (   a[MAZE_N*j+i]       && 
				(!((a[MAZE_N*(j-1)+i]   && 
				    a[MAZE_N*(j+1)+i])  ||
				(   a[MAZE_N*(j)+i+1]   && 
				    a[MAZE_N*(j)+i-1])) ||
				((  a[MAZE_N*(j+1)+i]   &&
				    a[MAZE_N*(j-1)+i]   &&
				    a[MAZE_N*(j)+i-1])  ||
				(   a[MAZE_N*(j+1)+i]   &&
				    a[MAZE_N*(j-1)+i]   &&
				    a[MAZE_N*(j)+i+1])  ||
				(   a[MAZE_N*(j)+i+1]   &&
				    a[MAZE_N*(j)+i-1]   &&
				    a[MAZE_N*(j+1)+i])  ||
				(   a[MAZE_N*(j)+i+1]   &&
				    a[MAZE_N*(j)+i-1]   &&
				    a[MAZE_N*(j-1)+i]))))
					{
						a[MAZE_N*j+i]=2;
						//mark(i*CELL_W,j*CELL_W);
						//uncomment above to see junctions marked
					}
}

int main(int argc,char **argv)
{	
	
	int i=0,j=0;
 	srand((unsigned int)time(NULL));
        initialize();      
        generate();    
        create();
	solve();

	clutter_init(&argc,&argv);
	ClutterColor stage_color=FLOOR_COL;
	stage=clutter_stage_get_default();
	clutter_stage_set_color(CLUTTER_STAGE(stage),&stage_color);
	clutter_actor_set_size(stage,MAZE_N*CELL_W,MAZE_N*CELL_W);
	clutter_actor_show(stage);
	
	

	draw_runner(runnerx*CELL_W,runnery*CELL_W);
	draw_solver(solverx*CELL_W,solvery*CELL_W);
	junction_mark();

	for(i=0;i<MAZE_N;i++)
		for(j=0;j<MAZE_N;j++)
			if(a[MAZE_N*j+i]==0)
				create_wall(CELL_W*i,CELL_W*j);

	g_timeout_add (SOLVER_SPEED,(GSourceFunc)solver_react,NULL);
	g_signal_connect (stage, "key-press-event",G_CALLBACK (runner_react), NULL);
	g_message("STEPS REQD: %d",steps);	//from terminal?? you got it! th other way is to update a status bar of some sort
						//which i have no idea of.. have to do it later..
	
	clutter_main();
	return 0;
}
