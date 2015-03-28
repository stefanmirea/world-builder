/* Stefan-Gabriel Mirea - 331CC */

/* Contine functiile necesare pentru executiile comenzilor. */
#include <utils.hpp>

/* variabile definite in global_vars.cpp */
extern vector<string> commands;
extern int game_width, edit_mode, console_mode;
extern bool world_modified, multiple_selection, fragment;
extern float edge;
extern InfiniteMatrix<Element> modified;
extern Cell sel_begin, sel_end;
extern vector<pair<Cell, int>> clipboard;
extern void (*last_command)(stringstream &);
extern stringstream last_args;

/* structura pentru asocieri (nume comanda, functie care se ocupa cu executia acelei comenzi) */
struct CmdNameFct {
    string name;
    void (*handler)(stringstream &);
};

/* Identifica dintr-o lista de comenzii data, (sub)comanda introdusa. Daca utilizatorul a cerul
 * ajutor (prin tastarea '?'), afiseaza sugestiile si intoarce CMD_HELP. Altfel, intoarce numarul de
 * ordine al comenzii sau CMD_NOT_FOUND daca nu s-a gasit.
 */
static int find_command(string &command, const CmdNameFct *command_info)
{
    bool found = false;

    if (command[command.length() - 1] == '?')
    {
        if (!command_info)
        {
            if (command.length() == 1)
                add_cli_text("<cr>");
            else
                add_cli_text("Unrecognized command.");
            return CMD_HELP;
        }
        string help;
        command.pop_back();
        int current_col = 0;
        for (int i = 0; command_info[i].name.length() > 0; ++i)
            if (command_info[i].name.find(command) == 0)
            {
                if (current_col > 0 && (current_col +
                    static_cast<int>(command_info[i].name.length())) * 9 > game_width)
                {
                    add_cli_text(help);
                    help.clear();
                    current_col = 0;
                }
                help += command_info[i].name + "  ";
                current_col += command_info[i].name.length() + 2;
                found = true;
            }
        if (!found)
            help = "Unrecognized command.";
        add_cli_text(help);
        return CMD_HELP;
    }

    if (!command_info)
        return CMD_NOT_FOUND;
    /* pozitia unei potentiale potriviri, fie ea numai de sufix */
    int cmd_index;
    /* cate comenzi au comanda introdusa ca prefix */
    int cmds_starting_with_cmd_name = 0;
    for (int i = 0; command_info[i].name.length() > 0; ++i)
    {
        if (command == command_info[i].name)
        {
            cmd_index = i;
            found = true;
            break;
        }
        if (command_info[i].name.find(command) == 0)
        {
            cmd_index = i;
            ++cmds_starting_with_cmd_name;
        }
    }
    if (found || cmds_starting_with_cmd_name == 1)
        return cmd_index;
    return CMD_NOT_FOUND;
}

/* Se apeleaza cand argumentul dat nu e identificat. Afiseaza un mesaj corespunzator si o lista de
 * sugestii. */
static void unrecognized_argument(const CmdNameFct *arguments, const string &wrong_arg)
{
    string help = "Unrecognized argument: \"" + wrong_arg + "\". Use ";
    int i;
    for (i = 0; arguments[i + 1].name.length(); ++i)
        help += "\"" + arguments[i].name + "\", ";
    help += "or \"" + arguments[i].name + "\".";
    add_cli_text(help);
}

/* verifica daca s-a transmis un argument in plus si apeleaza find_command pentru a trata cazul cand
 * se cere ajutor (argumentul e '?') */
static bool found_extra_argument(stringstream &args)
{
    string arg;
    args >> arg;
    if (args)
    {
        int signal = find_command(arg, NULL);
        if (signal == CMD_NOT_FOUND)
            add_cli_text("Extra argument unrecognized: " + arg + ".");
        return true;
    }
    return false;
}

static void handler_clear(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    int lines_num = commands.size();
    commands.erase(commands.begin() + 1, commands.end());
    DrawingWindow::texts.erase(DrawingWindow::texts.begin() + 1,
                               DrawingWindow::texts.begin() + lines_num);
}

static void handler_zoom_in(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (edge * 1.3f <= EDGE_MAX)
    {
        edge *= 1.3f;
        draw_visible_objects(true);
    }
    else
        add_cli_text("Cannot zoom beyond this level.");
}

static void handler_zoom_out(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    edge /= 1.3f;
    draw_visible_objects(true);
}

static void handler_zoom_reset(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    edge = EDGE_DEFAULT;
    draw_visible_objects(true);
}

static void handler_zoom(stringstream &args)
{
    CmdNameFct zoom_command_info[] = {
        {"in",    handler_zoom_in},
        {"out",   handler_zoom_out},
        {"reset", handler_zoom_reset},
        {"",      NULL}
    };
    string arg;
    args >> arg;
    if (!args)
        handler_zoom_in(args);
    else
    {
        int cmd_index = find_command(arg, zoom_command_info);
        if (cmd_index == CMD_HELP)
            return;
        if (cmd_index >= 0)
            zoom_command_info[cmd_index].handler(args);
        else
            unrecognized_argument(zoom_command_info, arg);
    }
}

static void handler_fragment_on(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    fragment = true;
}

static void handler_fragment_off(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    fragment = false;
}

static void handler_fragment(stringstream &args)
{
    CmdNameFct fragment_command_info[] = {
        {"on",  handler_fragment_on},
        {"off", handler_fragment_off},
        {"",    NULL}
    };
    string arg;
    args >> arg;
    if (!args)
        handler_fragment_on(args);
    else
    {
        int cmd_index = find_command(arg, fragment_command_info);
        if (cmd_index == CMD_HELP)
            return;
        if (cmd_index >= 0)
            fragment_command_info[cmd_index].handler(args);
        else
            unrecognized_argument(fragment_command_info, arg);
    }
}

static void handler_selection_start(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (!multiple_selection)
        multiple_selection = true;
    else if (sel_end != sel_begin)
    {
        manage_graphical_selection(ERASE);
        sel_begin = sel_end;
        manage_graphical_selection(DRAW);
    }
}

static void handler_selection_exit(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (!multiple_selection)
        add_cli_text("There is no active selection.");
    else
        cancel_multiple_selection();
}

static void handler_selection(stringstream &args)
{
    CmdNameFct selection_command_info[] = {
        {"start", handler_selection_start},
        {"exit",  handler_selection_exit},
        {"",      NULL}
    };
    string arg;
    args >> arg;
    if (!args)
        handler_selection_start(args);
    else
    {
        int cmd_index = find_command(arg, selection_command_info);
        if (cmd_index == CMD_HELP)
            return;
        if (cmd_index >= 0)
            selection_command_info[cmd_index].handler(args);
        else
            unrecognized_argument(selection_command_info, arg);
    }
}

static void handler_edit_terrain(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    edit_mode = EDIT_TERRAIN;
}

static void handler_edit_square(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    edit_mode = EDIT_SQUARE;
}

static void handler_edit_houses(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    edit_mode = EDIT_HOUSES;
}

static void handler_edit_roads(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    edit_mode = EDIT_ROADS;
}

static void handler_edit(stringstream &args)
{
    CmdNameFct edit_command_info[] = {
        {"terrain", handler_edit_terrain},
        {"square",  handler_edit_square},
        {"houses",  handler_edit_houses},
        {"roads",   handler_edit_roads},
        {"",        NULL}
    };
    string arg;
    args >> arg;
    if (!args)
        add_cli_text(
            "Please provide an argument (\"terrain\", \"square\", \"houses\" or \"roads\").");
    else
    {
        int cmd_index = find_command(arg, edit_command_info);
        if (cmd_index == CMD_HELP)
            return;
        if (cmd_index >= 0)
        {
            edit_command_info[cmd_index].handler(args);
            draw_visible_objects(false);
        }
        else
            unrecognized_argument(edit_command_info, arg);
    }
}

static void handler_water(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (edit_mode != EDIT_TERRAIN)
    {
        add_cli_text("You have to be in the Terrain mode to perform this action.");
        return;
    }
    bool failures = false, successes = false;
    Cell first(min(sel_begin.row, sel_end.row), min(sel_begin.col, sel_end.col));
    Cell last(max(sel_begin.row, sel_end.row), max(sel_begin.col, sel_end.col));
    for (int i = first.row; i <= last.row; ++i)
        for (int j = first.col; j <= last.col; ++j)
        {
            Element elem = modified.query(Cell(i, j));
            if ((elem.terrain & 0x3C) == 0x3C && elem.object == EL_EMPTY)
            {
                elem.terrain = 0x3C + GR_WATER;
                modified.update(Cell(i, j), elem);
                erase_element(Cell(i, j), elem.altitude, SEL_NONE);
                draw_element(Cell(i, j), elem.altitude, SEL_NONE);
                successes = true;
            }
            else
                failures = true;
        }
    if (failures)
    {
        add_cli_text("One or more cells could not be modified.");
        add_cli_text("The terrain must be straight and the cell must not contain any object.");
    }
    if (successes)
    {
        update_front_water(first, last);
        if (multiple_selection)
            handler_selection_exit(args);
        world_modified = true;
    }
}

static void handler_grass(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (edit_mode != EDIT_TERRAIN)
    {
        add_cli_text("You have to be in the Terrain mode to perform this action.");
        return;
    }
    Cell first(min(sel_begin.row, sel_end.row), min(sel_begin.col, sel_end.col));
    Cell last(max(sel_begin.row, sel_end.row), max(sel_begin.col, sel_end.col));
    for (int i = first.row; i <= last.row; ++i)
        for (int j = first.col; j <= last.col; ++j)
        {
            Element elem = modified.query(Cell(i, j));
            elem.terrain = (elem.terrain & 0xFC) + GR_GRASS;
            modified.update(Cell(i, j), elem);
            erase_element(Cell(i, j), elem.altitude, SEL_NONE);
            draw_element(Cell(i, j), elem.altitude, SEL_NONE);
        }
    update_front_water(first, last);
    if (multiple_selection)
        handler_selection_exit(args);
    world_modified = true;
}

static void handler_delete(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    Cell first(min(sel_begin.row, sel_end.row), min(sel_begin.col, sel_end.col));
    Cell last(max(sel_begin.row, sel_end.row), max(sel_begin.col, sel_end.col));

    bool successes = false, failures = false;
    if (edit_mode == EDIT_TERRAIN)
    {
        for (int i = first.row; i <= last.row; ++i)
            for (int j = first.col; j <= last.col; ++j)
            {
                Element elem = modified.query(Cell(i, j));
                if ((elem.terrain & 3) == GR_WATER || (elem.terrain & 3) == GR_GRASS)
                {
                    elem.terrain = (elem.terrain & 0xFC) + GR_EMPTY;
                    modified.update(Cell(i, j), elem);
                    erase_element(Cell(i, j), elem.altitude, SEL_NONE);
                    draw_element(Cell(i, j), elem.altitude, SEL_NONE);
                    successes = true;
                }
            }
        if (!successes)
        {
            add_cli_text("Nothing to delete.");
            return;
        }
        update_front_water(first, last);
        world_modified = true;
    }
    else if (edit_mode == EDIT_SQUARE)
    {
        for (int i = first.row; i <= last.row; ++i)
            for (int j = first.col; j <= last.col; ++j)
            {
                Element elem = modified.query(Cell(i, j));
                if (elem.object == EL_SQUARE)
                {
                    InfiniteMatrix<Element> aux(modified);
                    elem.object = EL_EMPTY;
                    modified.update(Cell(i, j), elem);
                    update_square_coverage(false);
                    bool able_to_delete = true;
                    for (int row = modified.delta.row;
                         row < modified.delta.row + modified.get_height(); ++row)
                    {
                        for (int col = modified.delta.col;
                             col < modified.delta.col + modified.get_width(); ++col)
                        {
                            Element el = modified.query(Cell(row, col));
                            if (el.object == EL_HOUSE && !el.close_to_square())
                            {
                                able_to_delete = false;
                                break;
                            }
                        }
                        if (!able_to_delete)
                            break;
                    }
                    modified = aux;
                    if (!able_to_delete)
                    {
                        failures = true;
                        continue;
                    }
                    successes = true;
                    modified.update(Cell(i, j), elem);
                    erase_element(Cell(i, j), elem.altitude, SEL_NONE);
                    draw_element(Cell(i, j), elem.altitude, SEL_NONE);

                    /* actualizez drumurile din jur */
                    int drow[] = {-1, -1, -1,  0,  1,  1,  1,  0};
                    int dcol[] = {-1,  0,  1,  1,  1,  0, -1, -1};
                    for (int k = 0; k < 8; ++k)
                    {
                        Element neighbor = modified.query(Cell(i + drow[k], j + dcol[k]));
                        if (neighbor.object == EL_ROAD || neighbor.object == EL_HOUSE)
                        {
                            erase_element(Cell(i + drow[k], j + dcol[k]), elem.altitude, SEL_NONE);
                            draw_element(Cell(i + drow[k], j + dcol[k]), elem.altitude, SEL_NONE);
                        }
                    }
                }
            }
        if (!failures && !successes)
        {
            add_cli_text("No square to delete.");
            return;
        }
        if (failures)
        {
            add_cli_text("One or more squares could not be deleted.");
            add_cli_text("Some houses would get away from square coverage.");
        }
        if (!successes)
            return;
        update_square_coverage(true);
        update_square_connectivity();
        world_modified = true;
    }
    else if (edit_mode == EDIT_HOUSES)
    {
        for (int i = first.row; i <= last.row; ++i)
            for (int j = first.col; j <= last.col; ++j)
            {
                Element elem = modified.query(Cell(i, j));
                if (elem.object == EL_HOUSE)
                {
                    successes = true;
                    int old_floors_num = elem.house_floors;
                    if (elem.house_floors == 0)
                        elem.object = EL_EMPTY;
                    else
                        --elem.house_floors;
                    modified.update(Cell(i, j), elem);
                    if (elem.object == EL_EMPTY)
                        redraw_roads_around(Cell(i, j));
                    erase_element(Cell(i, j), elem.altitude + 1 + old_floors_num / 2, SEL_NONE);
                    draw_element(Cell(i, j), elem.altitude + 1 + old_floors_num / 2, SEL_NONE);
                    if (old_floors_num % 2 == 1)
                        erase_element(Cell(i, j), elem.altitude + 2 + old_floors_num / 2, SEL_NONE);
                    /* actualizarea fuziunii elementelor vecine */
                    int drow[] = {-1, -1, -1,  0,  1,  1,  1,  0};
                    int dcol[] = {-1,  0,  1,  1,  1,  0, -1, -1};
                    for (int k = 0; k < 8; ++k)
                    {
                        Element neighbor = modified.query(Cell(i + drow[k], j + dcol[k]));
                        if (neighbor.object == EL_HOUSE && neighbor.house_floors >= old_floors_num)
                        {
                            erase_element(Cell(i + drow[k], j + dcol[k]),
                                          elem.altitude + 1 + (old_floors_num + 1) / 2, SEL_NONE);
                            draw_element(Cell(i + drow[k], j + dcol[k]),
                                         elem.altitude + 1 + (old_floors_num + 1) / 2, SEL_NONE);
                        }
                    }
                }
            }
        if (!successes)
        {
            add_cli_text("No house to delete.");
            return;
        }
        update_square_connectivity();
        world_modified = true;
    }
    else if (edit_mode == EDIT_ROADS)
    {
        for (int i = first.row; i <= last.row; ++i)
            for (int j = first.col; j <= last.col; ++j)
            {
                Element elem = modified.query(Cell(i, j));
                if (elem.object == EL_ROAD)
                {
                    successes = true;
                    elem.object = EL_EMPTY;
                    modified.update(Cell(i, j), elem);
                    erase_element(Cell(i, j), elem.altitude, SEL_NONE);
                    draw_element(Cell(i, j), elem.altitude, SEL_NONE);

                    /* reactualizez drumurile si casele din jur */
                    int drow[] = {-1, -1, -1,  0,  1,  1,  1,  0};
                    int dcol[] = {-1,  0,  1,  1,  1,  0, -1, -1};
                    for (int k = 0; k < 8; ++k)
                    {
                        Element neighbor = modified.query(Cell(i + drow[k], j + dcol[k]));
                        if (neighbor.object == EL_ROAD || neighbor.object == EL_HOUSE)
                        {
                            erase_element(Cell(i + drow[k], j + dcol[k]), elem.altitude, SEL_NONE);
                            draw_element(Cell(i + drow[k], j + dcol[k]), elem.altitude, SEL_NONE);
                        }
                    }
                }
            }
        if (!successes)
        {
            add_cli_text("No road to delete.");
            return;
        }
        update_square_connectivity();
        world_modified = true;
    }
    if (multiple_selection)
        handler_selection_exit(args);
}

static void handler_up(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    change_relief(DIR_UP);
}

static void handler_down(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    change_relief(DIR_DOWN);
}

static void handler_add(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (edit_mode == EDIT_TERRAIN)
    {
        add_cli_text("You have to be in the Square, Houses or Roads mode to perform this action.");
        return;
    }

    Cell first(min(sel_begin.row, sel_end.row), min(sel_begin.col, sel_end.col));
    Cell last(max(sel_begin.row, sel_end.row), max(sel_begin.col, sel_end.col));

    bool failures = false, successes = false;
    if (edit_mode == EDIT_SQUARE)
    {
        for (int i = first.row; i <= last.row; ++i)
            for (int j = first.col; j <= last.col; ++j)
            {
                Element elem = modified.query(Cell(i, j));
                if ((elem.terrain & 0x3C) == 0x3C && (elem.terrain & 3) != GR_WATER &&
                    elem.object != EL_HOUSE && elem.object != EL_ROAD)
                {
                    elem.object = EL_SQUARE;
                    modified.update(Cell(i, j), elem);
                    erase_element(Cell(i, j), elem.altitude, SEL_NONE);
                    draw_element(Cell(i, j), elem.altitude, SEL_NONE);
                    successes = true;
                }
                else
                    failures = true;
            }
        if (failures)
        {
            add_cli_text("One or more cells could not be modified.");
            add_cli_text("The terrain must be straight and the cell must not contain any object.");
        }
        if (!successes)
            return;
        /* actualizez drumurile alaturate, care trebuie sa se uneasca de piata */
        int i = first.row - 1;
        int j = first.col - 1;
        int dir = DIR_RIGHT;
        do {
            Element neighbor = modified.query(Cell(i, j));
            if (neighbor.object == EL_ROAD || neighbor.object == EL_HOUSE)
            {
                erase_element(Cell(i, j), neighbor.altitude, SEL_NONE);
                draw_element(Cell(i, j), neighbor.altitude, SEL_NONE);
            }
            if (dir == DIR_RIGHT && j == last.col + 1)
                dir = DIR_DOWN;
            else if (dir == DIR_DOWN && i == last.row + 1)
                dir = DIR_LEFT;
            else if (dir == DIR_LEFT && j == first.col - 1)
                dir = DIR_UP;
            switch (dir)
            {
                case DIR_RIGHT:
                    ++j;
                    break;
                case DIR_DOWN:
                    ++i;
                    break;
                case DIR_LEFT:
                    --j;
                    break;
                default:
                    --i;
            }
        } while (i != first.row - 1 || j != first.col - 1);
        update_square_coverage(true);
        update_square_connectivity();
        world_modified = true;
    }
    else if (edit_mode == EDIT_HOUSES)
    {
        for (int i = first.row; i <= last.row; ++i)
            for (int j = first.col; j <= last.col; ++j)
            {
                Element elem = modified.query(Cell(i, j));
                if (elem.object != EL_HOUSE)
                    if ((elem.terrain & 0x3C) == 0x3C && (elem.terrain & 3) != GR_WATER &&
                        elem.object == EL_EMPTY && elem.close_to_square())
                    {
                        elem.object = EL_HOUSE;
                        elem.house_floors = 0;
                    }
                    else
                    {
                        failures = true;
                        continue;
                    }
                else
                    ++elem.house_floors;
                successes = true;
                modified.update(Cell(i, j), elem);
                if (elem.house_floors == 0)
                {
                    /* desenez drumrile adiacente */
                    erase_element(Cell(i, j), elem.altitude, SEL_NONE);
                    draw_element(Cell(i, j), elem.altitude, SEL_NONE);
                }
                erase_element(Cell(i, j), elem.altitude + 1 + elem.house_floors / 2, SEL_NONE);
                draw_element(Cell(i, j), elem.altitude + 1 + elem.house_floors / 2, SEL_NONE);
                if (elem.house_floors % 2 == 1)
                    draw_element(Cell(i, j), elem.altitude + 2 + elem.house_floors / 2, SEL_NONE);
                /* fuziunea elementelor vecine cu cel nou */
                int drow[] = {-1, -1, -1,  0,  1,  1,  1,  0};
                int dcol[] = {-1,  0,  1,  1,  1,  0, -1, -1};
                for (int k = 0; k < 8; ++k)
                {
                    Element neighbor = modified.query(Cell(i + drow[k], j + dcol[k]));
                    if (neighbor.object == EL_HOUSE && neighbor.house_floors >= elem.house_floors)
                    {
                        erase_element(Cell(i + drow[k], j + dcol[k]),
                                      elem.altitude + 1 + (elem.house_floors + 1) / 2, SEL_NONE);
                        draw_element(Cell(i + drow[k], j + dcol[k]),
                                     elem.altitude + 1 + (elem.house_floors + 1) / 2, SEL_NONE);
                    }
                }
            }
        if (failures)
        {
            add_cli_text("One ore more houses could not be created.");
            add_cli_text("The terrain must be straight and the cell must be empty and inside square"
                         " coverage.");
        }
        if (!successes)
            return;
        /* actualizez drumurile de pe marginea si din jurul selectiei */
        int i = first.row - 1;
        int j = first.col - 1;
        int dir = DIR_RIGHT;
        int steps = (last.row - first.row + 3) * (last.col - first.col + 3);
        if (last.row > first.row && last.col > first.col)
            steps -= (last.row - first.row - 1) * (last.col - first.col - 1);
        while (steps--)
        {
            Element neighbor = modified.query(Cell(i, j));
            if (neighbor.object == EL_ROAD || neighbor.object == EL_SQUARE)
            {
                erase_element(Cell(i, j), neighbor.altitude, SEL_NONE);
                draw_element(Cell(i, j), neighbor.altitude, SEL_NONE);
            }
            if (dir == DIR_RIGHT && i + j == first.row + last.col)
                dir = DIR_DOWN;
            else if (dir == DIR_DOWN && i - j == last.row - last.col)
                dir = DIR_LEFT;
            else if (dir == DIR_LEFT && i + j == last.row + first.col)
                dir = DIR_UP;
            else if (dir == DIR_UP && i - j == first.row - first.col)
                dir = DIR_RIGHT;
            switch (dir)
            {
                case DIR_RIGHT:
                    ++j;
                    break;
                case DIR_DOWN:
                    ++i;
                    break;
                case DIR_LEFT:
                    --j;
                    break;
                default:
                    --i;
            }
        }
        update_square_connectivity();
        world_modified = true;
    }
    else if (edit_mode == EDIT_ROADS)
    {
        for (int i = first.row; i <= last.row; ++i)
            for (int j = first.col; j <= last.col; ++j)
            {
                Element elem = modified.query(Cell(i, j));
                if ((elem.terrain & 3) != GR_WATER && elem.object != EL_SQUARE &&
                    elem.object != EL_HOUSE)
                {
                    successes = true;
                    elem.object = EL_ROAD;
                    modified.update(Cell(i, j), elem);
                    erase_element(Cell(i, j), elem.altitude, SEL_NONE);
                    draw_element(Cell(i, j), elem.altitude, SEL_NONE);

                    /* actualizez drumurile si casele alaturate */
                    int drow[] = {-1, -1, -1, 0, 1, 1,  1,  0};
                    int dcol[] = {-1,  0,  1, 1, 1, 0, -1, -1};
                    for (int k = 0; k < 8; ++k)
                    {
                        Element neighbor = modified.query(Cell(i + drow[k], j + dcol[k]));
                        if (neighbor.object == EL_ROAD || neighbor.object == EL_HOUSE)
                        {
                            erase_element(Cell(i + drow[k], j + dcol[k]), neighbor.altitude,
                                          SEL_NONE);
                            draw_element(Cell(i + drow[k], j + dcol[k]), neighbor.altitude,
                                         SEL_NONE);
                        }
                    }
                }
                else
                    failures = true;
            }
        if (failures)
        {
            add_cli_text("One ore more roads could not be created.");
            add_cli_text("The cell must be empty.");
        }
        if (!successes)
            return;
        update_square_connectivity();
        world_modified = true;
    }
    if (multiple_selection)
        handler_selection_exit(args);
}

static void handler_rotate_clockwise(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (edit_mode != EDIT_HOUSES)
    {
        add_cli_text("You have to be in the Houses mode to perform this action.");
        return;
    }

    rotate_houses(DIR_CLOCKWISE);
}

static void handler_rotate_counterclockwise(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (edit_mode != EDIT_HOUSES)
    {
        add_cli_text("You have to be in the Houses mode to perform this action.");
        return;
    }

    rotate_houses(DIR_COUNTERCLOCKWISE);
}

static void handler_rotate_view_clockwise(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    rotate_camera(DIR_CLOCKWISE);
}

static void handler_rotate_view_counterclockwise(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    rotate_camera(DIR_COUNTERCLOCKWISE);
}

static void handler_rotate_view(stringstream &args)
{
    CmdNameFct rotate_view_command_info[] = {
        {"clockwise",        handler_rotate_view_clockwise},
        {"counterclockwise", handler_rotate_view_counterclockwise},
        {"",                 NULL}
    };
    string arg;
    args >> arg;
    if (!args)
        handler_rotate_view_clockwise(args);
    else
    {
        int cmd_index = find_command(arg, rotate_view_command_info);
        if (cmd_index == CMD_HELP)
            return;
        if (cmd_index >= 0)
            rotate_view_command_info[cmd_index].handler(args);
        else
            unrecognized_argument(rotate_view_command_info, arg);
    }
}

static void handler_rotate(stringstream &args)
{
    CmdNameFct rotate_command_info[] = {
        {"clockwise",        handler_rotate_clockwise},
        {"counterclockwise", handler_rotate_counterclockwise},
        {"view",             handler_rotate_view},
        {"",                 NULL}
    };
    string arg;
    args >> arg;
    if (!args)
        handler_rotate_clockwise(args);
    else
    {
        int cmd_index = find_command(arg, rotate_command_info);
        if (cmd_index == CMD_HELP)
            return;
        if (cmd_index >= 0)
            rotate_command_info[cmd_index].handler(args);
        else
            unrecognized_argument(rotate_command_info, arg);
    }
}

static void handler_cut(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    copy_houses(false);
}

static void handler_copy(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    copy_houses(true);
}

static void handler_paste(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (!clipboard.size())
    {
        add_cli_text("Nothing to paste.");
        return;
    }

    /* verific daca se poate realiza lipirea */
    for (int i = 0; i < static_cast<int>(clipboard.size()); ++i)
    {
        Element new_place = modified.query(Cell(sel_end.row + clipboard[i].first.row,
                                                sel_end.col + clipboard[i].first.col));
        if ((new_place.terrain & 0x3C) != 0x3C)
        {
            add_cli_text("Paste failure: one or more houses would stay on a sloping terrain.");
            return;
        }
        if ((new_place.terrain & 3) == GR_WATER || new_place.object != EL_EMPTY)
        {
            add_cli_text("Paste failure: there is not enough room.");
            return;
        }
        if (!new_place.close_to_square())
        {
            add_cli_text("Paste failure: one or more houses would move away from square coverage.");
            return;
        }
    }

    for (int i = 0; i < static_cast<int>(clipboard.size()); ++i)
    {
        Cell cell(sel_end.row + clipboard[i].first.row, sel_end.col + clipboard[i].first.col);

        Element new_elem = modified.query(cell);
        new_elem.object = EL_HOUSE;
        new_elem.house_floors = clipboard[i].second;
        modified.update(cell, new_elem);

        int h_max = new_elem.house_floors / 2 + 3;
        for (int h = 1; h <= h_max; ++h)
            draw_element(cell, new_elem.altitude + h, SEL_NONE);

        redraw_houses_around(cell);
        redraw_roads_around(cell);
    }

    update_square_connectivity();
    world_modified = true;
}

static void handler_new(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (world_modified)
    {
        if (console_mode == CONS_GLOBAL)
        {
            console_mode = CONS_LOCAL;
            last_command = handler_new;
            last_args.clear();
            last_args << args.rdbuf();
        }
        int action = save_dialog(handler_new, args);
        if (action == ACT_PROCEED || action == ACT_ABORT)
            console_mode = CONS_GLOBAL;
        if (action == ACT_WAIT || action == ACT_ABORT)
            return;
    }

    create_new_game();
}

static void handler_save(stringstream &args)
{
    static string save_filename;
    static bool first_call = true;
    string arg;
    args >> arg;

    if (first_call && args)
    {
        string command = args.str();
        save_filename = command.substr(command.find(' ', command.find('s')) + 1);
        if (save_filename[save_filename.length() - 1] == '?')
        {
            save_filename = "";
            add_cli_text("<filename>");
            return;
        }
        first_call = false;
    }
    save_game(handler_save, args, save_filename);
    first_call = true;
}

static void handler_load(stringstream &args)
{
    string arg;
    args >> arg;
    static string filename_to_open;
    static bool first_call = true;

    if (first_call)
    {
        if (!args)
        {
            add_cli_text("Please provide the file name as an argument.");
            return;
        }
        string command = args.str();
        /* comanda poate sa se fi dat ca 'load' sau 'open' */
        int command_begin_pos = min(command.find('l'), command.find('o'));
        filename_to_open = command.substr(command.find(' ', command_begin_pos) + 1);
        if (filename_to_open[filename_to_open.length() - 1] == '?')
        {
            filename_to_open = "";
            add_cli_text("<filename>");
            return;
        }
        first_call = false;
    }

    if (world_modified)
    {
        if (console_mode == CONS_GLOBAL)
        {
            console_mode = CONS_LOCAL;
            last_command = handler_load;
            last_args.clear();
            last_args << args.rdbuf();
        }
        int action = save_dialog(handler_load, args);
        if (action == ACT_ABORT)
        {
            filename_to_open.clear();
            first_call = true;
        }
        if (action == ACT_PROCEED || action == ACT_ABORT)
            console_mode = CONS_GLOBAL;
        if (action == ACT_WAIT || action == ACT_ABORT)
            return;
    }

    load_game(filename_to_open);
    filename_to_open.clear();
    first_call = true;
}

static void handler_exit(stringstream &args)
{
    if (found_extra_argument(args))
        return;

    if (world_modified)
    {
        if (console_mode == CONS_GLOBAL)
        {
            console_mode = CONS_LOCAL;
            last_command = handler_exit;
            last_args.clear();
            last_args << args.rdbuf();
        }
        int action = save_dialog(handler_exit, args);
        if (action == ACT_ABORT)
            console_mode = CONS_GLOBAL;
        if (action == ACT_WAIT || action == ACT_ABORT)
            return;
    }

    exit(0);
}

/* vector continand asocierile (nume comanda, functie care se ocupa cu executia acelei comenzi)
 * pentru comenzile "top-level" */
CmdNameFct command_info[] = {
    {"clear",     handler_clear},
    {"zoom",      handler_zoom},
    {"fragment",  handler_fragment},
    {"selection", handler_selection},
    {"edit",      handler_edit},
    {"water",     handler_water},
    {"grass",     handler_grass},
    {"delete",    handler_delete},
    {"up",        handler_up},
    {"down",      handler_down},
    {"add",       handler_add},
    {"rotate",    handler_rotate},
    {"cut",       handler_cut},
    {"copy",      handler_copy},
    {"paste",     handler_paste},
    {"new",       handler_new},
    {"save",      handler_save},
    {"load",      handler_load},
    {"open",      handler_load},
    {"exit",      handler_exit},
    {"quit",      handler_exit},
    {"",          NULL}
};

/* interpreteaza si executa comanda introdusa */
void execute(const string &full_command)
{
    stringstream iss(full_command);
    string command_name;
    iss >> command_name;
    if (!iss)
        return;
    int cmd_index = find_command(command_name, command_info);
    if (cmd_index == CMD_HELP)
        return;
    if (cmd_index >= 0)
        command_info[cmd_index].handler(iss);
    else
        add_cli_text("Unrecognized command: \"" + command_name + "\".");
}
