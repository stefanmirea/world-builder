/* Stefan-Gabriel Mirea - 331CC */

/* Functiile pentru desenarea diferitelor tipuri de elemente din matrice. */
#include <utils.hpp>

/* variabile definite in global_vars.cpp */
extern Point2D sel_center;
extern int edit_mode;
extern float edge;
extern InfiniteMatrix<Element> modified;
extern vector<DrawnElement> drawn_elements;

/* primeste ca parametru un vector de puncte si o lista de pozitii si intoarce subsirul
 * corespunzator */
static vector<Point3D *> select_vert(const vector<Point3D *> vertices, ...)
{
    va_list ap;
    va_start(ap, vertices);
    vector<Point3D *> ret;

    int index = va_arg(ap, int);
    while (index > -1)
    {
        ret.push_back(vertices[index]);
        index = va_arg(ap, int);
    }
    va_end(ap);
    return ret;
}

/* creeaza o fata din punctele date prin indici si actualizeaza si lista de luminozitati */
static Face *make_face(vector<char> &brightness_list, const char current_brightness, ...)
{
    va_list ap;
    va_start(ap, current_brightness);
    vector<int> contour;

    brightness_list.push_back(current_brightness);
    int elem = va_arg(ap, int);
    while (elem > -1)
    {
        contour.push_back(elem);
        elem = va_arg(ap, int);
    }
    va_end(ap);
    return new Face(contour);
}

/* deseneaza un etaj sau acoperisul unei case */
static vector<Object3D *> draw_floor(const Cell &cell, const int floor, const float vertical_offset)
{
    Element el = modified.query(cell);
    vector<Object3D *> to_be_drawn;
    vector<Point3D *> vertices;
    vector<Face *> faces;
    vector<int> contour;
    vector<char> brightness; /* luminozitatile fetelor */

    Transform3D::loadIdentityModelMatrix();
    Transform3D::translateMatrix(cell.col * edge, vertical_offset, cell.row * edge);

    Element el_up         = modified.query(Cell(cell.row - 1, cell.col    ));
    Element el_down       = modified.query(Cell(cell.row + 1, cell.col    ));
    Element el_left       = modified.query(Cell(cell.row    , cell.col - 1));
    Element el_right      = modified.query(Cell(cell.row    , cell.col + 1));
    Element el_left_up    = modified.query(Cell(cell.row - 1, cell.col - 1));
    Element el_left_down  = modified.query(Cell(cell.row + 1, cell.col - 1));
    Element el_right_up   = modified.query(Cell(cell.row - 1, cell.col + 1));
    Element el_right_down = modified.query(Cell(cell.row + 1, cell.col + 1));

    bool house_up         = el_up.object == EL_HOUSE         && el_up.house_floors         + 1 >= floor;
    bool house_down       = el_down.object == EL_HOUSE       && el_down.house_floors       + 1 >= floor;
    bool house_left       = el_left.object == EL_HOUSE       && el_left.house_floors       + 1 >= floor;
    bool house_right      = el_right.object == EL_HOUSE      && el_right.house_floors      + 1 >= floor;
    bool house_left_up    = el_left_up.object == EL_HOUSE    && el_left_up.house_floors    + 1 >= floor;
    bool house_left_down  = el_left_down.object == EL_HOUSE  && el_left_down.house_floors  + 1 >= floor;
    bool house_right_up   = el_right_up.object == EL_HOUSE   && el_right_up.house_floors   + 1 >= floor;
    bool house_right_down = el_right_down.object == EL_HOUSE && el_right_down.house_floors + 1 >= floor;

    if (floor == el.house_floors + 1)
    {
        /* desenez acoperisul (multe valori sunt hardcodate pentru performanta - se deseneaza doar
         * fetele care se vad - si pentru ca framework-ul nu are suport pentru luminozitate si
         * pentru intersectia intre obiecte distincte) */
        vector<Point3D *> all_vert{
            new Point3D(       0,        0,        0),
            new Point3D(       0,        0,     edge),
            new Point3D(    edge,        0,     edge),
            new Point3D(    edge,        0,        0),
            new Point3D(       0, edge / 2,        0),
            new Point3D(       0, edge / 2,     edge),
            new Point3D(    edge, edge / 2,     edge),
            new Point3D(    edge, edge / 2,        0),
            new Point3D(       0, edge / 2, edge / 2),
            new Point3D(edge / 2, edge / 2,     edge),
            new Point3D(    edge, edge / 2, edge / 2),
            new Point3D(edge / 2, edge / 2,        0),
            new Point3D(edge / 2, edge / 2, edge / 2)};

        if (house_up && house_left && house_down && house_right && house_left_up && house_left_down && house_right_up && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  5,  6,  7, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  5,  6, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  5,  4, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  6,  5, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_down && house_right_up && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  5,  6,  7,  8, 11, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  4,  3,  6,  8,  7,  5, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  5,  4, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_up && house_right_up && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  6,  7,  8,  9, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  5,  3,  6,  8,  7,  4, -1));
            faces.push_back(make_face(brightness, BRIGHT,  0,  6,  8, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4,  7, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  5,  4, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_up && house_left_down && house_right_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  5,  7,  9, 10, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  6,  8,  7,  5, -1));
            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  6,  4, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  5,  7, -1));
            faces.push_back(make_face(brightness, DARK,    1,  6,  8, -1));
            faces.push_back(make_face(brightness, BRIGHT,  1,  7,  8, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_up && house_left_down && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  5,  6, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  4,  3,  7,  8,  6,  5, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  5,  4, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  6,  5, -1));
            faces.push_back(make_face(brightness, DARK,    2,  7,  8, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_up && house_left_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  5,  9, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  5,  7, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  5,  4, -1));
            faces.push_back(make_face(brightness, DARK,    1,  5,  8, -1));
            faces.push_back(make_face(brightness, BRIGHT,  1,  8,  6, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  6, -1));
            faces.push_back(make_face(brightness, DARK,    2,  7,  8, -1));
        }
        else if (house_up && house_left && house_down && house_left_up && house_left_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  5,  9, 11, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  5,  6, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  5,  4, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  6,  5, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_up && house_right_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  7,  8,  9, 10, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  5,  7,  4, -1));
            faces.push_back(make_face(brightness, BRIGHT,  0,  5,  8, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  6, -1));
            faces.push_back(make_face(brightness, DARK,    1,  6,  8, -1));
            faces.push_back(make_face(brightness, BRIGHT,  1,  7,  8, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4,  7, -1));
        }
        else if (house_up && house_left && house_right && house_left_up && house_right_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  7,  8, 10, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  6,  5, -1));
            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  6,  5, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4,  6, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_right_up && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  6,  7,  8,  9, 11, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  7,  6, -1));
            faces.push_back(make_face(brightness, BRIGHT,  0,  5,  8, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  3,  6, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4,  3, -1));
        }
        else if (house_up && house_down && house_right && house_right_up && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  6,  7,  9, 11, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  6,  5, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  3,  5, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4,  3, -1));
        }
        else if (house_left && house_down && house_right && house_left_down && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  5,  6,  8, 10, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  6,  5, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  6,  4, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_down && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  5,  6,  8, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  4,  6,  5, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  6,  4, -1));
            faces.push_back(make_face(brightness, DARK,    2,  7,  8, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_down && house_right_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  5,  7,  8,  9, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  5,  9,  6, -1));
            faces.push_back(make_face(brightness, NORMAL,  4,  7,  9,  8, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  6,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  6,  9, -1));
            faces.push_back(make_face(brightness, BRIGHT,  1,  7,  9, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4,  7, -1));
        }
        else if (house_up && house_left && house_down && house_right && house_left_up && house_right_down)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  4,  6,  8,  9, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, NORMAL,  3,  5,  9,  8, -1));
            faces.push_back(make_face(brightness, NORMAL,  4,  7,  9,  6, -1));
            faces.push_back(make_face(brightness, BRIGHT,  0,  5,  9, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4,  6, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  7,  4, -1));
            faces.push_back(make_face(brightness, DARK,    2,  8,  9, -1));
        }
        else if (house_left_up && house_up && house_left)
            if (house_right && house_down)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  4,  8,  9, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  4,  8,  7, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  4,  8, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  5, -1));
                faces.push_back(make_face(brightness, DARK,    1,  5,  8, -1));
                faces.push_back(make_face(brightness, BRIGHT,  1,  8,  6, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  6, -1));
                faces.push_back(make_face(brightness, DARK,    2,  7,  8, -1));
            }
            else if (house_right)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  4,  8, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  4,  7,  6, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  5,  4, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  5, -1));
                faces.push_back(make_face(brightness, DARK,    2,  6,  7, -1));
            }
            else if (house_down)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  4,  8,  9, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  4,  7,  6, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  4,  7, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  5, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  6,  5, -1));
            }
            else
            {
                vertices = select_vert(all_vert,  1,  2,  3,  4,  8, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  4,  6,  5, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  6,  4, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  5,  6, -1));
            }
        else if (house_up && house_right && house_right_up)
            if (house_left && house_down)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  7,  8,  9, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  7,  8,  6, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  4,  8, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  5, -1));
                faces.push_back(make_face(brightness, DARK,    1,  5,  8, -1));
                faces.push_back(make_face(brightness, BRIGHT,  1,  6,  8, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  3,  6, -1));
            }
            else if (house_down)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  7,  9, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  6,  7,  5, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  4, -1));
                faces.push_back(make_face(brightness, DARK,    1,  4,  7, -1));
                faces.push_back(make_face(brightness, BRIGHT,  1,  5,  7, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  3,  5, -1));
            }
            else if (house_left)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  7,  8, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  6,  7,  5, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  5,  4, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  3,  5, -1));
            }
            else
            {
                vertices = select_vert(all_vert,  1,  2,  3,  7, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  5,  6,  4, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  4,  6, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  3,  4, -1));
            }
        else if (house_right && house_down && house_right_down)
            if (house_up && house_left)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  6,  8,  9, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  5,  8,  6, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  4,  8, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  3,  5, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  6,  3, -1));
                faces.push_back(make_face(brightness, DARK,    2,  7,  8, -1));
            }
            else if (house_up)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  6,  9, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  5,  7,  4, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  3,  4, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  5,  3, -1));
                faces.push_back(make_face(brightness, DARK,    2,  6,  7, -1));
            }
            else if (house_left)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  6,  8,  9, 10, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  5,  7,  6, -1));
                faces.push_back(make_face(brightness, BRIGHT,  0,  4,  7, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  3,  5, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  6,  3, -1));
            }
            else
            {
                vertices = select_vert(all_vert,  1,  2,  3,  6,  9, 10, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  4,  6,  5, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  3,  4, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  5,  3, -1));
            }
        else if (house_left && house_down && house_left_down)
            if (house_right && house_up)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  5,  8,  9, 10, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  5,  8,  4, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  5,  3, -1));
                faces.push_back(make_face(brightness, DARK,    1,  5,  8, -1));
                faces.push_back(make_face(brightness, BRIGHT,  1,  6,  8, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  6, -1));
                faces.push_back(make_face(brightness, DARK,    2,  7,  8, -1));
            }
            else if (house_right)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  5,  8,  9, 10, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  4,  7,  5, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  5,  3, -1));
                faces.push_back(make_face(brightness, DARK,    1,  5,  7, -1));
                faces.push_back(make_face(brightness, BRIGHT,  1,  6,  7, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  6, -1));
            }
            else if (house_up)
            {
                vertices = select_vert(all_vert,  1,  2,  3,  5,  8,  9, 11, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  4,  7,  5, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  5,  3, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  6,  5, -1));
            }
            else
            {
                vertices = select_vert(all_vert,  1,  2,  3,  5,  8,  9, 12, -1);

                faces.push_back(make_face(brightness, NORMAL,  3,  4,  6,  5, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  5,  3, -1));
                faces.push_back(make_face(brightness, DARK,    1,  2,  6,  5, -1));
            }
        else if (!house_left && !house_down && !house_right && !house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  3, -1));
        }
        else if (!house_left && !house_down && !house_right && house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3, 11, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  4, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  3,  4, -1));
        }
        else if (!house_left && !house_down && house_right && !house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3, 10, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  4, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  3, -1));
        }
        else if (!house_left && !house_down && house_right && house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  5, -1));
            faces.push_back(make_face(brightness, DARK,    2,  4,  5, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  3, -1));
        }
        else if (!house_left && house_down && !house_right && !house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  9, 12, -1);

            faces.push_back(make_face(brightness, DARK,  1,  2,  4,  3, -1));
            faces.push_back(make_face(brightness, DARK,  0,  1,  3, -1));
        }
        else if (!house_left && house_down && !house_right && house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  9, 11, -1);

            faces.push_back(make_face(brightness, DARK,  1,  2,  4,  3, -1));
            faces.push_back(make_face(brightness, DARK,  0,  1,  3, -1));
        }
        else if (!house_left && house_down && house_right && !house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  9, 10, 12, -1);

            faces.push_back(make_face(brightness, DARK,    1,  3,  5, -1));
            faces.push_back(make_face(brightness, BRIGHT,  1,  4,  5, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4, -1));
        }
        else if (!house_left && house_down && house_right && house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  9, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, DARK,    1,  3,  6, -1));
            faces.push_back(make_face(brightness, DARK,    2,  5,  6, -1));
            faces.push_back(make_face(brightness, BRIGHT,  1,  4,  6, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4, -1));
        }
        else if (house_left && !house_down && !house_right && !house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  8, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  4,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  4,  2, -1));
        }
        else if (house_left && !house_down && !house_right && house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  8, 11, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  5,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4,  5, -1));
        }
        else if (house_left && !house_down && house_right && !house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  8, 10, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  4,  3, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4, -1));
        }
        else if (house_left && !house_down && house_right && house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  8, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  4,  3, -1));
            faces.push_back(make_face(brightness, DARK,    2,  5,  6, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  4, -1));
        }
        else if (house_left && house_down && !house_right && !house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  8,  9, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  3,  5, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  5,  4, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4, -1));
        }
        else if (house_left && house_down && !house_right && house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  8,  9, 11, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  3,  6, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  5,  4, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4, -1));
        }
        else if (house_left && house_down && house_right && !house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  8,  9, 10, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  3,  6, -1));
            faces.push_back(make_face(brightness, BRIGHT,  1,  5,  6, -1));
            faces.push_back(make_face(brightness, DARK,    1,  4,  6, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  5, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4, -1));
        }
        else if (house_left && house_down && house_right && house_up)
        {
            vertices = select_vert(all_vert,  1,  2,  3,  8,  9, 10, 11, 12, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  3,  7, -1));
            faces.push_back(make_face(brightness, BRIGHT,  1,  5,  7, -1));
            faces.push_back(make_face(brightness, DARK,    1,  4,  7, -1));
            faces.push_back(make_face(brightness, DARK,    2,  6,  7, -1));
            faces.push_back(make_face(brightness, DARK,    0,  1,  4, -1));
            faces.push_back(make_face(brightness, DARK,    1,  2,  5, -1));
        }
    }
    else if (floor >= 0 && floor < el.house_floors + 1)
    {
        /* desenez un etaj */
        float eighth = edge / 8.0f;

        vector<Point3D *> all_vert{
            new Point3D(            0,        0, edge - eighth),
            new Point3D(            0, edge / 2, edge - eighth),
            new Point3D(       eighth,        0, edge - eighth),
            new Point3D(       eighth, edge / 2, edge - eighth),
            new Point3D(edge - eighth,        0,          edge),
            new Point3D(edge - eighth, edge / 2,          edge),
            new Point3D(edge - eighth,        0, edge - eighth),
            new Point3D(edge - eighth, edge / 2, edge - eighth),
            new Point3D(         edge,        0, edge - eighth),
            new Point3D(         edge, edge / 2, edge - eighth),
            new Point3D(edge - eighth,        0,        eighth),
            new Point3D(edge - eighth, edge / 2,        eighth),
            new Point3D(edge - eighth,        0,             0),
            new Point3D(edge - eighth, edge / 2,             0),
            new Point3D(       eighth,        0,          edge),
            new Point3D(       eighth, edge / 2,          edge),
            new Point3D(         edge,        0,        eighth),
            new Point3D(         edge, edge / 2,        eighth),
            new Point3D(         edge,        0,          edge),
            new Point3D(         edge, edge / 2,          edge)};

        if (house_down && house_right)
            if (!house_right_down)
            {
                vertices = select_vert(all_vert, 14, 15,  4,  5,  6,  7,  8,  9, 16, 17, -1);

                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
                faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
                faces.push_back(make_face(brightness, BRIGHT,  4,  5,  7,  6, -1));
                faces.push_back(make_face(brightness, DARK,    6,  7,  9,  8, -1));
            }
            else
            {
                vertices = select_vert(all_vert, 14, 15, 18, 19, 16, 17, -1);

                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
                faces.push_back(make_face(brightness, DARK,    2,  4,  5,  3, -1));
            }
        else if (!house_left && !house_down && house_right)
        {
            vertices = select_vert(all_vert,  2,  3,  8,  9, 16, 17, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
            faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
        }
        else if (house_left && !house_down && house_right)
        {
            vertices = select_vert(all_vert,  0,  1,  8,  9, 16, 17, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
            faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
        }
        else if (house_down && !house_right && !house_up)
        {
            vertices = select_vert(all_vert, 14, 15,  4,  5, 10, 11, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
            faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
        }
        else if (house_down && !house_right && house_up)
        {
            vertices = select_vert(all_vert, 14, 15,  4,  5, 12, 13, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
            faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
        }
        else if (!house_left && !house_down && !house_right && !house_up)
        {
            vertices = select_vert(all_vert,  2,  3,  6,  7, 10, 11, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
            faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
        }
        else if (!house_left && !house_down && !house_right && house_up)
        {
            vertices = select_vert(all_vert,  2,  3,  6,  7, 12, 13, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
            faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
        }
        else if (house_left && !house_down && !house_right && !house_up)
        {
            vertices = select_vert(all_vert,  0,  1,  6,  7, 10, 11, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
            faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
        }
        else if (house_left && !house_down && !house_right && house_up)
        {
            vertices = select_vert(all_vert,  0,  1,  6,  7, 12, 13, -1);

            faces.push_back(make_face(brightness, BRIGHT,  0,  1,  3,  2, -1));
            faces.push_back(make_face(brightness, DARK,    2,  3,  5,  4, -1));
        }

        /* daca elementul de deasupra e mai mic (nu a avut vecini cu care sa fuzioneze), construiesc
         * un acoperis secundar */
        bool roof_left  = house_left  && el_left.house_floors  < floor;
        bool roof_right = house_right && el_right.house_floors < floor;
        bool roof_up    = house_up    && el_up.house_floors    < floor;
        bool roof_down  = house_down  && el_down.house_floors  < floor;
        if (roof_left || roof_right || roof_up || roof_down)
        {
            float x1, x2, z1, z2;

            if (house_left && house_left_up && house_up && (el_left.house_floors < floor ||
                el_left_up.house_floors < floor || el_up.house_floors < floor))
                roof_left = roof_up = true;
            if (house_up && house_right_up && house_right && (el_up.house_floors < floor ||
                el_right_up.house_floors < floor || el_right.house_floors < floor))
                roof_up = roof_right = true;
            if (house_right && house_right_down && house_down && (el_right.house_floors < floor ||
                el_right_down.house_floors < floor || el_down.house_floors < floor))
                roof_right = roof_down = true;
            if (house_down && house_left_down && house_left && (el_down.house_floors < floor ||
                el_left_down.house_floors < floor || el_left.house_floors < floor))
                roof_down = roof_left = true;

            x1 = roof_left  ?    0 : eighth;
            x2 = roof_right ? edge : edge - eighth;
            z1 = roof_up    ?    0 : eighth;
            z2 = roof_down  ? edge : edge - eighth;

            vector<Point3D *> sec_roof_vert{
                new Point3D(x1, edge / 2, z1),
                new Point3D(x1, edge / 2, z2),
                new Point3D(x2, edge / 2, z2),
                new Point3D(x2, edge / 2, z1)};

            vector<int> sec_roof_contour{0, 1, 2, 3};

            vector<Face *> sec_roof_faces{new Face(sec_roof_contour)};

            Object3D *sec_roof_border = new Object3D(sec_roof_vert, sec_roof_faces, Color(0, 0, 0),
                                                     false);
            Transform3D::applyTransform(sec_roof_border);
            to_be_drawn.push_back(sec_roof_border);
            Object3D *sec_roof = new Object3D(sec_roof_vert, sec_roof_faces,
                                              Color(0.82f, 0.18f, 0.18f), true);
            Transform3D::applyTransform(sec_roof);
            to_be_drawn.push_back(sec_roof);
        }
    }
    Object3D *border = new Object3D(vertices, faces, Color(0, 0, 0), false);
    Transform3D::applyTransform(border);
    to_be_drawn.push_back(border);
    for (int i = 0; i < static_cast<int>(faces.size()); ++i)
    {
        vector<Face *> current_face(faces.begin() + i, faces.begin() + i + 1);
        float r, g, b;
        if (floor == el.house_floors + 1)
        {
            r = 0.82f, g = 0.18f, b = 0.18f;
            float inc = brightness[i] == BRIGHT ? 0.05f : brightness[i] == DARK ? -0.05f : 0;
            r += inc; if (r > 1) r = 1; else if (r < 0) r = 0;
            g += inc; if (g > 1) g = 1; else if (g < 0) g = 0;
            b += inc; if (b > 1) b = 1; else if (b < 0) b = 0;
        }
        else
            r = g = b = brightness[i] == BRIGHT ? 1 : 0.74f;
        Object3D *wall = new Object3D(vertices, current_face, Color(r, g, b), true);
        Transform3D::applyTransform(wall);
        to_be_drawn.push_back(wall);
    }
    return to_be_drawn;
}

/* deseneaza un element situat in pozitia data */
void draw_element(const Cell &cell, const int h, const int sel_type)
{
    /* vector cu obiectele care vor fi inserate */
    Element el = modified.query(cell);
    vector<Object3D *> to_be_drawn;
    if (sel_type != SEL_NONE || (((el.terrain & 3) == GR_EMPTY || (el.terrain & 3) == GR_GRASS) &&
        el.altitude == h))
    {
        /* variabile pentru crearea pamantului */
        vector<Point3D *> ground_vert{
            new Point3D(   0,    0,    0),
            new Point3D(edge,    0,    0),
            new Point3D(edge,    0, edge),
            new Point3D(   0,    0, edge),
            new Point3D(   0, edge,    0),
            new Point3D(edge, edge,    0),
            new Point3D(edge, edge, edge),
            new Point3D(   0, edge, edge)};
        vector<Point3D *> vertices;
        vector<Face *> faces;
        vector<char> brightness; /* luminozitatile fetelor */

        /* variabile pentru crearea drumului, daca exista */
        vector<Point3D *> road_vert;
        vector<Face *> road_tiles;
        vector<char> road_brightness;

        /* daca trebuie desenat un drum, verific daca acesta trebuie legat de alt drum alaturat */
        bool road_up, road_down, road_left, road_right;
        if (el.object == EL_ROAD || el.object == EL_HOUSE)
        {
            int object_up    = modified.query(Cell(cell.row - 1, cell.col)).object;
            int object_down  = modified.query(Cell(cell.row + 1, cell.col)).object;
            int object_left  = modified.query(Cell(cell.row, cell.col - 1)).object;
            int object_right = modified.query(Cell(cell.row, cell.col + 1)).object;

            road_up    = object_up    == EL_ROAD || object_up    == EL_SQUARE;
            road_down  = object_down  == EL_ROAD || object_down  == EL_SQUARE;
            road_left  = object_left  == EL_ROAD || object_left  == EL_SQUARE;
            road_right = object_right == EL_ROAD || object_right == EL_SQUARE;

            if (el.object == EL_ROAD)
            {
                road_up    = road_up    || object_up    == EL_HOUSE;
                road_down  = road_down  || object_down  == EL_HOUSE;
                road_left  = road_left  || object_left  == EL_HOUSE;
                road_right = road_right || object_right == EL_HOUSE;
            }
        }

        int tilt = (el.terrain & 0x3C) >> 2;

        if (sel_type == SEL_SIMPLE)
        {
            sel_center.x = (cell.col - cell.row) * edge * sqrt(2.0f) / 2;
            sel_center.y = EDGE_PRINTED * (-1.0f * (cell.col + cell.row) / 2 + h);
            if (tilt == 15)
                sel_center.y += EDGE_PRINTED * 0.5f;
        }

        switch (tilt)
        {
            case 1:
                vertices = select_vert(ground_vert,  1,  2,  3,  4, -1);

                faces.push_back(make_face(brightness, BRIGHT,  1,  2,  3, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  3, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(12);
                    road_vert[0]  = new Point3D(    edge / 3, 2 * edge / 3,            0);
                    road_vert[1]  = new Point3D(2 * edge / 3,     edge / 3,            0);
                    road_vert[2]  = new Point3D(           0, 2 * edge / 3,     edge / 3);
                    road_vert[3]  = new Point3D(    edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[4]  = new Point3D(2 * edge / 3,     edge / 3,     edge / 3);
                    road_vert[5]  = new Point3D(        edge,            0,     edge / 3);
                    road_vert[6]  = new Point3D(           0,     edge / 3, 2 * edge / 3);
                    road_vert[7]  = new Point3D(    edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[8]  = new Point3D(2 * edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[9]  = new Point3D(        edge,            0, 2 * edge / 3);
                    road_vert[10] = new Point3D(    edge / 3,            0,         edge);
                    road_vert[11] = new Point3D(2 * edge / 3,            0,         edge);

                    road_tiles.push_back(make_face(road_brightness, BRIGHT,  3,  7,  8, -1));
                    road_tiles.push_back(make_face(road_brightness, DARK,    3,  4,  8, -1));
                    if (road_up)
                        road_tiles.push_back(make_face(road_brightness, DARK,    0,  1,  4,  3, -1));
                    if (road_down)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  7,  8, 11, 10, -1));
                    if (road_left)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  2,  3,  7,  6, -1));
                    if (road_right)
                        road_tiles.push_back(make_face(road_brightness, DARK,    4,  5,  9,  8, -1));
                }
                break;
            case 2:
                vertices = select_vert(ground_vert,  2,  3,  5, -1);

                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  2, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(7);
                    road_vert[0] = new Point3D(2 * edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[1] = new Point3D(        edge, 2 * edge / 3,     edge / 3);
                    road_vert[2] = new Point3D(    edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[3] = new Point3D(2 * edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[4] = new Point3D(        edge,     edge / 3, 2 * edge / 3);
                    road_vert[5] = new Point3D(    edge / 3,            0,         edge);
                    road_vert[6] = new Point3D(2 * edge / 3,            0,         edge);

                    road_tiles.push_back(make_face(road_brightness, BRIGHT,  0,  2,  3, -1));
                    if (road_down)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  2,  3,  6,  5, -1));
                    if (road_right)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  0,  1,  4,  3, -1));
                }
                break;
            case 3:
                vertices = select_vert(ground_vert,  2,  3,  4,  5, -1);

                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  2,  3, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(12);
                    road_vert[0]  = new Point3D(    edge / 3,         edge,            0);
                    road_vert[1]  = new Point3D(2 * edge / 3,         edge,            0);
                    road_vert[2]  = new Point3D(           0, 2 * edge / 3,     edge / 3);
                    road_vert[3]  = new Point3D(    edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[4]  = new Point3D(2 * edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[5]  = new Point3D(        edge, 2 * edge / 3,     edge / 3);
                    road_vert[6]  = new Point3D(           0,     edge / 3, 2 * edge / 3);
                    road_vert[7]  = new Point3D(    edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[8]  = new Point3D(2 * edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[9]  = new Point3D(        edge,     edge / 3, 2 * edge / 3);
                    road_vert[10] = new Point3D(    edge / 3,            0,         edge);
                    road_vert[11] = new Point3D(2 * edge / 3,            0,         edge);

                    road_tiles.push_back(make_face(road_brightness, BRIGHT,  3,  4,  8,  7, -1));
                    if (road_up)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  0,  1,  4,  3, -1));
                    if (road_down)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  7,  8, 11, 10, -1));
                    if (road_left)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  2,  3,  7,  6, -1));
                    if (road_right)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  4,  5,  9,  8, -1));
                }
                break;
            case 4:
                return;
            case 5:
                vertices = select_vert(ground_vert,  1,  3,  4,  6, -1);

                faces.push_back(make_face(brightness, BRIGHT,  1,  2,  3, -1));
                faces.push_back(make_face(brightness, DARK,    0,  2,  3, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(8);
                    road_vert[0] = new Point3D(    edge / 3, 2 * edge / 3,            0);
                    road_vert[1] = new Point3D(2 * edge / 3,     edge / 3,            0);
                    road_vert[2] = new Point3D(           0, 2 * edge / 3,     edge / 3);
                    road_vert[3] = new Point3D(    edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[4] = new Point3D(2 * edge / 3,     edge / 3,     edge / 3);
                    road_vert[5] = new Point3D(           0,     edge / 3, 2 * edge / 3);
                    road_vert[6] = new Point3D(    edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[7] = new Point3D(    edge / 2,     edge / 2,     edge / 2);

                    road_tiles.push_back(make_face(road_brightness, BRIGHT,  3,  6,  7, -1));
                    road_tiles.push_back(make_face(road_brightness, DARK,    3,  4,  7, -1));
                    if (road_up)
                        road_tiles.push_back(make_face(road_brightness, DARK,    0,  1,  4,  3, -1));
                    if (road_left)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  2,  3,  6,  5, -1));
                }
                break;
            case 6:
                return;
            case 7:
                vertices = select_vert(ground_vert,  3,  4,  5, -1);

                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  2, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(7);
                    road_vert[0] = new Point3D(    edge / 3,         edge,            0);
                    road_vert[1] = new Point3D(2 * edge / 3,         edge,            0);
                    road_vert[2] = new Point3D(           0, 2 * edge / 3,     edge / 3);
                    road_vert[3] = new Point3D(    edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[4] = new Point3D(2 * edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[5] = new Point3D(           0,     edge / 3, 2 * edge / 3);
                    road_vert[6] = new Point3D(    edge / 3,     edge / 3, 2 * edge / 3);

                    road_tiles.push_back(make_face(road_brightness, BRIGHT,  3,  4,  6, -1));
                    if (road_up)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  0,  1,  4,  3, -1));
                    if (road_left)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  2,  3,  6,  5, -1));
                }
                break;
            case 8:
                vertices = select_vert(ground_vert,  1,  2,  7, -1);

                faces.push_back(make_face(brightness, DARK,  0,  1,  2, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(7);
                    road_vert[0] = new Point3D(2 * edge / 3,     edge / 3,     edge / 3);
                    road_vert[1] = new Point3D(        edge,            0,     edge / 3);
                    road_vert[2] = new Point3D(    edge / 3, 2 * edge / 3, 2 * edge / 3);
                    road_vert[3] = new Point3D(2 * edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[4] = new Point3D(        edge,            0, 2 * edge / 3);
                    road_vert[5] = new Point3D(    edge / 3, 2 * edge / 3,         edge);
                    road_vert[6] = new Point3D(2 * edge / 3,     edge / 3,         edge);

                    road_tiles.push_back(make_face(road_brightness, DARK,  0,  2,  3, -1));
                    if (road_down)
                        road_tiles.push_back(make_face(road_brightness, DARK,  2,  3,  6,  5, -1));
                    if (road_right)
                        road_tiles.push_back(make_face(road_brightness, DARK,  0,  1,  4,  3, -1));
                }
                break;
            case 9:
                vertices = select_vert(ground_vert,  1,  2,  4,  7, -1);

                faces.push_back(make_face(brightness, DARK,  0,  1,  3,  2, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(12);
                    road_vert[0]  = new Point3D(    edge / 3, 2 * edge / 3,            0);
                    road_vert[1]  = new Point3D(2 * edge / 3,     edge / 3,            0);
                    road_vert[2]  = new Point3D(           0,         edge,     edge / 3);
                    road_vert[3]  = new Point3D(    edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[4]  = new Point3D(2 * edge / 3,     edge / 3,     edge / 3);
                    road_vert[5]  = new Point3D(        edge,            0,     edge / 3);
                    road_vert[6]  = new Point3D(           0,         edge, 2 * edge / 3);
                    road_vert[7]  = new Point3D(    edge / 3, 2 * edge / 3, 2 * edge / 3);
                    road_vert[8]  = new Point3D(2 * edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[9]  = new Point3D(        edge,            0, 2 * edge / 3);
                    road_vert[10] = new Point3D(    edge / 3, 2 * edge / 3,         edge);
                    road_vert[11] = new Point3D(2 * edge / 3,     edge / 3,         edge);

                    road_tiles.push_back(make_face(road_brightness, DARK,  3,  4,  8,  7, -1));
                    if (road_up)
                        road_tiles.push_back(make_face(road_brightness, DARK,  0,  1,  4,  3, -1));
                    if (road_down)
                        road_tiles.push_back(make_face(road_brightness, DARK,  7,  8, 11, 10, -1));
                    if (road_left)
                        road_tiles.push_back(make_face(road_brightness, DARK,  2,  3,  7,  6, -1));
                    if (road_right)
                        road_tiles.push_back(make_face(road_brightness, DARK,  4,  5,  9,  8, -1));
                }
                break;
            case 10:
                vertices = select_vert(ground_vert,  2,  5,  6,  7, -1);

                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  2, -1));
                faces.push_back(make_face(brightness, DARK,    0,  2,  3, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(8);
                    road_vert[0] = new Point3D(2 * edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[1] = new Point3D(        edge, 2 * edge / 3,     edge / 3);
                    road_vert[2] = new Point3D(    edge / 2,     edge / 2,     edge / 2);
                    road_vert[3] = new Point3D(    edge / 3, 2 * edge / 3, 2 * edge / 3);
                    road_vert[4] = new Point3D(2 * edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[5] = new Point3D(        edge,     edge / 3, 2 * edge / 3);
                    road_vert[6] = new Point3D(    edge / 3, 2 * edge / 3,         edge);
                    road_vert[7] = new Point3D(2 * edge / 3,     edge / 3,         edge);

                    road_tiles.push_back(make_face(road_brightness, DARK,    2,  3,  4, -1));
                    road_tiles.push_back(make_face(road_brightness, BRIGHT,  0,  2,  4, -1));
                    if (road_down)
                        road_tiles.push_back(make_face(road_brightness, DARK,    3,  4,  7,  6, -1));
                    if (road_right)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  0,  1,  5,  4, -1));
                }
                break;
            case 11:
                vertices = select_vert(ground_vert,  2,  4,  5,  7, -1);

                faces.push_back(make_face(brightness, BRIGHT,  0,  1,  2, -1));
                faces.push_back(make_face(brightness, DARK,    0,  1,  3, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(12);
                    road_vert[0]  = new Point3D(    edge / 3,         edge,            0);
                    road_vert[1]  = new Point3D(2 * edge / 3,         edge,            0);
                    road_vert[2]  = new Point3D(           0,         edge,     edge / 3);
                    road_vert[3]  = new Point3D(    edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[4]  = new Point3D(2 * edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[5]  = new Point3D(        edge, 2 * edge / 3,     edge / 3);
                    road_vert[6]  = new Point3D(           0,         edge, 2 * edge / 3);
                    road_vert[7]  = new Point3D(    edge / 3, 2 * edge / 3, 2 * edge / 3);
                    road_vert[8]  = new Point3D(2 * edge / 3,     edge / 3, 2 * edge / 3);
                    road_vert[9]  = new Point3D(        edge,     edge / 3, 2 * edge / 3);
                    road_vert[10] = new Point3D(    edge / 3, 2 * edge / 3,         edge);
                    road_vert[11] = new Point3D(2 * edge / 3,     edge / 3,         edge);

                    road_tiles.push_back(make_face(road_brightness, DARK,    3,  7,  8, -1));
                    road_tiles.push_back(make_face(road_brightness, BRIGHT,  3,  4,  8, -1));
                    if (road_up)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  0,  1,  4,  3, -1));
                    if (road_down)
                        road_tiles.push_back(make_face(road_brightness, DARK,    7,  8, 11, 10, -1));
                    if (road_left)
                        road_tiles.push_back(make_face(road_brightness, DARK,    2,  3,  7,  6, -1));
                    if (road_right)
                        road_tiles.push_back(make_face(road_brightness, BRIGHT,  4,  5,  9,  8, -1));
                }
                break;
            case 12:
                return;
            case 13:
                vertices = select_vert(ground_vert,  1,  4,  7, -1);

                faces.push_back(make_face(brightness, DARK,  0,  1,  2, -1));

                if (el.object == EL_ROAD)
                {
                    road_vert.resize(7);
                    road_vert[0] = new Point3D(    edge / 3, 2 * edge / 3,            0);
                    road_vert[1] = new Point3D(2 * edge / 3,     edge / 3,            0);
                    road_vert[2] = new Point3D(           0,         edge,     edge / 3);
                    road_vert[3] = new Point3D(    edge / 3, 2 * edge / 3,     edge / 3);
                    road_vert[4] = new Point3D(2 * edge / 3,     edge / 3,     edge / 3);
                    road_vert[5] = new Point3D(           0,         edge, 2 * edge / 3);
                    road_vert[6] = new Point3D(    edge / 3, 2 * edge / 3, 2 * edge / 3);

                    road_tiles.push_back(make_face(road_brightness, DARK,  3,  4,  6, -1));
                    if (road_up)
                        road_tiles.push_back(make_face(road_brightness, DARK,  0,  1,  4,  3, -1));
                    if (road_left)
                        road_tiles.push_back(make_face(road_brightness, DARK,  2,  3,  6,  5, -1));
                }
                break;
            case 14:
                return;
            case 15:
                vertices = select_vert(ground_vert,  4,  5,  6,  7, -1);

                faces.push_back(make_face(brightness, NORMAL,  0,  1,  2,  3, -1));

                if (el.object == EL_ROAD || el.object == EL_HOUSE)
                {
                    road_vert.resize(12);
                    road_vert[0]  = new Point3D(    edge / 3, edge,            0);
                    road_vert[1]  = new Point3D(2 * edge / 3, edge,            0);
                    road_vert[2]  = new Point3D(           0, edge,     edge / 3);
                    road_vert[3]  = new Point3D(    edge / 3, edge,     edge / 3);
                    road_vert[4]  = new Point3D(2 * edge / 3, edge,     edge / 3);
                    road_vert[5]  = new Point3D(        edge, edge,     edge / 3);
                    road_vert[6]  = new Point3D(           0, edge, 2 * edge / 3);
                    road_vert[7]  = new Point3D(    edge / 3, edge, 2 * edge / 3);
                    road_vert[8]  = new Point3D(2 * edge / 3, edge, 2 * edge / 3);
                    road_vert[9]  = new Point3D(        edge, edge, 2 * edge / 3);
                    road_vert[10] = new Point3D(    edge / 3, edge,         edge);
                    road_vert[11] = new Point3D(2 * edge / 3, edge,         edge);

                    if (el.object == EL_ROAD)
                        road_tiles.push_back(make_face(road_brightness, NORMAL,  3,  4,  8,  7, -1));
                    if (road_up)
                        road_tiles.push_back(make_face(road_brightness, NORMAL,  0,  1,  4,  3, -1));
                    if (road_down)
                        road_tiles.push_back(make_face(road_brightness, NORMAL,  7,  8, 11, 10, -1));
                    if (road_left)
                        road_tiles.push_back(make_face(road_brightness, NORMAL,  2,  3,  7,  6, -1));
                    if (road_right)
                        road_tiles.push_back(make_face(road_brightness, NORMAL,  4,  5,  9,  8, -1));
                }
        }

        Transform3D::loadIdentityModelMatrix();
        Transform3D::translateMatrix(cell.col * edge, h * edge, cell.row * edge);
        float r, g, b;
        if (sel_type == SEL_NONE)
            r = g = b = 0;
        else if (sel_type == SEL_SIMPLE)
            r = 0.5, g = b = 1;
        else
            r = g = b = 1;
        Object3D *borders = new Object3D(vertices, faces, Color(r, g, b), false);
        Transform3D::applyTransform(borders);
        to_be_drawn.push_back(borders);
        if (sel_type == SEL_NONE)
        {
            if (el.object == EL_ROAD || el.object == EL_HOUSE)
                for (int i = 0; i < static_cast<int>(road_tiles.size()); ++i)
                {
                    Object3D *borders = new Object3D(road_vert, road_tiles, Color(0, 0, 0), false);
                    Transform3D::applyTransform(borders);
                    to_be_drawn.push_back(borders);
                    vector<Face *> current_face(road_tiles.begin() + i, road_tiles.begin() + i + 1);
                    float r = 0.84f, g = 0.82f, b = 0.51f;
                    if (road_brightness[i] == BRIGHT)
                    {
                        r += 0.05f; if (r > 1) r = 1;
                        g += 0.05f; if (g > 1) g = 1;
                        b += 0.05f; if (b > 1) b = 1;
                    }
                    else if (road_brightness[i] == DARK)
                    {
                        r -= 0.05f; if (r < 0) r = 0;
                        g -= 0.05f; if (g < 0) g = 0;
                        b -= 0.05f; if (b < 0) b = 0;
                    }
                    Object3D *tile = new Object3D(road_vert, current_face, Color(r, g, b), true);
                    Transform3D::applyTransform(tile);
                    to_be_drawn.push_back(tile);
                }
            for (int i = 0; i < static_cast<int>(faces.size()); ++i)
            {
                vector<Face *> current_face(faces.begin() + i, faces.begin() + i + 1);
                float r, g, b;
                if (el.object == EL_SQUARE)
                    r = g = b = 0.75f;
                else if (edit_mode == EDIT_ROADS && el.object == EL_HOUSE && !el.connected())
                    r = 1, g = b = 0;
                else if ((el.terrain & 3) == GR_EMPTY)
                    r = 0.75f, g = 0.53f, b = 0.19f; /* pamant */
                else
                    r = 0.26f, g = 0.69f, b = 0; /* iarba */
                if ((edit_mode == EDIT_SQUARE || edit_mode == EDIT_HOUSES) &&
                    el.close_to_square() && (el.terrain & 0x3C) == 0x3C &&
                    (el.terrain & 3) != GR_WATER)
                {
                    r += 0.1f; if (r > 1) r = 1;
                    g += 0.1f; if (g > 1) g = 1;
                    b += 0.1f; if (b > 1) b = 1;
                }
                if (brightness[i] == BRIGHT)
                {
                    r += 0.05f; if (r > 1) r = 1;
                    g += 0.05f; if (g > 1) g = 1;
                    b += 0.05f; if (b > 1) b = 1;
                }
                else if (brightness[i] == DARK)
                {
                    r -= 0.05f; if (r < 0) r = 0;
                    g -= 0.05f; if (g < 0) g = 0;
                    b -= 0.05f; if (b < 0) b = 0;
                }
                Object3D *face = new Object3D(vertices, current_face, Color(r, g, b), true);
                Transform3D::applyTransform(face);
                to_be_drawn.push_back(face);
            }
        }
    }
    else if ((el.terrain & 3) == GR_WATER && el.altitude == h)
    {
        float level = edge * 4 / 5;
        vector<Point3D *> vertices{
            new Point3D(   0, level,    0),
            new Point3D(edge, level,    0),
            new Point3D(edge, level, edge),
            new Point3D(   0, level, edge),
            new Point3D(   0,  edge,    0),
            new Point3D(edge,  edge,    0),
            new Point3D(   0,  edge, edge)};

        /* fata superioara */
        vector<int> contour{0, 1, 2, 3};
        vector<Face *> faces{new Face(contour)};

        vector<Face *> right_coast;
        if (((modified.query(Cell(cell.row - 1, cell.col)).terrain & 3) == GR_EMPTY ||
            (modified.query(Cell(cell.row - 1, cell.col)).terrain & 3) == GR_GRASS))
        {
            vector<int> contour{0, 1, 5, 4};
            right_coast.push_back(new Face(contour));
        }
        vector<Face *> left_coast;
        if (((modified.query(Cell(cell.row, cell.col - 1)).terrain & 3) == GR_EMPTY ||
            (modified.query(Cell(cell.row, cell.col - 1)).terrain & 3) == GR_GRASS))
        {
            vector<int> contour{0, 3, 6, 4};
            left_coast.push_back(new Face(contour));
        }

        Transform3D::loadIdentityModelMatrix();
        Transform3D::translateMatrix(cell.col * edge, h * edge, cell.row * edge);
        Object3D *borders = new Object3D(vertices, faces, Color(0, 0, 0), false);
        Transform3D::applyTransform(borders);
        to_be_drawn.push_back(borders);
        Object3D *water = new Object3D(vertices, faces, Color(0.5f, 0.67f, 1), true);
        Transform3D::applyTransform(water);
        to_be_drawn.push_back(water);
        if (left_coast.size())
        {
            Object3D *coast_border = new Object3D(vertices, left_coast, Color(0, 0, 0), false);
            Transform3D::applyTransform(coast_border);
            to_be_drawn.push_back(coast_border);
            Object3D *coast = new Object3D(vertices, left_coast, Color(0.55f, 0.39f, 0.14f), true);
            Transform3D::applyTransform(coast);
            to_be_drawn.push_back(coast);
        }
        if (right_coast.size())
        {
            Object3D *coast_border = new Object3D(vertices, right_coast, Color(0, 0, 0), false);
            Transform3D::applyTransform(coast_border);
            to_be_drawn.push_back(coast_border);
            Object3D *coast = new Object3D(vertices, right_coast, Color(0.84f, 0.65f, 0.37f), true);
            Transform3D::applyTransform(coast);
            to_be_drawn.push_back(coast);
        }
    }
    else if (el.object == EL_HOUSE && el.altitude + (el.house_floors % 2 == 0 ? 1 : 2) +
        el.house_floors / 2 >= h)
    {
        vector<Object3D *> upside = draw_floor(cell, (h - el.altitude) * 2 - 1, (h + 0.5f) * edge);
        vector<Object3D *> downside = draw_floor(cell, (h - el.altitude - 1) * 2, h * edge);
        to_be_drawn.insert(to_be_drawn.end(), upside.begin(), upside.end());
        to_be_drawn.insert(to_be_drawn.end(), downside.begin(), downside.end());
    }
    if (!to_be_drawn.size())
        return;

    /* caut binar pozitia din drawn_elements unde ar trebui inserat */
    int a = 0;
    int b = drawn_elements.size() - 1;
    while (a <= b)
    {
        int mid = b - (b - a) / 2;
        if (compare_elements(drawn_elements[mid].cell, drawn_elements[mid].h,
                             drawn_elements[mid].sel_type, cell, h, sel_type) <= 0)
            a = mid + 1;
        else
            b = mid - 1;
    }

    /* la sfarsitul cautarii, elementul va trebui inserat in drawn_elements pe pozitia a */
    if (a >= static_cast<int>(drawn_elements.size()))
        drawn_elements.push_back(DrawnElement(cell, h, sel_type, DrawingWindow::objects3D.size()));
    else
    {
        int pos = drawn_elements[a].pos;
        drawn_elements.insert(drawn_elements.begin() + a, DrawnElement(cell, h, sel_type, pos));
        for (int i = a + 1; i < static_cast<int>(drawn_elements.size()); ++i)
            drawn_elements[i].pos += to_be_drawn.size();
    }

    DrawingWindow::objects3D.insert(DrawingWindow::objects3D.begin() + drawn_elements[a].pos,
                                    to_be_drawn.begin(), to_be_drawn.end());
}
