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
