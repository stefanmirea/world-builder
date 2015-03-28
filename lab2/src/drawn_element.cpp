/* Stefan-Gabriel Mirea - 331CC */

/* Implementarea clasei DrawnElement. */
#include <drawn_element.hpp>

DrawnElement::DrawnElement(const Cell &cell, const int h, const int sel_type, const int pos)
    : cell(cell),
      h(h),
      sel_type(sel_type),
      pos(pos) {}