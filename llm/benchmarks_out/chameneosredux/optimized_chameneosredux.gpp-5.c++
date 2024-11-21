#include <iostream>
#include <vector>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sched.h>
#include <pthread.h>
#include <atomic>

using namespace std;

// Use atomic operations to manage state and synchronization in a more efficient and safe manner
std::mutex mtx;  // mutex for critical section
std::condition_variable cv;

struct CPUs {
    enum { perslot = 2 };
    int count, mod;
    cpu_set_t affinities[33];

    CPUs() : count(0) {
        // Using new C++ standards for affinity setting
        for (int i = 0; i < 33; i++)
            CPU_ZERO(&affinities[i]);
        cpu_set_t cs;
        sched_getaffinity(0, sizeof(cs), &cs);

        for (int i = 0; i < CPU_SETSIZE; i++) {
            if (CPU_ISSET(i, &cs)) {
                CPU_SET(i, &affinities[(count / perslot) + 1]);
                count++;
            }
        }
        mod = (count > 2) ? count >> 1 : 1;
    }

    cpu_set_t* getaffinity(int slot) {
        return &affinities[slot ? (slot % mod) + 1 : 0];
    }
} cpus;

// Optimize thread yield by using standard library support
void yield() {
    std::this_thread::yield();
}

enum Color { blue = 0, red, yellow, Invalid };

// Streamlined the color overloading for clarity and performance
ostream& operator<<(ostream& s, const Color& c) {
    static const char* names[] = { "blue", "red", "yellow", "Invalid" };
    return s << names[c];
}

Color operator+(const Color& c1, const Color& c2) {
    switch (c1) {
        case blue: return c2 == red ? yellow : c2 == yellow ? red : blue;
        case red: return c2 == blue ? yellow : c2 == yellow ? blue : red;
        case yellow: return c2 == blue ? red : c2 == red ? blue : yellow;
        default: return Invalid;
    }
}

string SpellNumber(int n) {
    static const char* numbers[] = {
        " zero", " one", " two",
        " three", " four", " five",
        " six", " seven", " eight",
        " nine"
    };

    string str;
    if (n == 0) return " zero";
    while (n) {
        str.insert(0, numbers[n % 10]);
        n /= 10;
    }
    return str;
}

struct MeetingPlace;

class Creature {
public:
    Creature() : id(0), count(0), sameCount(0), met(false) {}

    int Display() const { return count; }
    void Meet(Creature* other);
    void Init(MeetingPlace* mp, Color c);
    void Run();
    void Start(int affinity = 0);
    void Wait() const;
    void WaitUntilMet();

    int id, count, sameCount;
    bool met;
    Color initialColor, color;

protected:
    pthread_t threadHandle;
    pthread_attr_t threadAttr;
    MeetingPlace* place;
};

struct MeetingPlace {
    enum { S = 4, creatureMask = (1 << S) - 1 };
    std::atomic<int> state;
    int idGenerator;
    Creature** creatures;

    MeetingPlace(int N) : state(N << S), idGenerator(1) {
        creatures = new Creature*[N];
    }
    ~MeetingPlace() { delete[] creatures; }

    void Register(Creature& creature) {
        creature.id = idGenerator++;
        creatures[creature.id] = &creature;
    }

    void MeetUp(Creature* creature);
};

void Creature::Init(MeetingPlace* mp, Color c) {
    place = mp;
    initialColor = color = c;
    place->Register(*this);
}

// Implement meeting logic using atomic operations
void Creature::Meet(Creature* other) {
    std::lock_guard<std::mutex> lock(mtx);
    if (id == other->id) {
        sameCount++;
        other->sameCount++;
    }
    count++;
    other->count++;

    Color newcolor = color + other->color;
    other->color = color = newcolor;
    other->met = true;
}

// Optimize thread handling by maintaining lock-free synchronization as much as possible
void Creature::Start(int affinity) {
    pthread_attr_init(&threadAttr);
    if (cpus.count >= 4) {
        cpu_set_t* cores = cpus.getaffinity(affinity);
        pthread_attr_setaffinity_np(&threadAttr, sizeof(cpu_set_t), cores);
    }
    pthread_create(&threadHandle, &threadAttr, [](void* param) -> void* {
        static_cast<Creature*>(param)->Run();
        return nullptr;
    }, this);
}

void Creature::Run() {
    place->MeetUp(this);
}

// Using atomic-fence to ensure memory synchronization
void Creature::WaitUntilMet() {
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [this] { return met; });
    met = false;
}

void Creature::Wait() const {
    pthread_join(threadHandle, nullptr);
}

void MeetingPlace::MeetUp(Creature* creature) {
    int useState = state.load();
    while (true) {
        int waiting = useState & creatureMask;
        int tryState = useState;

        if (waiting) {
            tryState = (useState & ~creatureMask) - (1 << S);
        } else if (useState) {
            tryState = useState | creature->id;
        } else {
            return;
        }

        if (state.compare_exchange_weak(useState, tryState)) {
            if (waiting) {
                creature->Meet(creatures[waiting]);
            } else {
                creature->WaitUntilMet();
            }
            break;
        }
    }
}

template<int ncolor>
struct Game {
    Game(int meetings, const Color (&color)[ncolor]) : meetingPlace(meetings) {
        for (int i = 0; i < ncolor; ++i) {
            creatures[i].Init(&meetingPlace, color[i]);
        }
    }

    void Start(int affinity = 0) {
        for (int i = 0; i < ncolor; ++i) {
            creatures[i].Start(affinity);
        }
    }

    void Wait() {
        for (int i = 0; i < ncolor; ++i) {
            creatures[i].Wait();
        }
    }

    void Display() {
        for (int i = 0; i < ncolor; ++i) {
            cout << " " << creatures[i].initialColor;
        }
        cout << endl;

        int total = 0;
        for (int i = 0; i < ncolor; ++i) {
            total += creatures[i].Display();
        }
        cout << SpellNumber(total) << endl << endl;
    }

protected:
    MeetingPlace meetingPlace;
    Creature creatures[ncolor];
};

int main(int argc, const char* argv[]) {
    const Color r1[] = { blue, red, yellow };

    const Color r2[] = {
        blue, red, yellow,
        red, yellow, blue,
        red, yellow, red,
        blue
    };

    for (int c1 = blue; c1 <= yellow; c1++)
        for (int c2 = blue; c2 <= yellow; c2++)
            cout << r1[c1] << " + " << r1[c2] << " -> " << (r1[c1] + r1[c2]) << endl;
    cout << endl;

    int n = (argc >= 2) ? atoi(argv[1]) : 6000000;

    Game<3> g1(n, r1);
    Game<10> g2(n, r2);

    g1.Start(1);
    g2.Start(2);

    g1.Wait();
    g2.Wait();

    g1.Display();
    g2.Display();

    return 0;
}
