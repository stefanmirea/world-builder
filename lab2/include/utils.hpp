/* Stefan-Gabriel Mirea - 331CC */

/* definiri de constante simbolice, declaratii de clase, functii */
#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <DrawingWindow.h>
#include <Visual2D.h>
#include <Transform2D.h>
#include <Transform3D.h>
#include <Line2D.h>
#include <Rectangle2D.h>
#include <Circle2D.h>
#include <Polygon2D.h>
#include <cell.hpp>
#include <element.hpp>
#include <infinite_matrix.hpp>
#include <drawn_element.hpp>

#include <windows.h>
#include <iostream>
#include <cctype>
#include <sstream>
#include <fstream>
#include <list>
#include <algorithm>

#define PI                   3.14159265358979323846
#define inf                  1000000

/* directiile de deplasare */
#define DIR_NONE             0
#define DIR_CANCELED         1
#define DIR_LEFT             2
#define DIR_RIGHT            3
#define DIR_UP               4
#define DIR_DOWN             5
#define DIR_LEFT_UP          6
#define DIR_RIGHT_UP         7
#define DIR_LEFT_DOWN        8
#define DIR_RIGHT_DOWN       9

/* sensurile de rotatie */
#define DIR_CLOCKWISE        0
#define DIR_COUNTERCLOCKWISE 1

/* actiunea efectuata asupra reprezentarii selectiei */
#define DRAW                 0
#define ERASE                1

/* modurile in care vor fi interpretate comenzile la consola */
#define CONS_GLOBAL          0 /* se asteapta o comanda obisnuita */
#define CONS_LOCAL           1 /* se asteapta un raspuns la o intrebare */

/* actiuni ce depind de raspunsul la un dialog */
#define ACT_PROCEED          0 /* s-a dat un raspuns valid si se poate continua executia comenzii */
#define ACT_WAIT             1 /* inca se asteapta un raspuns */
#define ACT_ABORT            2 /* s-a dat un raspuns care a anulat executia comenzii */

/* perioada clipirii cursorului din linia de comanda (ms) */
#define BLINK_PERIOD         1000

/* dimensiunea implicita a laturii cuburilor */
#define EDGE_DEFAULT         71

/* dimensiunea maxima a laturii cuburilor */
#define EDGE_MAX             700

/* dimensiunea laturii cuburilor, asa cum se afiseaza in fereastra in urma proiectiei */
#define EDGE_PRINTED         (edge * static_cast<float>(sqrt(2.0 / 3)))

/* factorul de zoom */
#define ZOOM_FACTOR          1.3f

/* timpul maxim (ms) intre apasarile a doua sageti pentru a schimba directia intr-una "compusa"
 * (diagonala) */
#define DIAGONAL_DIR_TIME    100

/* lungimea laturii (in numar de cuburi) fragmentelor din cadrul lumii afisate la un momentdat */
/* Daca memoria e o problema, trebuie sa fie cat mai mic. Daca timpul de executie e o problema,
 * trebuie sa fie cat mai mare. */
#define FRAGMENT_SIZE        2

/* distanta suplimentara pentru separarea spatiului din coordonatele logice in care sunt afisate
 * elementele liniei de comanda de lumea 3D */
#define CLI_OFFSET           1000

/* raza ce determina aria de acoperire a pietelor */
#define RADIUS               6

/* valori pe care le poate returna find_command() */
#define CMD_NOT_FOUND        -1
#define CMD_HELP             -2 /* s-a solicitat ajutor (comanda se termina cu '?') */

/* modurile de editare */
#define EDIT_TERRAIN         0
#define EDIT_SQUARE          1
#define EDIT_HOUSES          2
#define EDIT_ROADS           3

/* tipuri de elemente de selectie */
#define SEL_NONE             0
#define SEL_SIMPLE           1
#define SEL_MULTIPLE         2

/* luminozitati */
#define BRIGHT               0
#define NORMAL               1
#define DARK                 2

using namespace std;

void execute(const string &full_command);
void add_cli_text(string text);
int compare_elements(const Cell &cell1, const int h1, const int sel_type1, const Cell &cell2,
                     const int h2, const int sel_type2);
bool erase_element(const Cell &cell, const int h, const int sel_type);
void draw_element(const Cell &cell, const int h, const int sel_type);
void manage_graphical_selection(const int action);
void draw_visible_objects(const bool reset_camera);
void cancel_multiple_selection();
void update_front_water(const Cell &first, const Cell &last);
void update_square_coverage(const bool draw_changes);
void update_square_connectivity();
void redraw_houses_around(const Cell &cell);
void redraw_roads_around(const Cell &cell);
bool combine(InfiniteMatrix<Element> &matrix, const Cell &cell, const int altitude,
             const unsigned char terrain_mask, const int dir);
void change_relief(const int dir);
void rotate_houses(const int way);
void rotate_camera(const int way);
void copy_houses(const bool keep_original);
int save_game(void (*caller)(stringstream &), const stringstream &caller_args,
              string &save_filename);
int save_dialog(void (*caller)(stringstream &), const stringstream &caller_args);
bool load_game(const string &filename);
void create_new_game();

#endif
