// 04_virtual_functions_and_vtables — YOUR attempt. Fill in the TODO bodies, then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables -v
// Or compile and run directly:
//   clang++ -std=c++17 -Wall -o /tmp/vtables starter.cpp && /tmp/vtables
// The stubs compile as-is but fail main()'s asserts until you implement them.
// You're done when the last line printed is: ALL TESTS PASSED

#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Provided plumbing (not a drill): destructors append their class name here so
// main() can assert WHICH destructors ran, and in what ORDER. A function-local
// static is C++'s simplest shared-state idiom — think lazily initialized
// static field, built on first call, one instance for the whole program.
std::vector<std::string>& destructionLog() {
    static std::vector<std::string> log;
    return log;
}

// Drill 1 — Sensor: a C++ "interface".
// JAVA: interface Sensor { String name(); double read(); }
// C++:  an abstract base class: pure virtual methods (= 0) declare the
//       contract, plus the one member Java never needed — a VIRTUAL destructor,
//       so `delete` through a Sensor* tears down the real object completely.
// TODO: make the destructor append "Sensor" to destructionLog(). (It runs LAST
//       in every teardown: derived destructor first, then base.)
class Sensor {
public:
    virtual ~Sensor() { /* TODO: log "Sensor" */ }
    virtual std::string name() const = 0;  // = 0: pure virtual, no body here
    virtual double read() = 0;
};

// Drill 2 — Camera and Lidar: two concrete backends.
// name() -> "camera" / "lidar"; read() -> 30.0 / 10.0 (pretend rates in Hz).
// Each destructor logs its own class name ("Camera" / "Lidar"); base ~Sensor
// then logs "Sensor", so destroying a Camera through ANY handle must produce
// {"Camera","Sensor"}.
// JAVA: class Camera implements Sensor — but here `override` is doing
//       @Override's job, and inheriting Sensor's virtual dtor is load-bearing.
class Camera : public Sensor {
public:
    ~Camera() override { /* TODO: log "Camera" */ }
    std::string name() const override {
        // TODO: implement
        return "";
    }
    double read() override {
        // TODO: implement
        return 0.0;
    }
};

class Lidar : public Sensor {
public:
    ~Lidar() override { /* TODO: log "Lidar" */ }
    std::string name() const override {
        // TODO: implement
        return "";
    }
    double read() override {
        // TODO: implement
        return 0.0;
    }
};

// Drill 3 — pollAll: one loop over mixed sensors, ZERO type checks.
// pollAll({Camera, Lidar}) -> {"camera=30", "lidar=10"}
// JAVA: for (Sensor s : sensors) out.add(s.name() + "=" + ...); — free in Java.
// C++:  s->name() / s->read() are virtual calls, so each unique_ptr<Sensor>
//       dispatches to whatever it really holds. Build each line with an
//       std::ostringstream (it prints 30.0 as "30", which is what we want).
std::vector<std::string> pollAll(const std::vector<std::unique_ptr<Sensor>>& sensors) {
    // TODO: implement
    return {};
}

// Drill 4 — describe: polymorphism through a REFERENCE.
// describe(camera) -> "Sensor[camera]"
// The parameter is `const Sensor&` — dynamic dispatch works through references
// exactly as through pointers. Taking `Sensor` BY VALUE would slice a concrete
// base (README §6); here it wouldn't even compile, because an abstract class
// can't be instantiated — a hidden safety bonus of pure-virtual interfaces.
std::string describe(const Sensor& sensor) {
    // TODO: implement — return "Sensor[" + ... + "]"
    return "";
}

// Drill 5 — the "forgot virtual" demo. id() is NOT virtual — on purpose.
// BrokenDerived::id() HIDES the base version (a new, unrelated function); it
// does not override it. There is no vtable here at all. Leave these two
// structs exactly as they are — the drill is the function below them.
struct BrokenBase {
    std::string id() const { return "BrokenBase"; }
};
struct BrokenDerived : BrokenBase {
    std::string id() const { return "BrokenDerived"; }  // hides — does NOT override
};

// TODO: build a BrokenDerived on the stack, point a `const BrokenBase*` at it,
// and return p->id(). Before running it, predict the string it returns —
// that prediction is the whole point of the drill.
std::string brokenDispatchDemo() {
    // TODO: implement
    return "";
}

int main() {
    // Drills 1+2: dynamic dispatch — a Sensor* runs the OBJECT's methods
    {
        std::unique_ptr<Sensor> cam = std::make_unique<Camera>();
        std::unique_ptr<Sensor> lid = std::make_unique<Lidar>();
        assert(cam->name() == "camera");  // Camera::name, not "some Sensor default"
        assert(lid->name() == "lidar");
        assert(cam->read() == 30.0);
        assert(lid->read() == 10.0);
    }

    // Drill 3: pollAll — one heterogeneous loop, no if/else on the type
    {
        std::vector<std::unique_ptr<Sensor>> sensors;
        sensors.push_back(std::make_unique<Camera>());
        sensors.push_back(std::make_unique<Lidar>());
        sensors.push_back(std::make_unique<Camera>());
        auto lines = pollAll(sensors);
        assert((lines ==
                std::vector<std::string>{"camera=30", "lidar=10", "camera=30"}));
    }

    // Drill 4: describe — dispatch through a const reference
    {
        Camera cam;
        Lidar lid;
        assert(describe(cam) == "Sensor[camera]");
        assert(describe(lid) == "Sensor[lidar]");
        const Sensor& asBase = cam;                  // upcast the reference...
        assert(describe(asBase) == "Sensor[camera]");  // ...still the Camera underneath
    }

    // Drills 1+2, the destructor half: deleting through a Sensor* (which is
    // what unique_ptr<Sensor> does at its closing brace) must run the DERIVED
    // destructor first, then the base — {"Camera","Sensor"}, never just
    // {"Sensor"}. This assert is exactly the virtual-destructor classic.
    {
        destructionLog().clear();
        {
            std::unique_ptr<Sensor> s = std::make_unique<Camera>();
        }  // s destroys its Camera through a Sensor* right here
        assert((destructionLog() == std::vector<std::string>{"Camera", "Sensor"}));

        destructionLog().clear();
        {
            std::unique_ptr<Sensor> s = std::make_unique<Lidar>();
        }
        assert((destructionLog() == std::vector<std::string>{"Lidar", "Sensor"}));
    }

    // Drill 5: no virtual anywhere -> the base version runs through a base ptr
    assert(brokenDispatchDemo() == "BrokenBase");

    std::cout << "ALL TESTS PASSED" << std::endl;
    return 0;
}
