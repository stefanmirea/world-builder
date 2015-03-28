/* Stefan-Gabriel Mirea - 331CC */

/* Declararea clasei InfiniteMatrix, pentru lucrul cu matrice ce se pot extinde in orice directie /
 * sens. */
#ifndef INFINITE_MATRIX_HPP_
#define INFINITE_MATRIX_HPP_

#include <cell.hpp>
#include <element.hpp>

#include <vector>
#include <fstream>

using namespace std;

template<class T>
class InfiniteMatrix {
private:
    vector<vector<T>> matrix;
public:
    /* offsetul matricei de elemente modificate, relativ la sistemul implicit */
    Cell delta;

    InfiniteMatrix();
    InfiniteMatrix(const int height, const int width, const Cell &delta);
    int get_height() const;
    int get_width() const;
    bool in_matrix(const Cell &cell) const;
    T query(const Cell &cell) const;
    void update(const Cell &cell, const T &new_elem);
    void read(ifstream &file);
    void write(ofstream &file);
    void clear();
};

#endif
