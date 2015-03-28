/* Stefan-Gabriel Mirea - 331CC */

/* Variabile globale folosite in mai multe surse - am folosit multe, pentru ca trebuiau completate
 * doar anumite metode din clasa DrawingWindow si nu era recomandata modificarea framework-ului. */
#include <utils.hpp>

/* vectorul de comenzi din linia de comanda (sunt retinute doar 6 randuri + comanda curenta) */
vector<string> commands;

/* latimea portii spatiului de joc */
int game_width = 1024;

/* spune daca s-a apasat o tasta care a declansat apelul DrawingWindow::specialFunction */
bool special_key_pressed = false;

/* spune daca s-au facut orice fel de modificari ale lumii */
bool world_modified;

/* modul de editare (TERRAIN / SQUARE / HOUSES / ROADS) */
int edit_mode;

/* matrice care retine elementele modificate */
InfiniteMatrix<Element> modified;

/* elementele afisate la un momentdat */
vector<DrawnElement> drawn_elements;

/* coordonatele selectiei simple */
Cell sel_begin, sel_end;

/* spune daca selectia e multipla sau simpla */
bool multiple_selection;

/* sagetile apasate la un momentdat */
bool pressed_left, pressed_right, pressed_up, pressed_down;

/* directia de deplasare */
int dir = DIR_NONE;

/* punctul din centrul elementului selectat, care va fi urmarit de camera */
Point2D sel_center;

/* dimensiunea laturii cuburilor */
float edge;

/* true daca la scroll nu se vor actualiza fragmentele afisate, false in caz contrar */
bool fragment;

/* vector de perechi (pozitie, numar_etaje) ce retine casele care s-au decupat / copiat */
vector<pair<Cell, int>> clipboard;

/* modul de interpretare al comenzilor */
int console_mode;

/* functia care tocmai se executa inainte de lansarea dialogului curent si argumentele ei */
void (*last_command)(stringstream &);
stringstream last_args;
