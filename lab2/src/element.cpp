/* Stefan-Gabriel Mirea - 331CC */

/* Implementarea clasei Element. */
#include <element.hpp>

Element::Element(const unsigned char terrain, const char object, const int house_floors,
                 const int altitude)
    : terrain(terrain),
      object(object),
      house_floors(house_floors),
      altitude(altitude),
      flags(0) {}

/* spune daca elementul se afla in zona de acoperire a unei piete */
bool Element::close_to_square() const
{
    return flags & 1;
}

/* marcheaza ca elementul se afla in zona de acoperire a unei piete */
void Element::set_close_to_square(const bool value)
{
    if (value)
        flags |= 1;
    else
        flags &= 0xFE;
}

/* spune daca elementul a fost vizitat in cadrul unei parcurgeri */
bool Element::visited() const
{
    return (flags & 2) != 0;
}

/* marcheaza ca elementul a fost vizitat in cadrul unei parcurgeri */
void Element::set_visited(const bool value)
{
    if (value)
        flags |= 2;
    else
        flags &= 0xFD;
}

/* spune daca elementul e conectat la o piata (direct sau printr-un drum) */
bool Element::connected() const
{
    return (flags & 4) != 0;
}

/* marcheaza ca elementul e conectat la o piata (direct sau printr-un drum) */
void Element::set_connected(const bool value)
{
    if (value)
        flags |= 4;
    else
        flags &= 0xFB;
}
