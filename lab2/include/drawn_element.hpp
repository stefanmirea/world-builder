/* Stefan-Gabriel Mirea - 331CC */

/* Declararea clasei DrawnElement, pentru retinerea obiectelor afisate la un momentdat. */
#ifndef DRAWN_ELEMENT_HPP_
#define DRAWN_ELEMENT_HPP_

#include <cell.hpp>

struct DrawnElement {
    Cell cell;      /* linia si coloana din sistemul implicit (nu din matrice) */
    int h;          /* inaltimea */
    int sel_type;   /* daca elementul este unul de selectie si daca da ce fel de selectie
                     * (SEL_NONE / SEL_SIMPLE / SEL_MULTIPLE) */
    int pos;        /* pozitia in objects3D la care incep obiectele din cadrul lui */

    DrawnElement(const Cell &cell, const int h, const int sel_type, const int pos);
};

#endif