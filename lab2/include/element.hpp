/* Stefan-Gabriel Mirea - 331CC */

/* Declararea clasei Element, pentru retinerea elementelor modificate. */
#ifndef ELEMENT_HPP_
#define ELEMENT_HPP_

/* tipuri de pamant */
#define GR_EMPTY             0
#define GR_WATER             1
#define GR_GRASS             2

/* tipuri de elemente ale lumii (care se pun peste pamant) */
#define EL_EMPTY             0
#define EL_SQUARE            1
#define EL_HOUSE             2
#define EL_ROAD              3

struct Element {
    unsigned char terrain; /* bitii 0 si 1 spun daca e gol, apa sau iarba; bitii 2 - 5 specifica
                            * inclinarea */
    char object;           /* daca e gol, piata, casa sau drum */
    int house_floors;      /* numarul de etaje ale casei, daca obiectul e o casa */
    int altitude;          /* inaltimea reliefului */
    unsigned char flags;   /* bitul 0 spune daca elementul e aproape de o piata */
                           /* bitul 1 spune daca elementul a fost vizitat (folosit la parcurgeri) */
                           /* bitul 2 spune daca elementul (in cazul in care e o casa) e unit de o
                            * piata prin drumuri */

    Element(const unsigned char terrain = 0x3C + GR_EMPTY, const char object = EL_EMPTY,
            const int house_floors = 0, const int altitude = 0);
    bool close_to_square() const;
    void set_close_to_square(const bool value);
    bool visited() const;
    void set_visited(const bool value);
    bool connected() const;
    void set_connected(const bool value);
};

#endif
