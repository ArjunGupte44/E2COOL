#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <atomic>

using namespace std;

enum Color { blue = 0, red, yellow, Invalid };

// Helper function to write a color
ostream &operator<<(ostream &s, const Color &c) {
    static const char *names[] = { "blue", "red", "yellow", "Invalid" };
    s << names[c];
    return s;
}

// Addition of colors
Color operator+(const Color &c1, const Color &c2) {
    switch (c1) {
        case blue:
            switch (c2) {
                case blue:   return blue;
                case red:    return yellow;
                case yellow: return red;
                default:;
            }
        case red:
            switch (c2) {
                case blue:   return yellow;
                case red:    return red;
                case yellow: return blue;
                default:;
            }
        case yellow:
            switch (c2) {
                case blue:   return red;
                case red:    return blue;
                case yellow: return yellow;
                default:;
            }
        default:;
    }
    return Invalid;
}

// Spelling out numbers
string SpellNumber(int n) {
    static const char *numbers[] = {
        " zero", " one", " two",
        " three", " four", " five",
        " six", " seven", " eight",
        " nine"
    };

    string str;
    do {
        str.insert(0, numbers[n % 10]);
        n /= 10;
    } while (n);

    return str;
}

struct Creature;

struct MeetingPlace {
    atomic<int> state;
    vector<Creature *> creatures;
    int idGenerator = 1;

    MeetingPlace(int N) : state(N << 4) { creatures.resize(N); }

    void Register(Creature &creature) {
        creature.id = idGenerator++;
        creatures[creature.id] = &creature;
    }

    void MeetUp(Creature *creature);
};

struct Creature {
    Creature() : id(0), count(0), sameCount(0), met(false) {
    }

    int Display() const {
        cout << count << SpellNumber(sameCount) << endl;
        return count;
    }

    void Meet(Creature &other) {
        if (id == other.id) {
            sameCount++;
            other.sameCount++;
        }

        count++;
        other.count++;

        Color newcolor = color + other.color;
        other.color = color = newcolor;
        other.met = true;
    }

    void Run();
    void Init(MeetingPlace *mp, Color c) { 
        place = mp;
        initialColor = color = c;
        place->Register(*this); 
    }

    int id;
    int count, sameCount;
    bool met;
    Color initialColor, color;
    MeetingPlace *place;
};

void MeetingPlace::MeetUp(Creature *creature) {
    int useState = state.load();
    while (true) {
        int waiting = useState & 0xF, tryState = 0;
        if (waiting) {
            tryState = ((useState & ~0xF) - (1 << 4));
        } else if (useState) {
            tryState = useState | creature->id;
        } else {
            return;
        }
        if (state.compare_exchange_weak(useState, tryState)) {
            if (waiting) {
                creature->Meet(*creatures[waiting]);
            } else {
                while (!creature->met) std::this_thread::yield();
                creature->met = false;
            }
            break;
        }
    }
}

void Creature::Run() {
    place->MeetUp(this);
}

int main(int argc, const char *argv[]) {
    const Color r1[] = { blue, red, yellow };

    const Color r2[] = { blue, red, yellow, red, yellow, blue, red, yellow, red, blue };

    for (int c1 = blue; c1 <= yellow; c1++)
        for (int c2 = blue; c2 <= yellow; c2++)
            cout << r1[c1] << " + " << r1[c2] << " -> " << (r1[c1] + r1[c2]) << endl;
    cout << endl;

    int n = (argc >= 2) ? atoi(argv[1]) : 6000000;

    MeetingPlace mp1(3); 
    vector<Creature> g1(3);
    for (int i = 0; i < 3; ++i) {
        g1[i].Init(&mp1, r1[i]);
    }

    MeetingPlace mp2(10);
    vector<Creature> g2(10);
    for (int i = 0; i < 10; ++i) {
        g2[i].Init(&mp2, r2[i]);
    }

    vector<future<void>> futures1;
    for (int i = 0; i < 3; ++i) {
        futures1.emplace_back(async(&Creature::Run, &g1[i]));
    }</code>
>
     
    vector<future<void>> futures2;
    for (int i = 0; i < 10; ++i) {
        futures2.emplace_back(async(&Creature::Run, &g2[i]));
    }

    for (auto &f : futures1) {
        f.get();
    }

    for (auto &f : futures2) {
        f.get();
    }

    for (auto &c : g1) { c.Display(); }

    for (auto &c : g2) { c.Display(); }

    return 0;
}

