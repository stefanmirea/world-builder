/* Stefan-Gabriel Mirea - 331CC */

/* Implementari ale metodelor claselor Element, InfiniteMatrix, DrawnElement, DrawingWindow si alte
 * functii. */
#include <utils.hpp>

/* variabile definite in global_vars.cpp */
extern Point2D sel_center;
extern vector<string> commands;
extern int game_width, edit_mode, dir, console_mode;
extern bool special_key_pressed, world_modified, multiple_selection, fragment;
extern bool pressed_left, pressed_right, pressed_up, pressed_down;
extern float edge;
extern InfiniteMatrix<Element> modified;
extern Cell sel_begin, sel_end;
extern vector<DrawnElement> drawn_elements;
extern vector<pair<Cell, int>> clipboard;
extern void (*last_command)(stringstream &);
extern stringstream last_args;

/* variabile globale interne modulului */
/* contextele vizuale */
static Visual2D *current_cmd, *current_cmd_backgr, *cli, *cli_backgr, *game, *backgr1, *backgr2;
/* locatia din fereastra a textelor din linia de comanda si a backgroundului lor */
static Point2D cli_offset;
/* coordonatele coltului din stanga-jos al ferestrei jocului */
static Point2D camera;
/* cursorul din linia de comanda, asemanator cu '_' */
static Rectangle2D *cli_cursor;
/* obiect neafisabil care inlocuieste cursorul cand acesta trebuie sa dispara (pentru a nu face
 * insertii si stergeri liniare) */
static Rectangle2D *cli_cursor_empty;
/* pozitia din objects2D unde se afla cursorul din linia de comanda */
static int cli_cursor_pos;
/* spune daca la un momentdat cursorul din linia de comanda e afisat */
static bool cursor_visible;
/* numarul de apeluri ale functiei onIdle() */
static int idle_calls;
/* momentul curent de timp, repectiv momentul de timp al primului apel onIdle() */
static double current_time, start_time;
/* numele fisierului curent */
static string current_filename;
/* dreptunghiuri care constituie fundalurile afisate in anumite contexte vizuale */
static Rectangle2D *cmd_backgr, *window_backgr;
/* linia si coloana celui mai (1) din stanga si (2) de jos element afisat */
static Cell left_bottom;
/* linia si coloana celui mai (1) din dreapta si (2) de sus element afisat */
static Cell right_top;
/* timpul cand s-a apasat ultima data o singura sageata */
static double last_single_arrow;
/* spune daca fisierul editat e unul nou (false) sau unul cunoscut (true) */
static bool file_known;
/* frecventa performance counter-ului */
static LARGE_INTEGER freq;

/* returneaza [x] (merge corect si pe numere negative) */
static int int_trunc(const double x)
{
    if (x >= 0)
        return static_cast<int>(x);
    return static_cast<int>(x) - 1;
}

/* distanta euclidiana dintre doua celule */
static double dist(const Cell &cell1, const Cell &cell2)
{
    return sqrt((cell1.row - cell2.row) * (cell1.row - cell2.row) +
                (cell1.col - cell2.col) * (cell1.col - cell2.col));
}

/* timpul curent */
static double get_time()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return static_cast<double>(time.QuadPart) / freq.QuadPart;
}

/* spune daca un element se afla intr-un fragment (pentru a sti care sunt elementele care trebuie
 * afisate / sterse la un momentdat) */
static bool in_fragment(const Cell &cell, const Cell &left_bottom, const Cell &right_top)
{
    if (cell.row + cell.col < right_top.row + right_top.col - 1)
        return false;
    if (cell.row + cell.col > left_bottom.row + left_bottom.col + 1)
        return false;
    if (cell.row - cell.col < right_top.row - right_top.col)
        return false;
    if (cell.row - cell.col > left_bottom.row - left_bottom.col)
        return false;
    return true;
}

/* intoarce prompul din CLI, generat in functie de diferite variabile de stare */
static string get_prompt()
{
    string mode_str;
    if (edit_mode == EDIT_TERRAIN)
        mode_str = "Terrain";
    else if (edit_mode == EDIT_SQUARE)
        mode_str = "Square";
    else if (edit_mode == EDIT_HOUSES)
        mode_str = "Houses";
    else if (edit_mode == EDIT_ROADS)
        mode_str = "Roads";
    return current_filename + (world_modified ? "*" : "") + '(' + mode_str + ")>";
}

/* in functie de coordonatele camerei, recalculeaza fragmentele vizibile din lume */
static void find_fragment_coords(Cell &left_bottom, Cell &right_top)
{
    if (fragment)
        return;

    int top_strip    = -int_trunc((camera.y + 576 - EDGE_PRINTED) /
                                  (FRAGMENT_SIZE * EDGE_PRINTED)) - 1;
    int bottom_strip = -int_trunc((camera.y - EDGE_PRINTED) /
                                  (FRAGMENT_SIZE * EDGE_PRINTED)) - 1;
    int left_strip   =  int_trunc((camera.x) /
                                  (FRAGMENT_SIZE * edge * sqrt(2)));
    int right_strip  =  int_trunc((camera.x + 1024) /
                                  (FRAGMENT_SIZE * edge * sqrt(2)));

    left_bottom.row = FRAGMENT_SIZE * (-left_strip  + bottom_strip + 1) - 1;
    left_bottom.col = FRAGMENT_SIZE * ( left_strip  + bottom_strip + 1) - 1;
    right_top.row   = FRAGMENT_SIZE * (-right_strip + top_strip    - 1) - 1;
    right_top.col   = FRAGMENT_SIZE * ( right_strip + top_strip    + 1) - 1;
}

/* adauga un text in linia de comanda */
void add_cli_text(const string text)
{
    if (commands.size() < 7)
    {
        DrawingWindow::addText(
            new Text(text, Point2D(cli_offset.x, cli_offset.y + 15 * (6 - commands.size()) + 3),
                     Color(1, 1, 1), GLUT_BITMAP_9_BY_15));
        commands.push_back(text);
    }
    else
    {
        for (int i = 1; i < 6; ++i)
            commands[i] = commands[i + 1];
        commands[6] = text;
        for (int i = 1; i < 7; ++i)
            DrawingWindow::texts[i] = new Text(
                commands[i],
                Point2D(cli_offset.x, cli_offset.y + 15 * (6 - i) + 3), Color(1, 1, 1),
                GLUT_BITMAP_9_BY_15);
    }
}

/* compara coordonatele a doua elemente pentru a decide care se vede mai in fata (negativ = primul
 * element e mai in fata) */
int compare_elements(const Cell &cell1, const int h1, const int sel_type1,
                     const Cell &cell2, const int h2, const int sel_type2)
{
    if (sel_type1 == sel_type2)
    {
        if (h1 != h2)
            return h2 - h1;
        return cell2.row + cell2.col - cell1.row - cell1.col;
    }
    double h1_sublevel = h1 + (sel_type1 == SEL_NONE ? 0 : sel_type1 == SEL_SIMPLE ? 0.6 : 0.3);
    double h2_sublevel = h2 + (sel_type2 == SEL_NONE ? 0 : sel_type2 == SEL_SIMPLE ? 0.6 : 0.3);
    if (h1_sublevel < h2_sublevel)
        return 1;
    return -1;
}

/* sterge un element desenat */
bool erase_element(const Cell &cell, const int h, const int sel_type)
{
    /* caut binar pozitia din drawn_elements */
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

    /* la sfarsitul cautarii, elementul de pe pozitia a - 1 este cel mai din dreapta dintre cele de
     * aceeasi prioritate ca cel cautat */
    int pos = a - 1;
    while (pos >= 0 && !compare_elements(drawn_elements[pos].cell, drawn_elements[pos].h,
                                         drawn_elements[pos].sel_type, cell, h, sel_type)
            && (drawn_elements[pos].cell != cell || drawn_elements[pos].h != h ||
                drawn_elements[pos].sel_type != sel_type))
        --pos;
    if (pos >= 0 && drawn_elements[pos].cell == cell && drawn_elements[pos].h == h &&
        drawn_elements[pos].sel_type == sel_type)
    {
        int last_pos = (pos + 1) < static_cast<int>(drawn_elements.size()) ?
                                                    drawn_elements[pos + 1].pos :
                                                    DrawingWindow::objects3D.size();
        vector<Object3D *> to_be_deleted(DrawingWindow::objects3D.begin() + drawn_elements[pos].pos,
                                         DrawingWindow::objects3D.begin() + last_pos);
        DrawingWindow::objects3D.erase(DrawingWindow::objects3D.begin() + drawn_elements[pos].pos,
                                       DrawingWindow::objects3D.begin() + last_pos);
        drawn_elements.erase(drawn_elements.begin() + pos);
        for (unsigned int i = pos; i < drawn_elements.size(); ++i)
            drawn_elements[i].pos -= to_be_deleted.size();
        for (unsigned int i = 0; i < to_be_deleted.size(); ++i)
            delete to_be_deleted[i];
        return true;
    }
    return false;
}

/* actualizeaza ferestrele contextelor vizuale */
static void update_v2d()
{
    backgr1->fereastra(cli_offset.x + 7, cli_offset.y + 107, cli_offset.x + 8, cli_offset.y + 108);
    backgr2->fereastra(cli_offset.x + 7, cli_offset.y + 107, cli_offset.x + 8, cli_offset.y + 108);
    current_cmd->fereastra(cli_offset.x, cli_offset.y + 90, cli_offset.x + game_width,
                           cli_offset.y + 105);
    current_cmd_backgr->fereastra(cli_offset.x + 2, cli_offset.y + 107, cli_offset.x + 3,
                                  cli_offset.y + 108);
    cli->fereastra(cli_offset.x, cli_offset.y, cli_offset.x + 1024, cli_offset.y + 90);
    cli_backgr->fereastra(cli_offset.x + 2, cli_offset.y + 107, cli_offset.x + 3,
                          cli_offset.y + 108);
    game->fereastra(camera.x, camera.y, camera.x + 1024, camera.y + 576);
}

/* actualizeaza pozitiile elementelor care nu trebuie sa apara in spatiul de joc (texte, cursor,
 * fundaluri) */
static void update_hidden_elements()
{
    /* actualizez comanda curenta */
    DrawingWindow::texts[0] = new Text(commands[0], Point2D(cli_offset.x, cli_offset.y + 93),
                                       Color(1, 1, 1), GLUT_BITMAP_9_BY_15);
    Transform2D::loadIdentityMatrix();
    Transform2D::translateMatrix(cli_offset.x + commands[0].length() * 9, cli_offset.y + 91);
    Transform2D::applyTransform(cli_cursor);

    /* daca era deja text in linia de comanda, ii actualizez pozitia din fereastra */
    for (int i = 1; i < static_cast<int>(commands.size()); ++i)
        DrawingWindow::texts[i] = new Text(
            commands[i], Point2D(cli_offset.x, cli_offset.y + 15 * (6 - i) + 3), Color(1, 1, 1),
            GLUT_BITMAP_9_BY_15);

    /* translatez elementele care nu trebuie sa apara in spatiul de joc */
    Transform2D::loadIdentityMatrix();
    Transform2D::translateMatrix(cli_offset.x, cli_offset.y);
    Transform2D::applyTransform(cli_cursor_empty);
    Transform2D::applyTransform(cmd_backgr);
    Transform2D::applyTransform(window_backgr);

    update_v2d();
}

/* deseneaza sau sterge reprezentarea grafica a selectiei */
void manage_graphical_selection(const int action)
{
    /* adaug sau sterg elementele de selectie */
    Cell first(min(sel_begin.row, sel_end.row), min(sel_begin.col, sel_end.col));
    Cell last(max(sel_begin.row, sel_end.row), max(sel_begin.col, sel_end.col));
    for (int i = first.row; i <= last.row; ++i)
        for (int j = first.col; j <= last.col; ++j)
            if (Cell(i, j) != sel_end)
                if (action == DRAW)
                    draw_element(Cell(i, j), modified.query(Cell(i, j)).altitude, SEL_MULTIPLE);
                else
                    erase_element(Cell(i, j), modified.query(Cell(i, j)).altitude, SEL_MULTIPLE);
    /* adaug sau sterg cursorul */
    if (action == DRAW)
        draw_element(sel_end, modified.query(sel_end).altitude, SEL_SIMPLE);
    else
        erase_element(sel_end, modified.query(sel_end).altitude, SEL_SIMPLE);
}

/* redeseneaza obiectele ce intra in fereastra contextului vizual game, cu eventuala mutare a
 * camerei pentru a centra cursorul */
void draw_visible_objects(const bool reset_camera)
{
    /* sterg obiectele deja desenate */
    vector<Object3D *> aux(DrawingWindow::objects3D);
    DrawingWindow::objects3D.clear();
    for (unsigned int i = 0; i < aux.size(); ++i)
        delete aux[i];
    drawn_elements.clear();

    manage_graphical_selection(DRAW);

    /* cu ocazia adaugarii cursorului, am aflat ce zona e incadrata si ce obiecte trebuie sa adaug
     * mai departe */
    if (reset_camera)
    {
        camera.x = sel_center.x - 512;
        camera.y = sel_center.y - 288;
        cli_offset.x = 0;
        cli_offset.y = camera.y + 576 + FRAGMENT_SIZE * 4 * EDGE_PRINTED + CLI_OFFSET;
    }
    update_hidden_elements();
    find_fragment_coords(left_bottom, right_top);

    /* adaug elementele din lume */
    int h = max((modified.delta.col + modified.get_width() + modified.delta.row +
                 modified.get_height() - right_top.row - right_top.col - 1) / 2,
                0);
    int last_h = min((modified.delta.row + modified.delta.col - left_bottom.row - left_bottom.col
                      - 1) / 2,
                     0);
    while (h >= last_h)
    {
        Cell start;
        start.col = (left_bottom.row + left_bottom.col - right_top.row + right_top.col) / 2;
        start.row = start.col + right_top.row - right_top.col + 1;
        for (int step = 0; start.row + start.col >= right_top.row + right_top.col - 1; ++step)
        {
            int i = start.row, j = start.col;
            while (i - j <= left_bottom.row - left_bottom.col)
            {
                if (h == 0 || modified.in_matrix(Cell(i + h, j + h)))
                    draw_element(Cell(i + h, j + h), h, SEL_NONE);
                ++i;
                --j;
            }
            if (step % 2 == 0)
                --start.row;
            else
                --start.col;
        }
        --h;
    }
}

/* anuleaza selectia curenta */
void cancel_multiple_selection()
{
    multiple_selection = false;
    if (sel_end != sel_begin)
    {
        manage_graphical_selection(ERASE);
        sel_begin = sel_end;
        manage_graphical_selection(DRAW);
    }
}

/* redeseneaza apa din fata unei selectii (pentru a actualiza prezenta "malurilor") */
void update_front_water(const Cell &first, const Cell &last)
{
    for (int i = first.row; i <= last.row + 1; ++i)
        if ((modified.query(Cell(i, last.col + 1)).terrain & 3) == GR_WATER)
        {
            erase_element(Cell(i, last.col + 1), modified.query(Cell(i, last.col + 1)).altitude,
                          SEL_NONE);
            draw_element(Cell(i, last.col + 1), modified.query(Cell(i, last.col + 1)).altitude,
                         SEL_NONE);
        }
    for (int j = first.col; j <= last.col; ++j)
        if ((modified.query(Cell(last.row + 1, j)).terrain & 3) == GR_WATER)
        {
            erase_element(Cell(last.row + 1, j), modified.query(Cell(last.row + 1, j)).altitude,
                          SEL_NONE);
            draw_element(Cell(last.row + 1, j), modified.query(Cell(last.row + 1, j)).altitude,
                         SEL_NONE);
        }
}

/* actualizeaza elementele aflate la distanta de cel mult RADIUS fata de o piata */
void update_square_coverage(const bool draw_changes)
{
    InfiniteMatrix<bool> aux(modified.get_height(), modified.get_width(), modified.delta);

    vector<Cell> square;
    list<pair<Cell, int>> queue;
    for (int i = modified.delta.row; i < modified.delta.row + modified.get_height(); ++i)
        for (int j = modified.delta.col; j < modified.delta.col + modified.get_width(); ++j)
        {
            Element el = modified.query(Cell(i, j));
            aux.update(Cell(i, j), el.close_to_square());
            el.set_close_to_square(false);
            if (el.object == EL_SQUARE)
            {
                queue.push_back(make_pair(Cell(i, j), 0));
                square.push_back(Cell(i, j));
                el.set_visited(true);
            }
            else
                el.set_visited(false);
            modified.update(Cell(i, j), el);
        }
    int drow[] = {-1, -1, -1,  0,  1,  1,  1,  0};
    int dcol[] = {-1,  0,  1,  1,  1,  0, -1, -1};
    while (!queue.empty())
    {
        pair<Cell, int> front = queue.front();
        queue.pop_front();
        if (front.second)
            for (unsigned int i = 0; i < square.size(); ++i)
                if (dist(front.first, square[i]) <= RADIUS)
                {
                    Element elem = modified.query(front.first);
                    elem.set_close_to_square(true);
                    modified.update(front.first, elem);
                    break;
                }
        for (int i = 0; i < 8; ++i)
        {
            Cell neigh(front.first.row + drow[i], front.first.col + dcol[i]);
            Element neighbor = modified.query(neigh);
            if (!neighbor.visited() && front.second < RADIUS)
            {
                neighbor.set_visited(true);
                modified.update(neigh, neighbor);
                queue.push_back(make_pair(neigh, front.second + 1));
            }
        }
    }
    if (draw_changes)
    {
        int height = modified.get_height();
        for (int i = modified.delta.row; i < modified.delta.row + height; ++i)
            for (int j = modified.delta.col; j < modified.delta.col + modified.get_width(); ++j)
            {
                bool try_redraw;
                if (i < aux.delta.row || i >= aux.delta.row + aux.get_height() ||
                    j < aux.delta.col || j >= aux.delta.col + aux.get_width())
                    try_redraw = modified.query(Cell(i, j)).close_to_square();
                else
                    try_redraw = modified.query(Cell(i, j)).close_to_square() !=
                                 aux.query(Cell(i, j));
                if (try_redraw && erase_element(Cell(i, j), modified.query(Cell(i, j)).altitude,
                                                SEL_NONE))
                    draw_element(Cell(i, j), modified.query(Cell(i, j)).altitude, SEL_NONE);
            }
    }
}

/* actualizeaza informatiile privind conexiunile caselor cu pietele */
void update_square_connectivity()
{
    InfiniteMatrix<bool> aux(modified.get_height(), modified.get_width(), modified.delta);

    /* fac o parcurgere in latime pornind de la piete si deplasandu-ma doar pe drumuri */
    list<Cell> queue;
    for (int i = modified.delta.row; i < modified.delta.row + modified.get_height(); ++i)
        for (int j = modified.delta.col; j < modified.delta.col + modified.get_width(); ++j)
        {
            Element el = modified.query(Cell(i, j));
            aux.update(Cell(i, j), el.connected());
            el.set_connected(false);
            if (el.object == EL_SQUARE)
            {
                queue.push_back(Cell(i, j));
                el.set_visited(true);
            }
            else
                el.set_visited(false);
            modified.update(Cell(i, j), el);
        }

    int drow[] = {-1,  0,  1,  0};
    int dcol[] = { 0,  1,  0, -1};
    while (!queue.empty())
    {
        Cell front = queue.front();
        queue.pop_front();
        for (int i = 0; i < 4; ++i)
        {
            Cell neigh(front.row + drow[i], front.col + dcol[i]);
            Element neighbor = modified.query(neigh);
            if (!neighbor.visited())
            {
                if (neighbor.object == EL_ROAD || neighbor.object == EL_HOUSE)
                {
                    neighbor.set_visited(true);
                    modified.update(neigh, neighbor);
                }
                if (neighbor.object == EL_ROAD)
                    queue.push_back(neigh);
                else if (neighbor.object == EL_HOUSE)
                {
                    /* fac o alta parcurgere in latime a casei, pentru a o marca in intregime drept
                     * conectata */
                    list<Cell> house_queue;
                    house_queue.push_back(neigh);
                    while (!house_queue.empty())
                    {
                        Cell house_front = house_queue.front();
                        house_queue.pop_front();
                        Element el = modified.query(house_front);
                        el.set_connected(true);
                        modified.update(house_front, el);
                        for (int j = 0; j < 4; ++j)
                        {
                            Cell house_neigh(house_front.row + drow[j], house_front.col + dcol[j]);
                            Element house_neighbor = modified.query(house_neigh);
                            if (!house_neighbor.visited() && house_neighbor.object == EL_HOUSE)
                            {
                                house_neighbor.set_visited(true);
                                modified.update(house_neigh, house_neighbor);
                                house_queue.push_back(house_neigh);
                            }
                        }
                    }
                }
            }
        }
    }
    for (int i = modified.delta.row; i < modified.delta.row + modified.get_height(); ++i)
        for (int j = modified.delta.col; j < modified.delta.col + modified.get_width(); ++j)
        {
            bool try_redraw;
            if (i < aux.delta.row || i >= aux.delta.row + aux.get_height() || j < aux.delta.col ||
                j >= aux.delta.col + aux.get_width())
                try_redraw = modified.query(Cell(i, j)).connected();
            else
                try_redraw = modified.query(Cell(i, j)).connected() != aux.query(Cell(i, j));
            if (try_redraw && erase_element(Cell(i, j), modified.query(Cell(i, j)).altitude,
                                            SEL_NONE))
                draw_element(Cell(i, j), modified.query(Cell(i, j)).altitude, SEL_NONE);
        }
}

/* redeseneaza casele din jurul unei celule ce contine o casa, pentru a actualiza lipirea lor de
 * aceasta */
void redraw_houses_around(const Cell &cell)
{
    int drow[] = {-1, -1, -1,  0,  1,  1,  1,  0};
    int dcol[] = {-1,  0,  1,  1,  1,  0, -1, -1};

    for (int k = 0; k < 8; ++k)
    {
        Element neighbor = modified.query(Cell(cell.row + drow[k], cell.col + dcol[k]));
        if (neighbor.object == EL_HOUSE)
        {
            int h_max = neighbor.house_floors / 2 + 3;
            for (int h = 1; h <= h_max; ++h)
            {
                erase_element(Cell(cell.row + drow[k], cell.col + dcol[k]), neighbor.altitude + h,
                              SEL_NONE);
                draw_element(Cell(cell.row + drow[k], cell.col + dcol[k]), neighbor.altitude + h,
                             SEL_NONE);
            }
        }
    }
}

/* redeseneaza drumurile din interiorul si din jurul unei celule, pentru a actualiza unirile lor */
void redraw_roads_around(const Cell &cell)
{
    int drow[] = {0, -1, -1, -1,  0,  1,  1,  1,  0};
    int dcol[] = {0, -1,  0,  1,  1,  1,  0, -1, -1};

    for (int k = 0; k < 9; ++k)
    {
        Element neighbor = modified.query(Cell(cell.row + drow[k], cell.col + dcol[k]));
        erase_element(Cell(cell.row + drow[k], cell.col + dcol[k]), neighbor.altitude, SEL_NONE);
        draw_element(Cell(cell.row + drow[k], cell.col + dcol[k]), neighbor.altitude, SEL_NONE);
    }
}

/* functie folosite la ridicarea sau coborarea nivelului pamantului, pentru a combina vechea
 * inaltime + inclinare a reliefului (determinata de celula data din matrice) cu noua
 * inaltime(altitude) + inclinare(terrain_mask). Daca inaltimile sunt diferite, combinarea va
 * inseamna alegerea uneia din cele doua variante.
 */
bool combine(InfiniteMatrix<Element> &matrix, const Cell &cell, const int altitude,
             const unsigned char terrain_mask, const int dir)
{
    Element elem = matrix.query(cell);
    if ((dir == DIR_UP && elem.altitude > altitude) ||
        (dir == DIR_DOWN && elem.altitude < altitude))
        return false;
    if ((dir == DIR_UP && elem.altitude < altitude) ||
        (dir == DIR_DOWN && elem.altitude > altitude))
    {
        elem.altitude = altitude;
        elem.terrain = terrain_mask + (elem.terrain & 3);
        matrix.update(cell, elem);
        return true;
    }
    if (dir == DIR_UP && (elem.terrain & 0x3C) == 0x3C)
        return false;
    if (dir == DIR_UP)
        elem.terrain |= terrain_mask;
    else
    {
        elem.terrain &= terrain_mask + 0xC3;
        if ((elem.terrain & 0x3C) == 0)
        {
            --elem.altitude;
            elem.terrain |= 0x3C;
        }
    }
    matrix.update(cell, elem);
    return true;
}

/* in functie de dir, coboara sau urca relieful in zona selectata, actualizand si elementele
 * vecine pentru pastrarea unei pante maxime de 1 */
void change_relief(const int dir)
{
    if (edit_mode != EDIT_TERRAIN)
    {
        add_cli_text("You have to be in the Terrain mode to perform this action.");
        return;
    }
    InfiniteMatrix<Element> aux(modified);

    Cell first(min(sel_begin.row, sel_end.row), min(sel_begin.col, sel_end.col));
    Cell last(max(sel_begin.row, sel_end.row), max(sel_begin.col, sel_end.col));

    for (int i = first.row; i <= last.row; ++i)
        for (int j = first.col; j <= last.col; ++j)
        {
            Element elem = modified.query(Cell(i, j));
            int new_altitude;
            if (dir == DIR_UP)
                new_altitude = (elem.terrain & 0x3C) != 0x3C ? elem.altitude : elem.altitude + 1;
            else
                new_altitude = elem.altitude - 1;
            combine(aux, Cell(i, j), new_altitude, 0x3C, dir);
            if (dir == DIR_DOWN)
                ++new_altitude;
            int alt_inc = dir == DIR_UP ? -1 : 1;
            /* spune daca elementele pe care incerc sa le suprapun nu se mai vad */
            bool hidden = false;
            for (int row = i - 1, current_altitude = new_altitude; !hidden;
                 --row, current_altitude += alt_inc)
            {
                hidden = true;
                for (int col = row - i + j + 1; col <= i + j - 1 - row; ++col)
                    if (combine(aux, Cell(row, col), current_altitude, dir == DIR_UP ? 0x30 : 0x0C,
                                dir))
                        hidden = false;
            }
            hidden = false;
            for (int col = j + 1, current_altitude = new_altitude; !hidden;
                 ++col, current_altitude += alt_inc)
            {
                hidden = true;
                for (int row = i + j - col + 1; row <= col + i - j - 1; ++row)
                    if (combine(aux, Cell(row, col), current_altitude, dir == DIR_UP ? 0x24 : 0x18,
                                dir))
                        hidden = false;
            }
            hidden = false;
            for (int row = i + 1, current_altitude = new_altitude; !hidden;
                 ++row, current_altitude += alt_inc)
            {
                hidden = true;
                for (int col = i + j - row + 1; col <= row - i + j - 1; ++col)
                    if (combine(aux, Cell(row, col), current_altitude, dir == DIR_UP ? 0x0C : 0x30,
                                dir))
                        hidden = false;
            }
            hidden = false;
            for (int col = j - 1, current_altitude = new_altitude; !hidden;
                 --col, current_altitude += alt_inc)
            {
                hidden = true;
                for (int row = col + i - j + 1; row <= i + j - col - 1; ++row)
                    if (combine(aux, Cell(row, col), current_altitude, dir == DIR_UP ? 0x18 : 0x24,
                                dir))
                        hidden = false;
            }
            int row = i - 1;
            int col = j - 1;
            int current_altitude = new_altitude;
            while (combine(aux, Cell(row, col), current_altitude, dir == DIR_UP ? 0x10 : 0x2C, dir))
                --row, --col, current_altitude += alt_inc;
            row = i - 1;
            col = j + 1;
            current_altitude = new_altitude;
            while (combine(aux, Cell(row, col), current_altitude, dir == DIR_UP ? 0x20 : 0x1C, dir))
                --row, ++col, current_altitude += alt_inc;
            row = i + 1;
            col = j + 1;
            current_altitude = new_altitude;
            while (combine(aux, Cell(row, col), current_altitude, dir == DIR_UP ? 0x04 : 0x38, dir))
                ++row, ++col, current_altitude += alt_inc;
            row = i + 1;
            col = j - 1;
            current_altitude = new_altitude;
            while (combine(aux, Cell(row, col), current_altitude, dir == DIR_UP ? 0x08 : 0x34, dir))
                ++row, --col, current_altitude += alt_inc;
        }

    for (int i = aux.delta.row; i < aux.delta.row + aux.get_height(); ++i)
        for (int j = aux.delta.col; j < aux.delta.col + aux.get_width(); ++j)
        {
            Element elem = aux.query(Cell(i, j));
            if ((elem.terrain & 0x3C) != 0x3C &&
                ((elem.terrain & 3) == GR_WATER || elem.object == EL_SQUARE ||
                    elem.object == EL_HOUSE))
            {
                add_cli_text("Error: modifying terrain would lay some objects (water/squares/houses"
                             ") on a tilted surface.");
                return;
            }
        }

    manage_graphical_selection(ERASE);
    swap(modified, aux);
    for (int i = modified.delta.row; i < modified.delta.row + modified.get_height(); ++i)
        for (int j = modified.delta.col; j < modified.delta.col + modified.get_width(); ++j)
        {
            Element mod_elem = modified.query(Cell(i, j));
            Element aux_elem = aux.query(Cell(i, j));
            if (mod_elem.altitude != aux_elem.altitude || mod_elem.terrain != aux_elem.terrain)
            {
                erase_element(Cell(i, j), aux_elem.altitude, SEL_NONE);
                draw_element(Cell(i, j), aux_elem.altitude, SEL_NONE);
                if (aux_elem.object == EL_HOUSE)
                {
                    int h_max = aux_elem.house_floors / 2 + 3;
                    for (int h = 1; h <= h_max; ++h)
                    {
                        erase_element(Cell(i, j), aux_elem.altitude + h, SEL_NONE);
                        draw_element(Cell(i, j), aux_elem.altitude + h, SEL_NONE);
                    }
                }
                erase_element(Cell(i, j), mod_elem.altitude, SEL_NONE);
                draw_element(Cell(i, j), mod_elem.altitude, SEL_NONE);
                if (mod_elem.object == EL_HOUSE)
                {
                    int h_max = mod_elem.house_floors / 2 + 3;
                    for (int h = 1; h <= h_max; ++h)
                    {
                        erase_element(Cell(i, j), mod_elem.altitude + h, SEL_NONE);
                        draw_element(Cell(i, j), mod_elem.altitude + h, SEL_NONE);
                    }
                }
            }
        }

    if (multiple_selection)
        cancel_multiple_selection();
    else
        manage_graphical_selection(DRAW);
    world_modified = true;
}

/* roteste casele ce au cel putin un corp in zona selectata in directia way */
void rotate_houses(const int way)
{
    Cell first(min(sel_begin.row, sel_end.row), min(sel_begin.col, sel_end.col));
    Cell last(max(sel_begin.row, sel_end.row), max(sel_begin.col, sel_end.col));

    /* marchez tot spatiul ca nevizitat */
    for (int i = 0; i < modified.get_height(); ++i)
        for (int j = 0; j < modified.get_width(); ++j)
        {
            Element elem = modified.query(Cell(i + modified.delta.row, j + modified.delta.col));
            elem.set_visited(false);
            modified.update(Cell(i + modified.delta.row, j + modified.delta.col), elem);
        }

    bool succeeded = true;
    bool house_found = false;

    for (int i = first.row; i <= last.row; ++i)
        for (int j = first.col; j <= last.col; ++j)
        {
            Element elem = modified.query(Cell(i, j));
            if (elem.object == EL_HOUSE && !elem.visited())
            {
                house_found = true;
                elem.set_visited(true);
                modified.update(Cell(i, j), elem);
                Cell first_house(i, j), last_house(i, j);

                /* casele fuzioneaza numai pe verticala sau orizontala */
                int drow[] = {-1, 0, 1,  0};
                int dcol[] = { 0, 1, 0, -1};
                /* coada pentru BFS pe elementele casei */
                list<pair<Cell, Element>> queue;
                /* lista in care mut temporar toate elementele casei */
                vector<pair<Cell, Element>> temp;
                queue.push_back(make_pair(Cell(i, j), elem));
                while (!queue.empty())
                {
                    pair<Cell, Element> front = queue.front();
                    queue.pop_front();
                    temp.push_back(front);
                    Element house_deleted = front.second;
                    house_deleted.object = EL_EMPTY;
                    house_deleted.house_floors = 0;
                    modified.update(front.first, house_deleted);

                    if (front.first.row < first_house.row)
                        first_house.row = front.first.row;
                    if (front.first.row > last_house.row)
                        last_house.row = front.first.row;
                    if (front.first.col < first_house.col)
                        first_house.col = front.first.col;
                    if (front.first.col > last_house.col)
                        last_house.col = front.first.col;

                    for (int k = 0; k < 4; ++k)
                    {
                        Element neighbor = modified.query(Cell(front.first.row + drow[k],
                                                               front.first.col + dcol[k]));
                        if (!neighbor.visited() && neighbor.object == EL_HOUSE)
                        {
                            neighbor.set_visited(true);
                            modified.update(Cell(front.first.row + drow[k],
                                                 front.first.col + dcol[k]), neighbor);
                            queue.push_back(make_pair(Cell(front.first.row + drow[k],
                                                           front.first.col + dcol[k]), neighbor));
                        }
                    }
                }

                /* verific daca rotatia poate fi efectuata */
                bool rotation_possible = true;
                vector<Cell> new_coords;
                for (unsigned int k = 0; k < temp.size(); ++k)
                {
                    /* calculez coordonatele dupa rotatie */
                    Cell new_cell;
                    new_cell.row = first_house.row + (last_house.row - first_house.row -
                                   last_house.col + first_house.col) / 2;
                    new_cell.col = new_cell.col = first_house.col +
                                                  (last_house.col - first_house.col - last_house.row
                                                    + first_house.row) / 2;
                    if (way == DIR_CLOCKWISE)
                    {
                         new_cell.row += temp[k].first.col - first_house.col;
                         new_cell.col += last_house.row - temp[k].first.row;
                    }
                    else
                    {
                        new_cell.row += last_house.col - temp[k].first.col;
                        new_cell.col += temp[k].first.row - first_house.row;
                    }
                    new_coords.push_back(new_cell);
                    Element to_be_replaced = modified.query(new_cell);
                    if ((to_be_replaced.terrain & 0x3C) != 0x3C ||
                        (to_be_replaced.terrain & 3) == GR_WATER ||
                        to_be_replaced.object != EL_EMPTY || !to_be_replaced.close_to_square())
                    {
                        rotation_possible = false;
                        break;
                    }
                }

                if (!rotation_possible)
                    succeeded = false;

                /* repozitionez elementele unde au fost initial daca nu pot fi rotite, sau in noua
                 * pozitie in caz contrar */
                for (unsigned int k = 0; k < temp.size(); ++k)
                {
                    Cell new_cell;
                    if (!rotation_possible)
                        new_cell = temp[k].first;
                    else
                        new_cell = new_coords[k];
                    modified.update(new_cell, temp[k].second);
                }

                /* actualizez afisarea */
                if (rotation_possible)
                    for (unsigned int k = 0; k < temp.size(); ++k)
                    {
                        /* sterg vechiul element al casei (pana la inaltimea sa) si il adaug pe cel
                         * nou */
                        int h_max = temp[k].second.house_floors / 2 + 3;
                        for (int h = 1; h <= h_max; ++h)
                        {
                            erase_element(temp[k].first, temp[k].second.altitude + h, SEL_NONE);
                            draw_element(temp[k].first, temp[k].second.altitude + h, SEL_NONE);
                            erase_element(new_coords[k], temp[k].second.altitude + h, SEL_NONE);
                            draw_element(new_coords[k], temp[k].second.altitude + h, SEL_NONE);
                        }
                        /* daca in urma rotatiei casa a fuzionat cu elemente ale altor case vecine,
                         * trebuie reafisate si acelea */
                        redraw_houses_around(new_coords[k]);

                        /* actualizez drumurile din vechiul si noul loc */
                        redraw_roads_around(temp[k].first);
                        redraw_roads_around(new_coords[k]);
                    }
            }
        }
    if (!house_found)
        add_cli_text("No house selected.");
    else if (succeeded)
    {
        update_square_connectivity();
        world_modified = true;
    }
    else
    {
        add_cli_text("One or more houses could not be rotated:");
        add_cli_text("There is not enough room or the house would move away from square coverage.");
    }
}

/* simuleaza rotirea camerei prin rotirea elementelor */
void rotate_camera(const int way)
{
    /* rotesc elementele lumii */
    InfiniteMatrix<Element> new_modified(modified.get_width(), modified.get_height(), Cell(0, 0));
    for (int i = 0; i < modified.get_height(); ++i)
        for (int j = 0; j < modified.get_width(); ++j)
        {
            Cell new_cell;
            new_cell.row = way == DIR_CLOCKWISE ? j : modified.get_width() - j - 1;
            new_cell.col = way == DIR_CLOCKWISE ? modified.get_height() - i - 1 : i;
            Element el = modified.query(Cell(i + modified.delta.row, j + modified.delta.col));
            unsigned char tilt;
            if (way == DIR_CLOCKWISE)
                tilt = ((el.terrain & 0x1C) << 1) + (el.terrain & 0x20 ? 4 : 0);
            else
                tilt = ((el.terrain & 0x38) >> 1) + (el.terrain & 4 ? 0x20 : 0);
            el.terrain &= 3;
            el.terrain |= tilt;
            new_modified.update(new_cell, el);
        }

    /* rotesc selectia */
    if (way == DIR_CLOCKWISE)
    {
        int aux = sel_begin.row;
        sel_begin.row = sel_begin.col - modified.delta.col;
        sel_begin.col = modified.get_height() - (aux - modified.delta.row) - 1;
        aux = sel_end.row;
        sel_end.row = sel_end.col - modified.delta.col;
        sel_end.col = modified.get_height() - (aux - modified.delta.row) - 1;
    }
    else
    {
        int aux = sel_begin.row;
        sel_begin.row = modified.get_width() - (sel_begin.col - modified.delta.col) - 1;
        sel_begin.col = aux - modified.delta.row;
        aux = sel_end.row;
        sel_end.row = modified.get_width() - (sel_end.col - modified.delta.col) - 1;
        sel_end.col = aux - modified.delta.row;
    }

    modified = new_modified;
    draw_visible_objects(true);

    /* rotesc elementele din clipboard */
    for (unsigned int i = 0; i < clipboard.size(); ++i)
    {
        int aux = clipboard[i].first.row;
        if (way == DIR_CLOCKWISE)
        {
            clipboard[i].first.row = clipboard[i].first.col;
            clipboard[i].first.col = -aux;
        }
        else
        {
            clipboard[i].first.row = -clipboard[i].first.col;
            clipboard[i].first.col = aux;
        }
    }
}

/* copiaza / muta casele ce au cel putin un corp in zona selectata */
void copy_houses(const bool keep_original)
{
    Cell first(min(sel_begin.row, sel_end.row), min(sel_begin.col, sel_end.col));
    Cell last(max(sel_begin.row, sel_end.row), max(sel_begin.col, sel_end.col));

    /* marchez tot spatiul ca nevizitat */
    for (int i = 0; i < modified.get_height(); ++i)
        for (int j = 0; j < modified.get_width(); ++j)
        {
            Element elem = modified.query(Cell(i + modified.delta.row, j + modified.delta.col));
            elem.set_visited(false);
            modified.update(Cell(i + modified.delta.row, j + modified.delta.col), elem);
        }

    bool house_found = false;
    clipboard.clear();

    for (int i = first.row; i <= last.row; ++i)
        for (int j = first.col; j <= last.col; ++j)
        {
            Element elem = modified.query(Cell(i, j));
            if (elem.object == EL_HOUSE && !elem.visited())
            {
                house_found = true;
                elem.set_visited(true);
                modified.update(Cell(i, j), elem);

                /* casele fuzioneaza numai pe verticala sau orizontala */
                int drow[] = {-1, 0, 1,  0};
                int dcol[] = { 0, 1, 0, -1};
                /* coada pentru BFS pe elementele casei */
                list<pair<Cell, Element>> queue;
                queue.push_back(make_pair(Cell(i, j), elem));
                while (!queue.empty())
                {
                    pair<Cell, Element> front = queue.front();
                    queue.pop_front();
                    clipboard.push_back(make_pair(Cell(front.first.row - sel_end.row,
                                                       front.first.col - sel_end.col),
                                                  front.second.house_floors));
                    if (!keep_original)
                    {
                        Element house_deleted = front.second;
                        house_deleted.object = EL_EMPTY;
                        house_deleted.house_floors = 0;
                        modified.update(front.first, house_deleted);

                        int h_max = front.second.house_floors / 2 + 3;
                        for (int h = 1; h <= h_max; ++h)
                            erase_element(front.first, house_deleted.altitude + h, SEL_NONE);

                        redraw_roads_around(front.first);
                    }

                    for (int k = 0; k < 4; ++k)
                    {
                        Element neighbor = modified.query(Cell(front.first.row + drow[k],
                                                               front.first.col + dcol[k]));
                        if (!neighbor.visited() && neighbor.object == EL_HOUSE)
                        {
                            neighbor.set_visited(true);
                            modified.update(Cell(front.first.row + drow[k],
                                                 front.first.col + dcol[k]), neighbor);
                            queue.push_back(make_pair(Cell(front.first.row + drow[k],
                                                           front.first.col + dcol[k]), neighbor));
                        }
                    }
                }
            }
        }

    if (!house_found)
        add_cli_text("No house selected.");
    else
    {
        if (!keep_original)
        {
            update_square_connectivity();
            world_modified = true;
        }
        if (multiple_selection)
            cancel_multiple_selection();
    }
}

/* gestioneaza dialogul de introducere a numelui fisierului in care se face salvarea */
static int filename_dialog(string &filename)
{
    static bool first_call = true;

    if (first_call)
    {
        add_cli_text("Please enter the file name. Leave blank to cancel.");
        first_call = false;
        return ACT_WAIT;
    }

    first_call = true;
    if (commands[0].length() == 0)
        return ACT_ABORT;

    filename = commands[0];
    return ACT_PROCEED;
}

/* gestioneaza dialogul privind suprascrierea unui fisier deja existent */
static int overwrite_dialog(const string &new_name)
{
    static bool first_call = true;

    if (first_call)
    {
        add_cli_text("\"" + new_name + "\" already exists. Overwrite? (yes/no)");
        first_call = false;
        return ACT_WAIT;
    }

    if (commands[0].length() == 0)
        return ACT_WAIT;

    if (string("yes").find(commands[0]) == 0)
    {
        first_call = true;
        return ACT_PROCEED;
    }

    if (string("no").find(commands[0]) == 0)
    {
        first_call = true;
        return ACT_ABORT;
    }

    add_cli_text("Please enter 'yes' or 'no'.");
    return ACT_WAIT;
}

int save_game(void (*caller)(stringstream &), const stringstream &caller_args,
              string &save_filename)
{
    static bool file_exists, file_exists_initialized = false;

    if (save_filename.length() == 0 && file_known)
        save_filename = current_filename;
    if (save_filename.length() == 0)
    {
        if (console_mode == CONS_GLOBAL)
        {
            console_mode = CONS_LOCAL;
            last_command = caller;
            last_args.clear();
            last_args << caller_args.rdbuf();
        }
        int action = filename_dialog(save_filename);
        if (action == ACT_ABORT)
        {
            save_filename.clear();
            file_exists_initialized = false;
        }
        if (action == ACT_PROCEED || action == ACT_ABORT)
            console_mode = CONS_GLOBAL;
        if (action == ACT_WAIT || action == ACT_ABORT)
            return action;
    }
    if (!file_known || current_filename != save_filename)
    {
        if (!file_exists_initialized)
        {
            ifstream file(save_filename);
            if (file.is_open())
            {
                file_exists = true;
                file.close();
            }
            else
                file_exists = false;
            file_exists_initialized = true;
        }
        if (file_exists)
        {
            if (console_mode == CONS_GLOBAL)
            {
                console_mode = CONS_LOCAL;
                last_command = caller;
                last_args.clear();
                last_args << caller_args.rdbuf();
            }
            int action = overwrite_dialog(save_filename);
            if (action == ACT_ABORT)
            {
                save_filename.clear();
                file_exists_initialized = false;
            }
            if (action == ACT_PROCEED || action == ACT_ABORT)
                console_mode = CONS_GLOBAL;
            if (action == ACT_WAIT || action == ACT_ABORT)
                return action;
        }
    }

    int ret;
    ofstream file(save_filename, ios::out | ios::binary);
    if (!file.is_open())
    {
        add_cli_text("Error: cannot open \"" + save_filename + "\" for output.");
        ret = ACT_ABORT;
    }
    else
    {
        modified.write(file);

        Cell selection(sel_end.row - modified.delta.row, sel_end.col - modified.delta.col);
        file.write(reinterpret_cast<const char *>(&selection.row), sizeof(selection.row));
        file.write(reinterpret_cast<const char *>(&selection.col), sizeof(selection.col));
        file.write(reinterpret_cast<const char *>(&edge), sizeof(edge));

        file.close();
        add_cli_text("Successfully saved as \"" + save_filename + "\".");
        if (!file_known || current_filename != save_filename)
        {
            file_known = true;
            current_filename = save_filename;
        }
        world_modified = false;
        ret = ACT_PROCEED;
    }

    save_filename.clear();
    file_exists_initialized = false;
    return ret;
}

/* gestioneaza dialogul de salvare a modificarilor (la inceperea unui joc nou / deschiderea unui
 * fisier / iesire) */
int save_dialog(void (*caller)(stringstream &), const stringstream &caller_args)
{
    static bool first_call = true;
    static bool save = false;
    static string initial_filename;

    if (first_call)
    {
        add_cli_text("Save changes to \"" + current_filename + "\"? (yes/no/cancel)");
        first_call = false;
        return ACT_WAIT;
    }

    if (!save)
    {
        if (commands[0].length() == 0)
            return ACT_WAIT;

        if (string("no").find(commands[0]) == 0)
        {
            first_call = true;
            return ACT_PROCEED;
        }

        if (string("cancel").find(commands[0]) == 0)
        {
            first_call = true;
            return ACT_ABORT;
        }

        if (string("yes").find(commands[0]) == 0)
            save = true;
    }
    if (save)
    {
        int action = save_game(caller, caller_args, initial_filename);
        if (action == ACT_PROCEED || action == ACT_ABORT)
        {
            first_call = true;
            save = false;
        }
        return action;
    }

    add_cli_text("Please enter 'yes', 'no' or 'cancel'.");
    return ACT_WAIT;
}

bool load_game(const string &filename)
{
    ifstream file(filename, ios::in | ios::binary);
    if (!file.is_open())
    {
        add_cli_text("Error: cannot open \"" + filename + "\" for input.");
        return false;
    }

    modified.read(file);
    file.read(reinterpret_cast<char *>(&sel_end.row), sizeof(sel_end.row));
    file.read(reinterpret_cast<char *>(&sel_end.col), sizeof(sel_end.col));
    file.read(reinterpret_cast<char *>(&edge), sizeof(edge));
    file.close();

    file_known = true;
    current_filename = filename;
    world_modified = false;
    edit_mode = EDIT_TERRAIN;
    commands[0] = get_prompt();

    sel_begin = sel_end;
    multiple_selection = false;
    fragment = false;
    draw_visible_objects(true);
    return true;
}

void create_new_game()
{
    file_known = false;
    current_filename = "Untitled";
    world_modified = false;
    edit_mode = EDIT_TERRAIN;
    commands[0] = get_prompt();

    modified.clear();
    edge = EDGE_DEFAULT;
    sel_begin.row = sel_begin.col = sel_end.row = sel_end.col = 0;
    multiple_selection = false;
    fragment = false;
    draw_visible_objects(true);
}

/* functia care permite adaugarea de obiecte */
void DrawingWindow::init(int argc, char** argv)
{
    commands.push_back("");
    addText(new Text("", Point2D(0, 0), Color(1, 1, 1), GLUT_BITMAP_9_BY_15));
    cli_cursor = new Rectangle2D(Point2D(0, 0), 9, 2, Color(1, 1, 1), true);
    cli_cursor_pos = objects2D.size();
    cli_cursor_empty = new Rectangle2D(Point2D(12, 107), 1, 1);
    addObject2D(cli_cursor_empty);
    cursor_visible = false;
    cmd_backgr = new Rectangle2D(Point2D(0, 105), 5, 5, Color(0, 0.24f, 0.15f), true);
    addObject2D(cmd_backgr);
    window_backgr = new Rectangle2D(Point2D(5, 105), 5, 5, Color(0, 0, 0), true);
    addObject2D(window_backgr);

    backgr1 = new Visual2D(0, 0, 0, 0, 0, 0, 0, 685);
    addVisual2D(backgr1);
    backgr2 = new Visual2D(0, 0, 0, 0, 1024, 0, 1024, 685);
    addVisual2D(backgr2);
    current_cmd = new Visual2D(0, 0, 0, 0, 0, 578, 1024, 593);
    addVisual2D(current_cmd);
    current_cmd_backgr = new Visual2D(0, 0, 0, 0, 0, 578, 1024, 593);
    addVisual2D(current_cmd_backgr);
    cli = new Visual2D(0, 0, 0, 0, 0, 595, 1024, 685);
    addVisual2D(cli);
    cli_backgr = new Visual2D(0, 0, 0, 0, 0, 595, 1024, 685);
    addVisual2D(cli_backgr);
    game = new Visual2D(0, 0, 0, 0, 0, 0, 1024, 576);
    addVisual2D(game);

    Transform3D::loadIdentityProjectionMatrix();
    Transform3D::isometricProjectionMatrix();

    if (argc == 1 || !load_game(string(argv[1])))
        create_new_game();
    console_mode = CONS_GLOBAL;
    last_command = NULL;
}

/* functia care permite animatia */
void DrawingWindow::onIdle()
{
    /* actualizarea cursorului din linia de comanda */
    if (!idle_calls)
    {
        QueryPerformanceFrequency(&freq);
        start_time = get_time();
    }
    current_time = get_time();
    if (static_cast<int>((current_time - start_time) * 1000) % BLINK_PERIOD < BLINK_PERIOD / 2)
    {
        if (!cursor_visible)
        {
            objects2D[cli_cursor_pos] = cli_cursor;
            cursor_visible = true;
        }
    }
    else if (cursor_visible)
    {
        objects2D[cli_cursor_pos] = cli_cursor_empty;
        cursor_visible = false;
    }

    /* actualizarea pozitiei camerei */
    float diff_y = sel_center.y - camera.y;
    if (diff_y <= EDGE_PRINTED / 2) /* selectia e pe cale sa iasa din cadru (ducandu-se in jos) */
        camera.y = sel_center.y - EDGE_PRINTED / 2 - 1;
    else if (diff_y <= 286)
        camera.y -= 1 + (1 - (diff_y - EDGE_PRINTED / 2) / (286 - EDGE_PRINTED / 2)) * 20;
    else if (diff_y >= 576 - EDGE_PRINTED / 2)
        camera.y = sel_center.y - (576 - EDGE_PRINTED / 2 - 1);
    else if (diff_y >= 290) /* selectia e pe cale sa iasa din cadru (ducandu-se in sus) */
        camera.y += 1 + (diff_y - 290) / (286 - EDGE_PRINTED / 2) * 20;
    float diff_x = sel_center.x - camera.x;
    if (diff_x <= edge * sqrt(2) / 2) /* selectia e pe cale sa iasa din cadru (ducandu-se in
                                       * stanga) */
        camera.x = sel_center.x - edge * sqrt(2.0f) / 2 - 1;
    else if (diff_x <= 510)
        camera.x -= 1 + (1 - (diff_x - edge * sqrt(2.0f) / 2) / (510 - edge * sqrt(2.0f) / 2)) * 20;
    else if (diff_x >= 1024 - edge * sqrt(2) / 2)
        camera.x = sel_center.x - (1024 - edge * sqrt(2.0f) / 2 - 1);
    else if (diff_x >= 514) /* selectia e pe cale sa iasa din cadru (ducandu-se in dreapta) */
        camera.x += 1 + (diff_x - 514) / (510 - edge * sqrt(2.0f) / 2) * 20;
    game->fereastra(camera.x, camera.y, camera.x + 1024, camera.y + 576);

    /* actualizez pozitiile elementelor din care e alcatuita linia de comanda si ferestrele
     * contextelor vizuale */
    if (cli_offset.y - camera.y < 576 + FRAGMENT_SIZE * 4 * EDGE_PRINTED)
    {
        cli_offset.y = camera.y + 576 + FRAGMENT_SIZE * 4 * EDGE_PRINTED + CLI_OFFSET;
        update_hidden_elements();
    }

    /* actualizarea elementelor vizibile */
    if (!fragment)
    {
        Cell new_left_bottom, new_right_top;
        find_fragment_coords(new_left_bottom, new_right_top);
        if (new_left_bottom != left_bottom || new_right_top != right_top)
        {
            /* parcurg vechile elemente pentru a vedea care ar trebui sterse de pe ecran */
            int col1 = right_top.col - 1, col2 = right_top.col;
            int col1_inc = -1, col2_inc = 1;
            for (int row = right_top.row; row <= left_bottom.row; ++row)
            {
                for (int col = col1; col <= col2; ++col)
                    if (!in_fragment(Cell(row, col), new_left_bottom, new_right_top))
                    {
                        /* un element care trebuie sters */
                        erase_element(Cell(row, col), 0, SEL_NONE);
                        int h = min(modified.delta.row + modified.get_height() - 1 - row,
                                    modified.delta.col + modified.get_width() - 1 - col);
                        int last_h = max(modified.delta.col - col, modified.delta.row - row);
                        while (h >= last_h)
                        {
                            if (h) /* pentru h = 0 am tratat separat */
                                erase_element(Cell(row + h, col + h), h, SEL_NONE);
                            --h;
                        }
                    }
                col1 += col1_inc;
                col2 += col2_inc;
                if (col1_inc == -1 && row - col1 + 1 > left_bottom.row - left_bottom.col)
                {
                    col1_inc = 1;
                    ++col1;
                }
                if (col2_inc == 1 && row + col2 + 1 > left_bottom.row + left_bottom.col)
                {
                    col2_inc = -1;
                    --col2;
                }
            }
            /* parcurg noile elemente pentru a vedea care trebuie adaugate */
            col1 = new_right_top.col - 1, col2 = new_right_top.col;
            col1_inc = -1, col2_inc = 1;
            for (int row = new_right_top.row; row <= new_left_bottom.row; ++row)
            {
                for (int col = col1; col <= col2; ++col)
                    if (!in_fragment(Cell(row, col), left_bottom, right_top))
                    {
                        /* un element care trebuie adaugat */
                        draw_element(Cell(row, col), 0, SEL_NONE);
                        int h = min(modified.delta.row + modified.get_height() - 1 - row,
                                    modified.delta.col + modified.get_width() - 1 - col);
                        int last_h = max(modified.delta.col - col, modified.delta.row - row);
                        while (h >= last_h)
                        {
                            if (h)
                                draw_element(Cell(row + h, col + h), h, SEL_NONE);
                            --h;
                        }
                    }
                col1 += col1_inc;
                col2 += col2_inc;
                if (col1_inc == -1 && row - col1 + 1 > new_left_bottom.row - new_left_bottom.col)
                {
                    col1_inc = 1;
                    ++col1;
                }
                if (col2_inc == 1 && row + col2 + 1 > new_left_bottom.row + new_left_bottom.col)
                {
                    col2_inc = -1;
                    --col2;
                }
            }
            left_bottom = new_left_bottom;
            right_top = new_right_top;
        }
    }

    ++idle_calls;
}

/* functia care se apeleaza la redimensionarea ferestrei */
void DrawingWindow::onReshape(int width, int height)
{
    if ((height - 109) * 16 / 9 <= width)
    {
        game_width = (height - 109) * 16 / 9;
        int left_limit = (width - game_width) / 2;
        current_cmd->fereastra(cli_offset.x, cli_offset.y + 90, cli_offset.x + game_width,
                               cli_offset.y + 105);
        current_cmd->poarta(left_limit, height - 107, left_limit + game_width, height - 92);
        current_cmd_backgr->poarta(left_limit, height - 107, left_limit + game_width, height - 92);
        cli->poarta(left_limit, height - 90, left_limit + game_width, height);
        cli_backgr->poarta(left_limit, height - 90, left_limit + game_width, height);
        game->poarta(left_limit, 0, left_limit + game_width, height - 109);
        backgr1->poarta(0, 0, left_limit, height);
        backgr2->poarta(left_limit + game_width, 0, width, height);
    }
    else
    {
        int game_height = width * 9 / 16;
        game_width = width;
        int upper_limit = (height - game_height - 109) / 2;
        current_cmd->fereastra(cli_offset.x, cli_offset.y + 90, cli_offset.x + width,
                               cli_offset.y + 105);
        current_cmd->poarta(0, upper_limit + game_height + 2, width,
                            upper_limit + game_height + 17);
        current_cmd_backgr->poarta(0, upper_limit + game_height + 2, width,
                                   upper_limit + game_height + 17);
        cli->poarta(0, upper_limit + game_height + 19, width, upper_limit + game_height + 109);
        cli_backgr->poarta(0, upper_limit + game_height + 19, width,
                           upper_limit + game_height + 109);
        game->poarta(0, upper_limit, width, upper_limit + game_height);
        backgr1->poarta(0, 0, width, upper_limit);
        backgr2->poarta(0, upper_limit + game_height + 109, width, height);
    }
}

/* functia care defineste ce se intampla cand se apasa pe tastatura */
void DrawingWindow::onKey(unsigned char key)
{
    if (special_key_pressed)
    {
        if (pressed_left == false && pressed_right == false && pressed_up == false &&
            pressed_down == false && (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP ||
            key == KEY_DOWN))
            last_single_arrow = get_time();
        if (key == KEY_LEFT)
            pressed_left = true;
        if (key == KEY_RIGHT)
            pressed_right = true;
        if (key == KEY_UP)
            pressed_up = true;
        if (key == KEY_DOWN)
            pressed_down = true;

        /* incerc sa creez o directie compusa */
        bool composed_dir = true;
        int arrows_pressed = (pressed_left ? 1 : 0) + (pressed_right ? 1 : 0) +
                             (pressed_up   ? 1 : 0) + (pressed_down  ? 1 : 0);
        if (arrows_pressed == 2 &&
            static_cast<int>((get_time() - last_single_arrow) * 1000) <= DIAGONAL_DIR_TIME)
        {
            if (pressed_left && pressed_up)
                dir = DIR_LEFT_UP;
            else if (pressed_right && pressed_up)
                dir = DIR_RIGHT_UP;
            else if (pressed_left && pressed_down)
                dir = DIR_LEFT_DOWN;
            else if (pressed_right && pressed_down)
                dir = DIR_RIGHT_DOWN;
            else
                composed_dir = false;
        }
        else
            composed_dir = false;

        /* actualizez selectia */
        manage_graphical_selection(ERASE);
        if (composed_dir)
        {
            /* anulez ultima deplasare a selectiei, datorata interpretarii gresite a directiei */
            if (dir == DIR_LEFT_UP)
            {
                if (key == KEY_LEFT)
                    ++sel_end.row, ++sel_end.col;
                else
                    --sel_end.row, ++sel_end.col;
            }
            else if (dir == DIR_RIGHT_UP)
            {
                if (key == KEY_RIGHT)
                    ++sel_end.row, ++sel_end.col;
                else
                    ++sel_end.row, --sel_end.col;
            }
            else if (dir == DIR_LEFT_DOWN)
            {
                if (key == KEY_LEFT)
                    --sel_end.row, --sel_end.col;
                else
                    --sel_end.row, ++sel_end.col;
            }
            else if (dir == DIR_RIGHT_DOWN)
            {
                if (key == KEY_RIGHT)
                    --sel_end.row, --sel_end.col;
                else
                    ++sel_end.row, --sel_end.col;
            }
        }
        else
        {
            if (key == KEY_LEFT)
            {
                if (dir == DIR_NONE)
                    dir = DIR_LEFT;
                else if (dir != DIR_LEFT && dir != DIR_LEFT_UP && dir != DIR_LEFT_DOWN)
                    dir = DIR_CANCELED;
            }
            if (key == KEY_RIGHT)
            {
                if (dir == DIR_NONE)
                    dir = DIR_RIGHT;
                else if (dir != DIR_RIGHT && dir != DIR_RIGHT_UP && dir != DIR_RIGHT_DOWN)
                    dir = DIR_CANCELED;
            }
            if (key == KEY_UP)
            {
                if (dir == DIR_NONE)
                    dir = DIR_UP;
                else if (dir != DIR_UP && dir != DIR_LEFT_UP && dir != DIR_RIGHT_UP)
                    dir = DIR_CANCELED;
            }
            if (key == KEY_DOWN)
            {
                if (dir == DIR_NONE)
                    dir = DIR_DOWN;
                else if (dir != DIR_DOWN && dir != DIR_LEFT_DOWN && dir != DIR_RIGHT_DOWN)
                    dir = DIR_CANCELED;
            }
        }
        switch (dir)
        {
            case DIR_LEFT:
                ++sel_end.row, --sel_end.col;
                break;
            case DIR_RIGHT:
                --sel_end.row, ++sel_end.col;
                break;
            case DIR_UP:
                --sel_end.row, --sel_end.col;
                break;
            case DIR_DOWN:
                ++sel_end.row, ++sel_end.col;
                break;
            case DIR_LEFT_UP:
                --sel_end.col;
                break;
            case DIR_RIGHT_UP:
                --sel_end.row;
                break;
            case DIR_LEFT_DOWN:
                ++sel_end.row;
                break;
            case DIR_RIGHT_DOWN:
                ++sel_end.col;
        }
        if (!multiple_selection)
            sel_begin = sel_end;
        if (!modified.get_height())
            modified.delta = sel_end;
        manage_graphical_selection(DRAW);
    }
    if (!special_key_pressed && (isprint(key) || key == 8 || key == 13))
    {
        if (isprint(key))
            commands[0] += key;
        else if (key == 8)
        {
            int min_col = console_mode == CONS_GLOBAL ? get_prompt().length() : 0;
            if (static_cast<int>(commands[0].length()) > min_col)
                commands[0].pop_back();
        }
        else if (key == 13)
        {
            add_cli_text(commands[0]);
            if (console_mode == CONS_GLOBAL)
                execute(commands[0].substr(get_prompt().length(),
                                           commands[0].length() - get_prompt().length()));
            else
                last_command(last_args);
            if (console_mode == CONS_GLOBAL)
                commands[0] = get_prompt();
            else
                commands[0] = "";
        }
        if (console_mode == CONS_GLOBAL && key == '?')
        {
            add_cli_text(commands[0]);
            execute(commands[0].substr(get_prompt().length(),
                                       commands[0].length() - get_prompt().length()));
            commands[0].pop_back();
        }
        texts[0] = new Text(commands[0], Point2D(cli_offset.x, cli_offset.y + 93), Color(1, 1, 1),
                            GLUT_BITMAP_9_BY_15);
        Transform2D::loadIdentityMatrix();
        Transform2D::translateMatrix(cli_offset.x + commands[0].length() * 9, cli_offset.y + 91);
        Transform2D::applyTransform(cli_cursor);
    }
    if (key == KEY_ESC && multiple_selection)
    {
        multiple_selection = false;
        if (sel_end != sel_begin)
        {
            manage_graphical_selection(ERASE);
            sel_begin = sel_end;
            manage_graphical_selection(DRAW);
        }
    }
}

int main(int argc, char** argv)
{
    /* creare fereastra */
    DrawingWindow dw(argc, argv, 1024, 685, 200, 1, "Tema 2 EGC");
    /* se apeleaza functia init() - in care s-au adaugat obiecte */
    dw.init(argc, argv);
    /* se intra in bucla principala de desenare - care face posibila desenarea, animatia si
     * procesarea evenimentelor */
    dw.run();
    return 0;
}
