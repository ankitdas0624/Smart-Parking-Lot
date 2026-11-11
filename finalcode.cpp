#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <limits>
#include <exception>
#include <stdexcept>
#include <functional>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;

class ParkingException : public exception {
protected:
    string message;

public:
    explicit ParkingException(const string& message)
        : message(message) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

    virtual ~ParkingException() noexcept = default;
};

class InvalidInputException : public ParkingException {
public:
    explicit InvalidInputException(const string& message)
        : ParkingException("Invalid input: " + message) {}
};

class InvalidDurationException : public InvalidInputException {
public:
    explicit InvalidDurationException(int hours)
        : InvalidInputException("duration cannot be negative (got "
                                + to_string(hours) + ")") {}
};

class InvalidDiscountException : public InvalidInputException {
public:
    explicit InvalidDiscountException(double discount)
        : InvalidInputException("discount must be between 0 and 100 (got "
                                + to_string(discount) + ")") {}
};

class SpaceNotFoundException : public ParkingException {
public:
    explicit SpaceNotFoundException(int id)
        : ParkingException("Parking space " + to_string(id)
                           + " was not found.") {}
};

class SpaceOccupiedException : public ParkingException {
public:
    explicit SpaceOccupiedException(int id)
        : ParkingException("Parking space " + to_string(id)
                           + " is already occupied.") {}
};

class SpaceAvailableException : public ParkingException {
public:
    explicit SpaceAvailableException(int id)
        : ParkingException("Parking space " + to_string(id)
                           + " is already available.") {}
};

class VehicleMismatchException : public ParkingException {
public:
    VehicleMismatchException(const string& vehicleType,
                             const string& spaceType)
        : ParkingException("A " + vehicleType
                           + " cannot be parked in a " + spaceType
                           + " space.") {}
};

class Finally {
private:
    function<void()> action;

public:
    explicit Finally(function<void()> action)
        : action(move(action)) {}

    Finally(const Finally&) = delete;
    Finally& operator=(const Finally&) = delete;

    ~Finally() noexcept {
        try {
            action();
        } catch (...) {
        }
    }
};

template <typename T>
class Registry {
private:
    vector<T> items;

public:
    void add(const T& item) {
        items.push_back(item);
    }

    int size() const {
        return static_cast<int>(items.size());
    }

    template <typename Pred>
    T* findIf(Pred pred) {
        for (T& item : items) {
            if (pred(item)) {
                return &item;
            }
        }
        return nullptr;
    }

    template <typename Pred>
    int countIf(Pred pred) const {
        int count = 0;
        for (const T& item : items) {
            if (pred(item)) {
                count++;
            }
        }
        return count;
    }

    template <typename Func>
    void forEach(Func fn) const {
        for (const T& item : items) {
            fn(item);
        }
    }
};

template <typename T>
void readInput(const string& prompt, T& value) {
    cout << prompt;
    if (!(cin >> value)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        throw InvalidInputException("could not read a value of the expected type.");
    }
}

const int UI_WIDTH = 52;

void printBorder(char fill = '-') {
    cout << '+' << string(UI_WIDTH, fill) << "+\n";
}

template <typename T>
void printCentered(const T& text) {
    ostringstream oss;
    oss << text;
    string s = oss.str();

    int len = static_cast<int>(s.size());
    int left = (UI_WIDTH - len) / 2;
    if (left < 0) left = 0;
    int right = UI_WIDTH - len - left;
    if (right < 0) right = 0;

    cout << '|' << string(left, ' ') << s
         << string(right, ' ') << "|\n";
}

void printSection(const string& title) {
    cout << "|  " << left << setw(UI_WIDTH - 2) << title << "|\n";
}

void printMenuItem(int key, const string& label) {
    cout << "|   [" << key << "]  "
         << left << setw(UI_WIDTH - 8) << label << "|\n";
}

class Vehicle {
protected:
    string number;
    string type;

public:
    Vehicle(string number, string type)
        : number(number), type(type) {
        if (number.empty()) {
            throw InvalidInputException("vehicle number cannot be empty.");
        }
    }

    Vehicle(const Vehicle& other)
        : number(other.number), type(other.type) {}

    virtual void display() const = 0;

    string getNumber() const {
        return number;
    }

    string getType() const {
        return type;
    }

    virtual ~Vehicle() = default;
};

class Car : public Vehicle {
public:
    Car(string number) : Vehicle(number, "Car") {}

    void display() const override {
        cout << "Vehicle Type: Car\n";
        cout << "Vehicle Number: " << number << '\n';
    }
};

class Bike : public Vehicle {
public:
    Bike(string number) : Vehicle(number, "Bike") {}

    void display() const override {
        cout << "Vehicle Type: Bike\n";
        cout << "Vehicle Number: " << number << '\n';
    }
};

class ParkingSpace final {
private:
    int id;
    string type;
    double rate;
    bool occupied;

public:
    ParkingSpace(int id, string type, double rate)
        : id(id), type(type), rate(rate), occupied(false) {
        if (rate < 0) {
            throw invalid_argument("Parking rate cannot be negative.");
        }
    }

    ParkingSpace(const ParkingSpace& other)
        : id(other.id),
          type(other.type),
          rate(other.rate),
          occupied(other.occupied) {}

    int getId() const {
        return id;
    }

    string getType() const {
        return type;
    }

    double getRate() const {
        return rate;
    }

    bool isOccupied() const {
        return occupied;
    }

    void park() {
        if (occupied) {
            throw SpaceOccupiedException(id);
        }

        occupied = true;
    }

    void exitParking() {
        if (!occupied) {
            throw SpaceAvailableException(id);
        }

        occupied = false;
    }

    void display() const {
        cout << "Space ID: " << id
             << " | Type: " << type
             << " | Rate/hour: Rs. " << rate
             << " | Status: "
             << (occupied ? "Occupied" : "Available")
             << '\n';
    }

    friend bool operator==(const ParkingSpace& a,
                           const ParkingSpace& b) {
        return a.id == b.id;
    }

    friend ostream& operator<<(ostream& out,
                               const ParkingSpace& space) {
        out << "Space ID: " << space.id
            << " | Type: " << space.type
            << " | Rate/hour: Rs. " << space.rate
            << " | Status: "
            << (space.occupied ? "Occupied" : "Available");

        return out;
    }
};

class User {
protected:
    string name;
    int userId;

public:
    User(string name, int userId)
        : name(name), userId(userId) {}

    virtual void display() const {
        cout << "User Name: " << name << '\n';
        cout << "User ID: " << userId << '\n';
    }

    virtual ~User() = default;
};

class Driver : public User {
private:
    Vehicle* vehicle;

public:
    Driver(string name, int userId, Vehicle* vehicle)
        : User(name, userId), vehicle(vehicle) {}

    void display() const override {
        cout << "\n--- Driver Details ---\n";
        User::display();

        if (vehicle != nullptr) {
            vehicle->display();
        }
    }
};

class ParkingSession {
private:
    int sessionId;
    Vehicle* vehicle;
    ParkingSpace* space;

public:
    ParkingSession(int sessionId,
                   Vehicle* vehicle,
                   ParkingSpace* space)
        : sessionId(sessionId),
          vehicle(vehicle),
          space(space) {
        if (vehicle == nullptr || space == nullptr) {
            throw invalid_argument("Session needs a vehicle and a space.");
        }
    }

    double calculateFee(int hours) const {
        if (hours < 0) {
            throw InvalidDurationException(hours);
        }

        return hours * space->getRate();
    }

    double calculateFee(int hours, double discount) const {
        if (hours < 0) {
            throw InvalidDurationException(hours);
        }

        if (discount < 0 || discount > 100) {
            throw InvalidDiscountException(discount);
        }

        double fee = hours * space->getRate();
        return fee - (fee * discount / 100.0);
    }

    void displaySession() const {
        cout << "\n--- Parking Session ---\n";
        cout << "Session ID: " << sessionId << '\n';
        cout << "Vehicle Number: " << vehicle->getNumber() << '\n';
        cout << "Vehicle Type: " << vehicle->getType() << '\n';
        cout << "Parking Space ID: " << space->getId() << '\n';
        cout << "Rate per Hour: Rs. " << space->getRate() << '\n';
    }
};

class Reportable {
public:
    virtual void showReport() const = 0;

    virtual ~Reportable() = default;
};

class ParkingLot : public Reportable {
private:
    Registry<ParkingSpace> spaces;
    mutable mutex spacesMutex; // Protects shared parking-space data

    static string parkingLotName;

    class ParkingReport {
    public:
        void generateReport(
            const Registry<ParkingSpace>& spaces) const {

            int total = spaces.size();
            int occupied = spaces.countIf(
                [](const ParkingSpace& s) { return s.isOccupied(); });

            cout << "\n--- Parking Report ---\n";
            cout << "Parking Lot: " << parkingLotName << '\n';
            cout << "Total Spaces: " << total << '\n';
            cout << "Occupied Spaces: " << occupied << '\n';
            cout << "Available Spaces: "
                 << total - occupied << '\n';
        }
    };

public:
    ParkingLot() = default;

    static string getParkingLotName() {
        return parkingLotName;
    }

    void addSpace(const ParkingSpace& space) {
        lock_guard<mutex> lock(spacesMutex);
        spaces.add(space);
    }

    int totalSpaces() const {
        lock_guard<mutex> lock(spacesMutex);
        return spaces.size();
    }

    int availableSpaces() const {
        lock_guard<mutex> lock(spacesMutex);
        return spaces.countIf(
            [](const ParkingSpace& s) { return !s.isOccupied(); });
    }
    int allotFirstAvailable(const string& vehicleType) {
        lock_guard<mutex> lock(spacesMutex);

        ParkingSpace* selected = spaces.findIf(
            [&vehicleType](const ParkingSpace& s) {
                return !s.isOccupied() && s.getType() == vehicleType;
            });

        if (selected == nullptr) {
            return -1;
        }

        selected->park();
        return selected->getId();
    }

    // Safely release a space by ID.
    void releaseSpaceSafely(int id) {
        lock_guard<mutex> lock(spacesMutex);

        ParkingSpace* selected = spaces.findIf(
            [id](const ParkingSpace& s) { return s.getId() == id; });

        if (selected == nullptr) {
            throw SpaceNotFoundException(id);
        }

        selected->exitParking();
    }

    void showAvailable() const {
        lock_guard<mutex> lock(spacesMutex);
        bool found = false;

        cout << "\n--- Available Parking Spaces ---\n";

        spaces.forEach([&](const ParkingSpace& space) {
            if (!space.isOccupied()) {
                cout << space << '\n';
                found = true;
            }
        });

        if (!found) {
            cout << "No parking spaces are available.\n";
        }
    }

    ParkingSpace& findSpace(int id) {
        ParkingSpace* space = spaces.findIf(
            [id](const ParkingSpace& s) { return s.getId() == id; });

        if (space == nullptr) {
            throw SpaceNotFoundException(id);
        }

        return *space;
    }

    void showAll() const {
        lock_guard<mutex> lock(spacesMutex);
        cout << "\n--- All Parking Spaces ---\n";

        spaces.forEach([](const ParkingSpace& space) {
            cout << space << '\n';
        });
    }

    void showReport() const override {
        lock_guard<mutex> lock(spacesMutex);
        ParkingReport report;
        report.generateReport(spaces);
    }
};

string ParkingLot::parkingLotName = "Smart Parking Lot";



mutex outputMutex;

.
void processParkingRequest(ParkingLot& lot,
                           const string& vehicleName,
                           const string& vehicleType) {
    {
        lock_guard<mutex> outputLock(outputMutex);
        cout << "[RUNNABLE] " << vehicleName
             << " request started. Checking for a " << vehicleType
             << " space...\n";
    }

    // C++ equivalent of Thread.sleep(1000).
    this_thread::sleep_for(chrono::milliseconds(1000));

    int spaceId = lot.allotFirstAvailable(vehicleType);

    {
        lock_guard<mutex> outputLock(outputMutex);
        if (spaceId != -1) {
            cout << "[SUCCESS] " << vehicleName
                 << " received parking space " << spaceId << ".\n";
        } else {
            cout << "[WAIT/NO SLOT] " << vehicleName
                 << ": no compatible space is available.\n";
        }
    }
    this_thread::yield();

    {
        lock_guard<mutex> outputLock(outputMutex);
        cout << "[FINISHED] " << vehicleName
             << " parking request completed.\n";
    }
}


class ParkingWorker {
private:
    ParkingLot& lot;
    string vehicleName;
    string vehicleType;

public:
    ParkingWorker(ParkingLot& lot,
                  string vehicleName,
                  string vehicleType)
        : lot(lot),
          vehicleName(move(vehicleName)),
          vehicleType(move(vehicleType)) {}

    void operator()() const {
        processParkingRequest(lot, vehicleName, vehicleType);
    }
};
void demonstrateMultithreading(ParkingLot& lot) {
    cout << "\n--- MULTITHREADING PARKING DEMONSTRATION ---\n";
    cout << "Creating three vehicle request threads...\n";
    cout << "Each thread will request a compatible parking space.\n\n";

    
    ParkingWorker carA(lot, "Car A", "Car");
    ParkingWorker carB(lot, "Car B", "Car");
    ParkingWorker bikeC(lot, "Bike C", "Bike");
    thread t1(carA);
    thread t2(carB);
    thread t3(bikeC);

    cout << "\nMain thread is waiting for all vehicle requests...\n";

    // join() makes the main thread wait for each worker to finish.
    t1.join();
    t2.join();
    t3.join();

    cout << "\nAll vehicle request threads have TERMINATED.\n";
    cout << "Updated parking status:\n";
    lot.showAll();
    cout << "\nMultithreading demonstration completed.\n";
}

void vehiclePolymorphism() {
    Car car("GJ01AB1234");
    Bike bike("GJ01CD5678");

    Vehicle* vehicle1 = &car;
    Vehicle* vehicle2 = &bike;

    cout << "\n--- Vehicle Polymorphism ---\n";

    vehicle1->display();
    cout << '\n';
    vehicle2->display();
}

void exceptionDemo(ParkingLot& lot) {
    Finally demoCleanup([]() {
        cout << "[finally] Exception demo cleanup executed.\n";
    });

    cout << "\n--- Exception Handling Demo ---\n";

    cout << "\n1. Derived exception caught by base-class handler:\n";
    try {
        throw VehicleMismatchException("Car", "Bike");
    } catch (const ParkingException& e) {
        cout << "Caught as ParkingException: " << e.what() << '\n';
    }

    cout << "\n2. Specific derived handler listed before the base handler:\n";
    try {
        lot.findSpace(99);
    } catch (const SpaceNotFoundException& e) {
        cout << "Caught as SpaceNotFoundException: " << e.what() << '\n';
    } catch (const ParkingException& e) {
        cout << "Caught as ParkingException: " << e.what() << '\n';
    }

    cout << "\n3. Rethrow with throw; and nested try/catch:\n";
    try {
        try {
            ParkingSpace temp(100, "Car", 50);
            Car car("GJ01ZZ0001");
            ParkingSession session(0, &car, &temp);
            session.calculateFee(-3);
        } catch (const InvalidDurationException& e) {
            cout << "Inner handler: " << e.what() << '\n';
            throw;
        }
    } catch (const InvalidInputException& e) {
        cout << "Outer handler (base InvalidInputException): "
             << e.what() << '\n';
    }

    cout << "\n4. Standard library exception:\n";
    try {
        ParkingSpace bad(101, "Car", -10);
    } catch (const exception& e) {
        cout << "Caught as std::exception: " << e.what() << '\n';
    }

    cout << "\n5. Catch-all handler:\n";
    try {
        throw 42;
    } catch (const ParkingException& e) {
        cout << "Parking error: " << e.what() << '\n';
    } catch (...) {
        cout << "Caught an unknown exception type.\n";
    }
}

void showMenu(const ParkingLot& lot) {
    ostringstream status;
    status << "Available: " << lot.availableSpaces()
           << " / " << lot.totalSpaces()
           << "   |   Occupied: "
           << lot.totalSpaces() - lot.availableSpaces();

    cout << '\n';
    printBorder('=');
    printCentered("SMART PARKING MANAGEMENT SYSTEM");
    printCentered(ParkingLot::getParkingLotName());
    printBorder('=');
    printCentered(status.str());
    printBorder('-');
    printSection("PARKING OPERATIONS");
    printMenuItem(1, "Show Available Spaces");
    printMenuItem(2, "Park a Vehicle");
    printMenuItem(3, "Exit Parking");
    printBorder('-');
    printSection("INFORMATION & REPORTS");
    printMenuItem(4, "Show Driver Details");
    printMenuItem(5, "Show All Parking Spaces");
    printMenuItem(6, "Demonstrate Vehicle Polymorphism");
    printMenuItem(7, "Show Parking Report");
    printMenuItem(8, "Demonstrate Exception Handling");
    printMenuItem(9, "Demonstrate Multithreading");
    printBorder('-');
    printSection("SYSTEM");
    printMenuItem(10, "Exit Program");
    printBorder('=');
}

int main() {
    ParkingLot lot;

    lot.addSpace(ParkingSpace(1, "Car", 50));
    lot.addSpace(ParkingSpace(2, "Car", 50));
    lot.addSpace(ParkingSpace(3, "Bike", 20));
    lot.addSpace(ParkingSpace(4, "Bike", 20));

    int choice = 0;
    int nextSessionId = 1;

    do {
        showMenu(lot);

        Finally requestDone([]() {
            cout << "[finally] Request processing finished.\n";
        });

        try {
            readInput("  Enter your choice: ", choice);

            switch (choice) {
            case 1:
                lot.showAvailable();
                break;

            case 2: {
                int vehicleChoice;
                int spaceId;
                int hours;
                string number;

                readInput("Select vehicle type (1. Car, 2. Bike): ",
                          vehicleChoice);

                if (vehicleChoice != 1 && vehicleChoice != 2) {
                    throw InvalidInputException("vehicle type must be 1 or 2.");
                }

                readInput("Enter vehicle number: ", number);

                unique_ptr<Vehicle> vehicle;

                if (vehicleChoice == 1) {
                    vehicle.reset(new Car(number));
                } else {
                    vehicle.reset(new Bike(number));
                }

                readInput("Enter parking space ID: ", spaceId);

    
                int allottedId = -1;
                {
                  
                    ParkingSpace& requestedSpace = lot.findSpace(spaceId);
                    if (requestedSpace.getType() != vehicle->getType()) {
                        throw VehicleMismatchException(vehicle->getType(),
                                                       requestedSpace.getType());
                    }
                    // The interactive menu is the only caller here; the
                    // concurrent demonstration uses allotFirstAvailable().
                    requestedSpace.park();
                    allottedId = requestedSpace.getId();
                }

                try {
                    readInput("Enter parking duration in hours: ", hours);

                    if (hours < 0) {
                        throw InvalidDurationException(hours);
                    }
                } catch (...) {
                    lot.releaseSpaceSafely(allottedId);
                    throw;
                }

                ParkingSpace& space = lot.findSpace(allottedId);
                ParkingSession session(
                    nextSessionId++, vehicle.get(), &space);

                session.displaySession();

                cout << "Parking Fee: Rs. "
                     << session.calculateFee(hours) << '\n';

                cout << "Fee with 10% discount: Rs. "
                     << session.calculateFee(hours, 10) << '\n';

                cout << "Vehicle parked successfully.\n";
                break;
            }

            case 3: {
                int spaceId;

                readInput("Enter parking space ID to free: ", spaceId);

                lot.releaseSpaceSafely(spaceId);

                cout << "Parking space " << spaceId
                     << " is now available.\n";
                break;
            }

            case 4: {
                string name;
                int userId;
                int vehicleChoice;
                string number;

                cout << "Enter driver name: ";
                cin >> ws;
                getline(cin, name);

                readInput("Enter user ID: ", userId);

                readInput("Select vehicle type (1. Car, 2. Bike): ",
                          vehicleChoice);

                if (vehicleChoice != 1 && vehicleChoice != 2) {
                    throw InvalidInputException("vehicle type must be 1 or 2.");
                }

                readInput("Enter vehicle number: ", number);

                if (vehicleChoice == 1) {
                    Car car(number);
                    Driver driver(name, userId, &car);
                    driver.display();
                } else {
                    Bike bike(number);
                    Driver driver(name, userId, &bike);
                    driver.display();
                }

                break;
            }

            case 5:
                lot.showAll();
                break;

            case 6:
                vehiclePolymorphism();
                break;

            case 7:
                lot.showReport();
                break;

            case 8:
                exceptionDemo(lot);
                break;

            case 9:
                demonstrateMultithreading(lot);
                break;

            case 10:
                cout << "Exiting Smart Parking Management System.\n";
                break;

            default:
                throw InvalidInputException("menu choice must be between 1 and 10.");
            }
        } catch (const SpaceNotFoundException& e) {
            cout << "Space error: " << e.what() << '\n';
        } catch (const SpaceOccupiedException& e) {
            cout << "Space error: " << e.what() << '\n';
        } catch (const InvalidInputException& e) {
            cout << e.what() << '\n';
        } catch (const ParkingException& e) {
            cout << "Parking error: " << e.what() << '\n';
        } catch (const exception& e) {
            cout << "Unexpected error: " << e.what() << '\n';
        } catch (...) {
            cout << "Unknown error occurred.\n";
        }

    } while (choice != 10);
return 0;
}