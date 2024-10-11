```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <atomic>

using namespace std;

enum Color { blue = 0, red, yellow, Invalid };

ostream &operator<<(ostream &s, const Color &c) {
    static const char *names[] = {"blue", "red", "yellow", "Invalid"};
    s << names[c];
    return s;
}

Color operator+(const Color &c1, const Color &c2) {
    const Color results[3][3] = {{blue, yellow, red}, {yellow, red, blue}, {red, blue, yellow}};
    if (c1 < 3 && c2 < 3) return results[c1][c2];
    else return Invalid;
}

string SpellNumber(int n) {
    static const char *numbers[] = {" zero", " one", " two", " three", " four", " five", " six", " seven", " eight", " nine"};

    string str;
    do {
        str.insert(0, numbers[n % 10]);
        n /= 10;
    } while (n);

    return str;
}

struct MeetingPlace;

class Creature {
public:
    Creature() : id(0), count(0), sameCount(0), met(false), place(nullptr), threadHandle(nullptr) {}
    ~Creature() { if(threadHandle) threadHandle->join(); delete threadHandle; }

    void Initialize(MeetingPlace *mp, Color c);
    void Wait() { threadHandle->join(); }

    int Display() const {
        cout << count << SpellNumber(sameCount) << endl;
        return count;
    }

private:
    void Run();
    void MeetWith(Creature *other);

    int id, count, sameCount;
    volatile bool met;
    Color initialColor, color;
    thread *threadHandle;
    MeetingPlace *place;

    friend struct MeetingPlace;
};

struct MeetingPlace {
    MeetingPlace(int max_meetings): state(max_meetings << 4), idGen(1) {
        creatures.reserve(max_meetings);
    }

    void Register(Creature &c) {
        c.id = idGen.fetch_add(1);
        creatures.push_back(&c);
    }

    void Meet(Creature &c);

private:
    atomic<int> state;
    atomic<int> idGen;
    vector<Creature*> creatures;
    mutex meetMutex;
};

void Creature::Initialize(MeetingPlace *mp, Color c) {
    place = mp;
    initialColor = color = c;
    place->Register(*this);
    threadHandle = new thread(&Creature::Run, this);
}

void Creature::Run() {
    place->Meet(*this);
}

void Creature::MeetWith(Creature *other) {
    if (this == other) return;

    // Logic to define what happens when two creatures meet
    // Possibility to increase count or change state
    color = color + other->color;
    if (initialColor == color) sameCount++;
    other->color = color + other->color; // Simulate mutual meeting affect
    count++;
}

void MeetingPlace::Meet(Creature &c) {
    bool loop = true;

    while (loop) {
        lock_guard<mutex> guard(meetMutex);

        int currentState = state.load();
        int waitingId = currentState & 0xF;

        if (waitingId) {
            // Decrement meetings, pairing is now ready to be performed
            state -= 1 << 4;
            c.MeetWith(creatures[waitingId - 1]);
            loop = false;
        } else if (currentState) {
            state |= c.id;
            c.met = true;
        } else {
            loop = false; // No more meetings, exit
        }

        if (c.met) {
            c.met = false;
            this_thread::yield();
        }
    }
}

class Game {
public:
    Game(int meetings, const vector<Color> &colors): meetingPlace(meetings) {
        for (const auto &color : colors) {
            creatures.emplace_back();
            creatures.back().Initialize(&meetingPlace, color);
        }
    }

    void Start() {
        for (auto &creature : creatures) {
            creature.Wait();
        }
    }

    void Display() {
        int total = 0;
        for (const auto &creature : creatures) {
            total += creature.Display();
        }
        cout << SpellNumber(total) << "\n\n";
    }

private:
    MeetingPlace meetingPlace;
    vector<Creature> creatures;
};

int main(int argc, const char *argv[]) {
    vector<Color> r1 = { blue, red, yellow };
    vector<Color> r2 = { blue, red, yellow, red, yellow, blue, red, yellow, red, blue };

    for (int c1 = blue; c1 <= yellow; c1++)
        for (int c2 = blue; c2 <= yellow; c2++)
            cout << r1[c1] << " + " << r1[c2] << " -> " << (r1[c1] + r1[c2]) << endl;
    cout << endl;

    int n = (argc >= 2) ? atoi(argv[1]) : 6000000;

    Game g1(n, r1);
    Game g2(n, r2);
    g1.Start();
    g2.Start();
    g1.Display();
    g2.Display();

    return 0;
}
```