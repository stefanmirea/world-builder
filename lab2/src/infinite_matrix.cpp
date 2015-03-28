/* Stefan-Gabriel Mirea - 331CC */

/* Implementarea clasei InfiniteMatrix. */
#include <infinite_matrix.hpp>

template<class T>
InfiniteMatrix<T>::InfiniteMatrix() : delta(Cell(0, 0)) {}

template<class T>
InfiniteMatrix<T>::InfiniteMatrix(const int height, const int width, const Cell &delta)
    : delta(delta)
{
    matrix.resize(height, vector<T>(width));
}

template<class T>
int InfiniteMatrix<T>::get_height() const
{
    return matrix.size();
}

template<class T>
int InfiniteMatrix<T>::get_width() const
{
    if (!get_height())
        return 0;
    return matrix[0].size();
}

/* spune daca celula data se afla in interiorul matricei */
template<class T>
bool InfiniteMatrix<T>::in_matrix(const Cell &cell) const
{
    return cell.row >= delta.row && cell.row < delta.row + get_height() &&
           cell.col >= delta.col && cell.col < delta.col + get_width();
}

/* intoarce elementul aflat intr-o celula data, simuland existenta unor elemente implicite in afara
 * matricei */
template<class T>
T InfiniteMatrix<T>::query(const Cell &cell) const
{
    if (!in_matrix(cell))
        return T();
    return matrix[cell.row - delta.row][cell.col - delta.col];
}

/* actualizeaza elementul dintr-o anumita celula, realizand si eventualele extinderi ale matricei */
template<class T>
void InfiniteMatrix<T>::update(const Cell &cell, const T &new_elem)
{
    if (!in_matrix(cell))
    {
        if (cell.row < delta.row)
        {
            vector<vector<T>> new_rows(delta.row - cell.row);
            for (int i = cell.row; i < delta.row; ++i)
                new_rows[i - cell.row].resize(get_width());
            matrix.insert(matrix.begin(), new_rows.begin(), new_rows.end());
            delta.row = cell.row;
        }
        else if (cell.row >= delta.row + get_height())
        {
            int initial_height = get_height();
            int width = get_width();
            matrix.resize(cell.row - delta.row + 1, vector<T>(width));
        }
        if (cell.col < delta.col)
        {
            vector<T> new_cells(delta.col - cell.col);
            int height = get_height();
            for (int i = 0; i < height; ++i)
                matrix[i].insert(matrix[i].begin(), new_cells.begin(), new_cells.end());
            delta.col = cell.col;
        }
        else if (cell.col >= delta.col + get_width())
        {
            int height = get_height();
            for (int i = 0; i < height; ++i)
                matrix[i].resize(cell.col - delta.col + 1);
        }
    }
    matrix[cell.row - delta.row][cell.col - delta.col] = new_elem;
}

/* citeste matricea din fisier */
template<class T>
void InfiniteMatrix<T>::read(ifstream &file)
{
    clear();
    int height, width;
    file.read(reinterpret_cast<char *>(&height), sizeof(height));
    file.read(reinterpret_cast<char *>(&width), sizeof(width));

    matrix.resize(height);
    for (int i = 0; i < height; ++i)
    {
        matrix[i].resize(width);
        file.read(reinterpret_cast<char *>(&matrix[i][0]), width * sizeof(T));
    }
}

/* scrie matricea in fisier */
template<class T>
void InfiniteMatrix<T>::write(ofstream &file)
{
    int height = get_height();
    int width = get_width();
    file.write(reinterpret_cast<const char *>(&height), sizeof(height));
    file.write(reinterpret_cast<const char *>(&width), sizeof(width));

    for (int i = 0; i < height; ++i)
        file.write(reinterpret_cast<const char *>(&matrix[i][0]), width * sizeof(T));
}

template<class T>
void InfiniteMatrix<T>::clear()
{
    matrix.clear();
    delta = Cell();
}

template class InfiniteMatrix<Element>;
template class InfiniteMatrix<bool>;
