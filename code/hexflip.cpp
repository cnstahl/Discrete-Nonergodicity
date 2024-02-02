
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <array>
#include <functional>

const int mult    = 2;
const int L       = 6*mult;     // MUST BE EVEN
      int l       = 0;     // l=L is fully packed
      int l_skip  = mult;
const int width   = L;     // System width 
const int height  = L/2;   // System height: let width/height =2
const int m = 3;           // number of NONTRIVIAL colors
int step  = 0;             // used for filenames
int init_steps = 300;         // used to thermalize the system
int prints = 10000;            // simulation time
int steps_per_print = 1;    // total steps = prints * steps_per_print
int run = 0; 
int runs = 10;
bool vertical = false;
std::string type = "autoc";

// declare random number generator outside of main
std::random_device rd;  // Used to obtain a seed for the random number engine
std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<int> rand_color(0,m);
std::uniform_int_distribution<int> rand_diff(1 , m);
std::uniform_int_distribution<int> rand_row(0,height-1);
std::uniform_int_distribution<int> rand_col(0,width-1);
std::uniform_int_distribution<int> rand_sub(0, 1);

//***************************************************************************//
//*** Timing info                                                         ***//
//***************************************************************************//

// auto start = std::chrono::steady_clock::now();
// auto end = std::chrono::steady_clock::now();
// std::chrono::duration<double> diff = end - start;
		
//***************************************************************************//
//*** Updating the state                                                  ***//
//***************************************************************************//

std::array<int, 6*height*width> underlying_spins;

int index_from_coords(int row, int column) {
	row = (row + (6*height)) % (6*height);
	column = (column + width) % width;
	return row*width + column;
}

void reset() {
	underlying_spins = {};
}

void set_all(int background) {
	for (auto &v : underlying_spins) v = background;
}

int get_spin(int row, int column) {
	// return the state of this spin
	return underlying_spins[index_from_coords(row, column)];
}

void set_spin(int row, int column, int color) {
	// flip to a different color 
	underlying_spins[index_from_coords(row, column)] = color;
}

bool flippable(int a, int b, int c, int d, int e, int f, int color) {
	return ((a == color or a == 0) and 
			(b == color or b == 0) and 
			(c == color or c == 0) and 
			(d == color or d == 0) and 
			(e == color or e == 0) and 
			(f == color or f == 0));
}

bool all_zero(int a, int b, int c, int d, int e, int f) {
	return ((a == 0) and 
			(b == 0) and 
			(c == 0) and 
			(d == 0) and 
			(e == 0) and 
			(f == 0));
}

// set all spins to a new color (not zero)
void all_flip(int row, int col, int sub) {
	int new_color = rand_diff(gen);
	if (0 == sub) {
		set_spin(6*row    , col,     new_color);
		set_spin(6*row + 1, col,     new_color);
		set_spin(6*row + 2, col,     new_color);
		set_spin(6*row + 2, col + 1, new_color);
		set_spin(6*row + 3, col,     new_color);
		set_spin(6*row + 4, col,     new_color);
	}
	else if (1 == sub) {
		set_spin(6*row + 4, col,     new_color);
		set_spin(6*row + 3, col + 1, new_color);
		set_spin(6*row + 5, col,     new_color);
		set_spin(6*row + 5, col + 1, new_color);
		set_spin(6*row + 7, col,     new_color);
		set_spin(6*row + 6, col + 1, new_color);
	}
}

void color_flip(int row, int col, int sub, int color) {
	if (0 == sub) {
		set_spin(6*row    , col,     color - get_spin(6*row    , col    ));
		set_spin(6*row + 1, col,     color - get_spin(6*row + 1, col    ));
		set_spin(6*row + 2, col,     color - get_spin(6*row + 2, col    ));
		set_spin(6*row + 2, col + 1, color - get_spin(6*row + 2, col + 1));
		set_spin(6*row + 3, col,     color - get_spin(6*row + 3, col    ));
		set_spin(6*row + 4, col,     color - get_spin(6*row + 4, col    ));
	}
	else if (1 == sub) {
		set_spin(6*row + 4, col,     color - get_spin(6*row + 4, col    ));
		set_spin(6*row + 3, col + 1, color - get_spin(6*row + 3, col + 1));
		set_spin(6*row + 5, col,     color - get_spin(6*row + 5, col    ));
		set_spin(6*row + 5, col + 1, color - get_spin(6*row + 5, col + 1));
		set_spin(6*row + 7, col,     color - get_spin(6*row + 7, col    ));
		set_spin(6*row + 6, col + 1, color - get_spin(6*row + 6, col + 1));
	}
}

void update() {
	// the six edges around a plaquette
	int a, b, c, d, e, f;

	// choose a hexigon (x is row, y is col, sub is sublattice)
	int row = rand_row(gen);
	int col = rand_col(gen);
	int sub = rand_sub(gen);

	if (0 == sub) {
		a = get_spin(6*row    , col    );
		b = get_spin(6*row + 1, col    );
		c = get_spin(6*row + 2, col    );
		d = get_spin(6*row + 2, col + 1);
		e = get_spin(6*row + 3, col    );
		f = get_spin(6*row + 4, col    );
	}
	else if (1 == sub) {
		a = get_spin(6*row + 4, col    );
		b = get_spin(6*row + 3, col + 1);
		c = get_spin(6*row + 5, col    );
		d = get_spin(6*row + 5, col + 1);
		e = get_spin(6*row + 7, col    );
		f = get_spin(6*row + 6, col + 1);
	}

	if (all_zero(a, b, c, d, e, f)) {
		all_flip(row, col, sub);
		return;
	}

	for (int color = 1; color < m+1; color++) {
		if (flippable(a, b, c, d, e, f, color)) {
			color_flip(row, col, sub, color);
			return;
		}
	}
}

void time_step(int time) {
	for (int i = 0; i < time*height*width; i++) update();
}

void print_state() {
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			std::cout << " " << get_spin(6*row, col) << " " << get_spin(6*row + 1, col);
		}
		std::cout << " " << "\n";
		for (int col = 0; col < width; col++) {
			std::cout << get_spin(6*row+2, col) << "   ";
		}
		std::cout << get_spin(6*row+2, 0) << " " << "\n";
		for (int col = 0; col < width; col++) {
			std::cout << " " << get_spin(6*row+3, col) << " " << get_spin(6*row+4, col);
		}
		std::cout << " " << "\n" << "  ";
		for (int col = 0; col < width; col++) {
			std::cout << get_spin(6*row+5, col) << "   ";
		}
		std::cout << "\n";
	}
	for (int col = 0; col < width; col++) {
		std::cout << " " << get_spin(0, col) << " " << get_spin(1, col);
	}
	std::cout << "\n";
}

//***************************************************************************//
//*** Inserting nontrivial loops                                          ***//
//***************************************************************************//

void insert_loop() {
	for (int row = 0; row < height; row++) {
		set_spin(6*row,   0, 1);
		set_spin(6*row+2, 0, 1);
		set_spin(6*row+3, 0, 1);
		set_spin(6*row+5, 0, 1);
	}
}

void insert_big_loops() {
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < 6; col ++) {
			set_spin(6*row,   col, col%3+1);
			set_spin(6*row+2, col, col%3+1);
			set_spin(6*row+3, col, col%3+1);
			set_spin(6*row+5, col, col%3+1);
		}
	}
}

// insert a label of length count. Label is 12121212..... with a 3 at the end if needed
void insert_vert_label(int count) {
	// if (count > width) ;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < 2*(count/2); col++) {
			set_spin(6*row,   col, col%2+1);
			set_spin(6*row+2, col, col%2+1);
			set_spin(6*row+3, col, col%2+1);
			set_spin(6*row+5, col, col%2+1);
		}
		if (count%2) {
			set_spin(6*row,   count-1, 3);
			set_spin(6*row+2, count-1, 3);
			set_spin(6*row+3, count-1, 3);
			set_spin(6*row+5, count-1, 3);
		}
	}
}

// insert a label of length count. Label is 12121212..... with a 3 at the end if needed
void insert_horz_label(int count) {
	for (int row = 0; row < (count/2); row++) {
		for (int col = 0; col < width; col++) {
			set_spin(6*row,   col, 1);
			set_spin(6*row+1, col, 1);
			set_spin(6*row+3, col, 2);
			set_spin(6*row+4, col, 2);
		}
	}
	if (count%2) {
		for (int col = 0; col < width; col++) {
			set_spin(3*count-3, col, 3);
			set_spin(3*count-2, col, 3);
		}
	}
}

//***************************************************************************//
//*** Observables                                                         ***//
//***************************************************************************//

int N_flippable() {
	int a, b, c, d, e, f;
	int total = 0;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			// first sublattice
			a = get_spin(6*row    , col    );
			b = get_spin(6*row + 1, col    );
			c = get_spin(6*row + 2, col    );
			d = get_spin(6*row + 2, col + 1);
			e = get_spin(6*row + 3, col    );
			f = get_spin(6*row + 4, col    );
			for (int color = 1; color < m+1; color++) {
				total += flippable(a, b, c, d, e, f, color);
			}
			//second sublattice
			a = get_spin(6*row + 4, col    );
			b = get_spin(6*row + 3, col + 1);
			c = get_spin(6*row + 5, col    );
			d = get_spin(6*row + 5, col + 1);
			e = get_spin(6*row + 7, col    );
			f = get_spin(6*row + 6, col + 1);
			for (int color = 1; color < m+1; color++) {
				total += flippable(a, b, c, d, e, f, color);
			}
		}
	}
	return total;
}

int is_color(int color) {
	int total = 0;
	for (int match : underlying_spins) {
		if (color == match) ++total;
	}
	return total;
}

//***************************************************************************//
//*** Printing info                                                       ***//
//***************************************************************************//

std::function<void(int time)> update_func = &time_step;
// std::function<void(int time)> update_func = &big_time_step;

// std::function<int()> observable = &total_s;
// std::string name = "total_s";

// std::function<int()> observable = &is_trivial;
// std::string name = "is_trivial";

std::function<int()> observable = &N_flippable;
std::string name = "flippable";

void print_N_flippable() {
	std::ofstream myfile;
	std::string filename = "../" + type + "/N_flippable_";

	// if (insert_nontrivial) {filename += "Nontrivial";}
	// if (small_loops)       {filename += "Loops";}

	filename += "L" + std::to_string(L) + "l" + std::to_string(l) + ".dat";

	myfile.open(filename);
	myfile << "Number of flippable plaquettes in a" + std::to_string(width) + "by"
			+ std::to_string(height) + "system. " + std::to_string(steps_per_print) + "time per step.\n";

	for (run = 0; run < runs; run++) {
		reset();
		if (l>0) insert_horz_label(l);
		myfile << observable();
		// if (insert_nontrivial) {insert_big_loops();}
		// if (small_loops) {insert_small_loops();}
		for (step = 0; step < prints; step++) {
			update_func(steps_per_print);
			myfile << ", " << observable();
		}
		myfile << "\n";
	}
}

//***************************************************************************//
//*** Main                                                                ***//
//***************************************************************************//

int main(int argc, char *argv[]) {
	// print_state();
	// std::cout << "\n\n\n";

	for (l = 0; l < L; l+=l_skip) {
		print_N_flippable();
	}

	// insert_horz_label(9);
	// insert_big_loops();
	// print_state();
}






