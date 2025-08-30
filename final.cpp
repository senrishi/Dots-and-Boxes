#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>

using namespace std;

const int A = 10;
const int B = 1;
int rows = 0, columns = 0;
int bot_score = 0;
int opp_score = 0;

enum State { 
    NO_OWNER = -1,
    HUMAN = 0, 
    AI = 1 
};
std::vector<std::vector<bool>> horizontal_lines;
std::vector<std::vector<bool>> vertical_lines;
std::vector<std::vector<int>> box_owners;

enum LineType { 
    HORIZONTAL, 
    VERTICAL 
};
struct Move { 
    int r, c; 
    LineType type; 
};
State turn = HUMAN;

void default_arr();
std::vector<Move> move_gen();
double eval_board();
int apply_move(const Move& move);
void undo_move(const Move& move);
bool game_state();
double minimax(int depth, double alpha, double beta, bool maxim);
Move winning_move();
string translate(const Move& move);
void parse_turn_input();
int count_sides(int r, int c);

void default_arr()
{
    horizontal_lines.clear();
    vertical_lines.clear();
    box_owners.clear();
    horizontal_lines.resize(rows, std::vector<bool>(columns - 1, false));
    vertical_lines.resize(rows - 1, std::vector<bool>(columns, false));
    box_owners.resize(rows - 1, std::vector<int>(columns - 1, NO_OWNER));
}

std::vector<Move> move_gen()
{
    std::vector<Move> avlbl_moves;
    avlbl_moves.reserve(2 * rows * columns);
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < columns - 1; ++j)
        {
            if (!horizontal_lines[i][j])
            {
                avlbl_moves.push_back({i, j, HORIZONTAL});
            }
        }
    }
    for (int i = 0; i < rows - 1; ++i)
    {
        for (int j = 0; j < columns; ++j)
        {
            if (!vertical_lines[i][j])
            {
                avlbl_moves.push_back({i, j, VERTICAL});
            }
        }
    }
    return avlbl_moves;
}

double eval_board()
{
    double p1 = A * (bot_score - opp_score);
    int three_sides = 0;
    for (int r = 0; r < rows - 1; r++)
    {
        for (int c = 0; c < columns - 1; c++)
        {
            if (count_sides(r, c) == 3)
            {
                three_sides++;
            }
        }
    }
    double p2 = B * three_sides;
    return p1 - p2;
}

int apply_move(const Move& move)
{
    int boxed = 0;
    if (move.type == HORIZONTAL)
    {
        horizontal_lines[move.r][move.c] = true;
        if (move.r < rows - 1 && horizontal_lines[move.r + 1][move.c] && vertical_lines[move.r][move.c] && vertical_lines[move.r][move.c + 1])
        {
            box_owners[move.r][move.c] = turn;
            boxed++;
        }
        if (move.r > 0 && horizontal_lines[move.r - 1][move.c] && vertical_lines[move.r - 1][move.c] && vertical_lines[move.r - 1][move.c + 1])
        {
            box_owners[move.r - 1][move.c] = turn;
            boxed++;
        }
    }
    else
    {
        vertical_lines[move.r][move.c] = true;
        if (move.c < columns - 1 && horizontal_lines[move.r][move.c] && horizontal_lines[move.r + 1][move.c] && vertical_lines[move.r][move.c + 1])
        {
            box_owners[move.r][move.c] = turn;
            boxed++;
        }
        if (move.c > 0 && horizontal_lines[move.r][move.c - 1] && horizontal_lines[move.r + 1][move.c - 1] && vertical_lines[move.r][move.c - 1])
        {
            box_owners[move.r][move.c - 1] = turn;
            boxed++;
        }
    }
    if (boxed > 0)
    {
        if (turn == AI)
        {
            bot_score += boxed;
        }
        else
        {
            opp_score += boxed;
        }
    }
    return boxed;
}

void undo_move(const Move& move)
{
    int boxed_undone = 0;
    if (move.type == HORIZONTAL)
    {
        if (move.r < rows - 1 && box_owners[move.r][move.c] != NO_OWNER && horizontal_lines[move.r + 1][move.c] && vertical_lines[move.r][move.c] && vertical_lines[move.r][move.c + 1])
        {
            box_owners[move.r][move.c] = NO_OWNER;
            boxed_undone++;
        }
        if (move.r > 0 && box_owners[move.r - 1][move.c] != NO_OWNER && horizontal_lines[move.r - 1][move.c] && vertical_lines[move.r - 1][move.c] && vertical_lines[move.r - 1][move.c + 1])
        {
            box_owners[move.r - 1][move.c] = NO_OWNER;
            boxed_undone++;
        }
    }
    else
    {
        if (move.c < columns - 1 && box_owners[move.r][move.c] != NO_OWNER && horizontal_lines[move.r][move.c] && horizontal_lines[move.r + 1][move.c] && vertical_lines[move.r][move.c + 1])
        {
            box_owners[move.r][move.c] = NO_OWNER;
            boxed_undone++;
        }
        if (move.c > 0 && box_owners[move.r][move.c - 1] != NO_OWNER && horizontal_lines[move.r][move.c - 1] && horizontal_lines[move.r + 1][move.c - 1] && vertical_lines[move.r][move.c - 1])
        {
            box_owners[move.r][move.c - 1] = NO_OWNER;
            boxed_undone++;
        }
    }
    if (boxed_undone > 0)
    {
        if (turn == AI)
        {
            bot_score -= boxed_undone;
        }
        else
        {
            opp_score -= boxed_undone;
        }
    }
    if (move.type == HORIZONTAL)
    {
        horizontal_lines[move.r][move.c] = false;
    }
    else
    {
        vertical_lines[move.r][move.c] = false;
    }
}

bool game_state()
{
    return (bot_score + opp_score == (rows - 1) * (columns - 1));
}

double minimax(int depth, double alpha, double beta, bool maxim)
{
    if (depth == 0 || game_state())
    {
        return eval_board();
    }
    if (maxim)
    {
        double maxEval = -numeric_limits<double>::infinity();
        State original_turn = turn;
        turn = AI;
        for (const Move& move : move_gen())
        {
            int boxed = apply_move(move);
            double eval = (boxed > 0) ? minimax(depth - 1, alpha, beta, true) : minimax(depth - 1, alpha, beta, false);
            undo_move(move);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
            {
                break;
            }
        }
        turn = original_turn;
        return maxEval;
    }
    else
    {
        double minEval = numeric_limits<double>::infinity();
        State original_turn = turn;
        turn = HUMAN;
        for (const Move& move : move_gen())
        {
            int boxed = apply_move(move);
            double eval = (boxed > 0) ? minimax(depth - 1, alpha, beta, false) : minimax(depth - 1, alpha, beta, true);
            undo_move(move);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
            {
                break;
            }
        }
        turn = original_turn;
        return minEval;
    }
}

int count_sides(int r, int c)
{
    if (r < 0 || r >= rows - 1 || c < 0 || c >= columns - 1)
    {
        return 0;
    }
    int side_count = 0;
    if (horizontal_lines[r][c]) side_count++;
    if (horizontal_lines[r + 1][c]) side_count++;
    if (vertical_lines[r][c]) side_count++;
    if (vertical_lines[r][c + 1]) side_count++;
    return side_count;
}

Move winning_move()
{
    turn = AI;
    std::vector<Move> available_moves = move_gen();
    if (available_moves.empty())
    {
        return {};
    }
    
    for (const Move& move : available_moves)
    {
        int temp_bot_score = bot_score;
        int boxed = apply_move(move);
        undo_move(move);
        bot_score = temp_bot_score;
        if (boxed > 0)
        {
            return move;
        }
    }

    std::vector<Move> safe_moves;
    for (const Move& move : available_moves)
    {
        apply_move(move);
        bool creates_third_side = false;
        if (move.type == HORIZONTAL)
        {
            if (move.r < rows - 1 && count_sides(move.r, move.c) == 3)
            {
                creates_third_side = true;
            }
            if (!creates_third_side && move.r > 0 && count_sides(move.r - 1, move.c) == 3)
            {
                creates_third_side = true;
            }
        }
        else
        {
            if (move.c < columns - 1 && count_sides(move.r, move.c) == 3)
            {
                creates_third_side = true;
            }
            if (!creates_third_side && move.c > 0 && count_sides(move.r, move.c - 1) == 3)
            {
                creates_third_side = true;
            }
        }
        undo_move(move);
        if (!creates_third_side)
        {
            safe_moves.push_back(move);
        }
    }
    if (!safe_moves.empty())
    {
        return safe_moves[0];
    }
    
    int depth = 3;
    Move best_move = available_moves[0];
    double best_score = -numeric_limits<double>::infinity();
    double alpha = -numeric_limits<double>::infinity();
    double beta = numeric_limits<double>::infinity();

    for (const Move& move : available_moves)
    {
        int boxed = apply_move(move);
        double eval = (boxed > 0) ? minimax(depth - 1, alpha, beta, true) : minimax(depth - 1, alpha, beta, false);
        undo_move(move);
        if (eval > best_score)
        {
            best_score = eval;
            best_move = move;
        }
        alpha = std::max(alpha, eval);
    }
    return best_move;
}

string translate(const Move& move)
{
    const int total_box_rows = rows - 1;
    string box_name;
    string side_char;

    if (move.type == HORIZONTAL)
    {
        if (move.r < total_box_rows)
        {
            box_name = string(1, 'A' + move.c) + to_string(total_box_rows - move.r);
            side_char = "T";
        }
        else
        {
            box_name = string(1, 'A' + move.c) + to_string(total_box_rows - (move.r - 1));
            side_char = "B";
        }
    }
    else
    {
        const int total_box_cols = columns - 1;
        if (move.c < total_box_cols)
        {
            box_name = string(1, 'A' + move.c) + to_string(total_box_rows - move.r);
            side_char = "L";
        }
        else
        {
            box_name = string(1, 'A' + (move.c - 1)) + to_string(total_box_rows - move.r);
            side_char = "R";
        }
    }
    return box_name + " " + side_char;
}

void parse_turn_input()
{
    std::cin >> bot_score >> opp_score;
    std::cin.ignore();
    int num_boxes;
    std::cin >> num_boxes;
    std::cin.ignore();

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < columns - 1; ++c)
        {
            horizontal_lines[r][c] = true;
        }
    }
    for (int r = 0; r < rows - 1; ++r)
    {
        for (int c = 0; c < columns; ++c)
        {
            vertical_lines[r][c] = true;
        }
    }

    for (int i = 0; i < num_boxes; i++)
    {
        std::string box_name, sides_str;
        std::cin >> box_name >> sides_str;
        std::cin.ignore();
        int box_c = box_name[0] - 'A';
        int box_r = (rows - 1) - std::stoi(box_name.substr(1));
        for (char side : sides_str)
        {
            if (side == 'T')
            {
                horizontal_lines[box_r][box_c] = false;
            }
            else if (side == 'B')
            {
                horizontal_lines[box_r + 1][box_c] = false;
            }
            else if (side == 'L')
            {
                vertical_lines[box_r][box_c] = false;
            }
            else if (side == 'R')
            {
                vertical_lines[box_r][box_c + 1] = false;
            }
        }
    }
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int board_size;
    std::cin >> board_size;
    std::cin.ignore();
    std::string player_id;
    std::cin >> player_id;
    std::cin.ignore();

    int dim = board_size;
    rows = dim + 1;
    columns = dim + 1;

    default_arr();

    while (true)
    {
        parse_turn_input();
        Move best_move = winning_move();
        std::string output_move = translate(best_move);
        std::cout << output_move << std::endl;
    }
    return 0;
}