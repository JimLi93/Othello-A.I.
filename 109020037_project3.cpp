#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

#define DEPTH 7

int player;
float global_chosen_array[30] = {0};

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
    int n_valid_spots;
    int force_no_move = 0;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }

public:
    OthelloBoard() {
        //reset();
    }
    OthelloBoard &operator = (const OthelloBoard & b){
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = b.board[i][j];
            }
        }
        n_valid_spots = b.n_valid_spots;
        cur_player = b.cur_player;
        winner = b.winner;
        done = b.done;
        for(int i=0;i<3;i++){
            disc_count[i] = b.disc_count[i];
        }
        for(int i=0;i<n_valid_spots;i++){
            Point p = b.next_valid_spots[i];
            next_valid_spots.push_back(p);
        }
        return *this;
    }

    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    void put_disc(Point p) {
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        n_valid_spots = next_valid_spots.size();
        // Check Win
        if (next_valid_spots.size() == 0) {
            force_no_move = 1;
        }
        else {
            force_no_move = 0;
        }
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
};

const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
OthelloBoard new_board;
float tryit[64] = {16.16, -3.03, 0.99, 0.43, 0.43, 0.99, -3.03, 16.16, 
                                 -4.12, -1.81, -0.08, -0.27, -0.27, -0.08, -1.81, -4.12,
                                 1.33, -0.04, 0.51, 0.07, 0.07, 0.51, -0.04, 1.33,
                                 0.63, -0.18, -0.04, -0.01, -0.01, -0.04, -0.18, 0.63,
                                 0.63, -0.18, -0.04, -0.01, -0.01, -0.04, -0.18, 0.63,
                                 1.33, -0.04, 0.51, 0.07, 0.07, 0.51, -0.04, 1.33,
                                 -4.12, -1.81, -0.08, -0.27, -0.27, -0.08, -1.81, -4.12,
                                 16.16, -3.03, 0.99, 0.43, 0.43, 0.99, -3.03, 16.16};
                                 
float state_value[SIZE][SIZE];

float calculate(OthelloBoard b){
    float v = 0;
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            if(b.board[i][j] == player){
                v += state_value[i][j] * 1;
            }
            else if(b.board[i][j] == 0){

            }
            else {
                v -= state_value[i][j];
            }
        }
    }
    if(b.force_no_move == 1){
        return v+100;
    }
    return v;
}

float alphabeta(OthelloBoard b, int depth, float alpha, float beta, bool maximizingPlayer){
    if(depth <= 0){
        return calculate(b);
    }
    if(maximizingPlayer){
        float value = -10000;
        for(int i=0;i<b.n_valid_spots;i++){
            OthelloBoard new_b = b;;
            new_b.put_disc(b.next_valid_spots[i]);
            float tmp;
            tmp = alphabeta(new_b, depth-1, alpha, beta, false);
            if(depth == DEPTH){
                global_chosen_array[i] = tmp;
            }
            if(tmp > value) value = tmp;
            if(value > alpha) alpha = value;
            if(alpha >= beta) break;
        }
        return value;
    }
    else {
        float value = 10000;
        for(int i=0;i<b.n_valid_spots;i++){
            OthelloBoard new_b = b;;
            new_b.put_disc(b.next_valid_spots[i]);
            float tmp;
            tmp = alphabeta(new_b, depth-1, alpha, beta, true);
            if(depth == DEPTH){
                global_chosen_array[i] = tmp;
            }
            if(tmp < value) value = tmp;
            if(value < beta) beta = value;
            if(alpha >= beta) break;
        }
        return value;
    }
}

void read_board(std::ifstream& fin) {
    for(int i=0;i<3;i++){
        new_board.disc_count[i] = 0;
    }
    new_board.done = false;
    new_board.winner = -1;
    new_board.force_no_move = 0;
    fin >> player;
    new_board.cur_player = player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
            new_board.board[i][j] = board[i][j];
            new_board.disc_count[board[i][j]] += 1;
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    new_board.n_valid_spots = n_valid_spots;
    int x, y;
    new_board.next_valid_spots.clear();
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        Point p = Point(x, y);
        next_valid_spots.push_back(p);
        new_board.next_valid_spots.push_back(p);
    }
}

void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = next_valid_spots.size();
    srand(time(NULL));
    int chosen = 0;
    float max = -10000;
    float value[n_valid_spots];
    //just use state value to input first value
    for(int i=0;i<n_valid_spots;i++){
        OthelloBoard tmp_board = new_board;
        tmp_board.put_disc(next_valid_spots[i]);
        value[i] = calculate(tmp_board);
        if(value[i] > max){
            max = value[i];
            chosen = i;
        }
    }
    //chosen = 0;
    Point p = next_valid_spots[chosen];
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << endl;
    chosen = 0;
    
    float target_value;
    target_value = alphabeta(new_board, DEPTH, -1000, 1000, true);
    for(int i=0;i<n_valid_spots;i++){
        float distance = global_chosen_array[i] - target_value;
        if(global_chosen_array[i] == target_value || (distance < 0.001 && distance > -0.001)){
            chosen = i;
            //p = next_valid_spots[i];
            //fout << p.x << " " << p.y << endl;
            break;
        }
    }


    // Choose random spot. (Not random uniform here)
    //int index = (rand() % n_valid_spots);
    p = next_valid_spots[chosen];
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << endl;
    fout.flush();
}

int main(int, char** argv) {
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            state_value[i][j] = tryit[i *SIZE+j];
            //state_value[i][j] = 1;
        }
    }
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
