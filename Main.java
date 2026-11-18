import java.util.*;
import java.util.function.Predicate;

// INTERFACE: Classes can implement multiple interfaces in Java.
interface Reportable {
    void showReport();
}

interface Printable {
    void printDetails();
}

// CUSTOM EXCEPTION HIERARCHY
class ParkingException extends RuntimeException {
    ParkingException(String msg) { super(msg); }
}
class SpaceNotFoundException extends ParkingException {
    SpaceNotFoundException(String msg) { super(msg); }
}
class SpaceOccupiedException extends ParkingException {
    SpaceOccupiedException(String msg) { super(msg); }
}
class VehicleMismatchException extends ParkingException {
    VehicleMismatchException(String msg) { super(msg); }
}
class InvalidDurationException extends ParkingException {
    InvalidDurationException(String msg) { super(msg); }
}

// GENERICS: Registry<T> can store objects of different types.
class Registry<T> {
    private ArrayList<T> items = new ArrayList<>();

    void add(T item) { items.add(item); }

    T findIf(Predicate<T> condition) {
        for (T item : items)
            if (condition.test(item)) return item;
        return null;
    }

    int size() { return items.size(); }

    void showAll() {
        for (T item : items) System.out.println(item);
    }
}

// ABSTRACT CLASS: common blueprint for all vehicles.
abstract class Vehicle implements Printable {
    protected String number;
    protected final String type;

    Vehicle(String number, String type) {
        this.number = number;
        this.type = type;
    }

    public String getNumber() { return number; }
    public String getType() { return type; }

    // Abstract method: child classes must implement it.
    abstract void display();

    // FINAL METHOD: child classes cannot override this.
    final void showNumber() {
        System.out.println("Number: " + number);
    }
}

// INHERITANCE + METHOD OVERRIDING
class Car extends Vehicle {
    Car(String number) { super(number, "Car"); }

    @Override
    void display() { System.out.println("I am a Car."); }

    @Override
    public void printDetails() { display(); }
}

class Bike extends Vehicle {
    Bike(String number) { super(number, "Bike"); }

    @Override
    void display() { System.out.println("I am a Bike."); }

    @Override
    public void printDetails() { display(); }
}

// ENCAPSULATION: private data + public methods.
class ParkingSpace {
    private int id;
    private String type;
    private double rate;
    private boolean occupied;

    // STATIC: shared counter across all ParkingSpace objects.
    static int totalSpaces = 0;

    // PARAMETERIZED CONSTRUCTOR
    ParkingSpace(int id, String type, double rate) {
        this.id = id;
        this.type = type;
        this.rate = rate;
        occupied = false;
        totalSpaces++;
    }

    // COPY CONSTRUCTOR
    ParkingSpace(ParkingSpace other) {
        this(other.id, other.type, other.rate);
        this.occupied = other.occupied;
    }

    int getId() { return id; }
    String getType() { return type; }
    double getRate() { return rate; }
    boolean isOccupied() { return occupied; }

    void park() {
        if (occupied)
            throw new SpaceOccupiedException("Space already occupied!");
        occupied = true;
    }

    void exitParking() {
        if (!occupied)
            throw new ParkingException("Space is already free!");
        occupied = false;
    }

    @Override
    public String toString() {
        return "Space " + id + " | " + type + " | Rs." + rate
                + " | " + (occupied ? "Occupied" : "Available");
    }
}

// INHERITANCE: Driver extends User.
// INTERFACE: Driver also implements Printable.
class User {
    protected String name;
    protected int userId;

    User(String name, int userId) {
        this.name = name;
        this.userId = userId;
    }
}

class Driver extends User implements Printable {
    private Vehicle vehicle;

    Driver(String name, int id, Vehicle vehicle) {
        super(name, id);
        this.vehicle = vehicle;
    }

    @Override
    public void printDetails() {
        System.out.println("Driver: " + name + ", ID: " + userId);
        vehicle.showNumber();
        vehicle.display();
    }
}

// PARKING SESSION: stores vehicle and space references.
class ParkingSession {
    private int sessionId;
    private Vehicle vehicle;
    private ParkingSpace space;

    ParkingSession(int id, Vehicle vehicle, ParkingSpace space) {
        sessionId = id;
        this.vehicle = vehicle;
        this.space = space;
    }

    // METHOD OVERLOADING
    double calculateFee(int hours) {
        if (hours < 0)
            throw new InvalidDurationException("Hours cannot be negative!");
        return hours * space.getRate();
    }

    double calculateFee(int hours, double discount) {
        if (discount < 0 || discount > 100)
            throw new ParkingException("Invalid discount!");
        return calculateFee(hours) * (1 - discount / 100);
    }

    void displaySession() {
        System.out.println("Session: " + sessionId);
        System.out.println("Vehicle: " + vehicle.getNumber());
        System.out.println("Space: " + space.getId());
    }
}

// REPORTABLE: ParkingLot implements an interface.
class ParkingLot implements Reportable {
    private Registry<ParkingSpace> spaces = new Registry<>();

    // NESTED CLASS
    static class ParkingReport {
        void generate(int total, int available) {
            System.out.println("Total: " + total);
            System.out.println("Available: " + available);
            System.out.println("Occupied: " + (total - available));
        }
    }

    void addSpace(ParkingSpace s) { spaces.add(s); }

    ParkingSpace findSpace(int id) {
        ParkingSpace s = spaces.findIf(x -> x.getId() == id);
        if (s == null)
            throw new SpaceNotFoundException("Space not found!");
        return s;
    }

    // SYNCHRONIZED: only one thread executes this at a time.
    synchronized void parkVehicle(Vehicle v, int id, int hours) {
        ParkingSpace s = findSpace(id);

        if (!s.getType().equals(v.getType()))
            throw new VehicleMismatchException("Wrong space type!");

        s.park();

        ParkingSession session =
                new ParkingSession(id, v, s);

        session.displaySession();
        System.out.println("Fee: Rs." + session.calculateFee(hours));
        System.out.println("10% discount: Rs."
                + session.calculateFee(hours, 10));
    }

    void exitVehicle(int id) { findSpace(id).exitParking(); }

    void showAvailable() {
        spaces.showAll();
    }

    @Override
    public void showReport() {
        int available = 0;
        // Count available spaces.
        for (int i = 1; i <= ParkingSpace.totalSpaces; i++) {
            try {
                if (!findSpace(i).isOccupied()) available++;
            } catch (ParkingException e) { }
        }
        new ParkingReport().generate(ParkingSpace.totalSpaces, available);
    }
}

// THREAD: Runnable allows a task to run on a separate thread.
class ParkingTask implements Runnable {
    private String name;

    ParkingTask(String name) { this.name = name; }

    public void run() {
        System.out.println(name + " checking parking...");
    }
}

public class Main {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        ParkingLot lot = new ParkingLot();

        // CREATE PARKING SPACE OBJECTS
        lot.addSpace(new ParkingSpace(1, "Car", 50));
        lot.addSpace(new ParkingSpace(2, "Car", 50));
        lot.addSpace(new ParkingSpace(3, "Bike", 20));
        lot.addSpace(new ParkingSpace(4, "Bike", 20));

        // THREADS + start() + join()
        Thread t1 = new Thread(new ParkingTask("Thread 1"));
        Thread t2 = new Thread(new ParkingTask("Thread 2"));
        t1.start();
        t2.start();

        try {
            t1.join();
            t2.join();
        } catch (InterruptedException e) {
            System.out.println("Thread interrupted!");
        }

        // POLYMORPHISM: parent reference, child object.
        Vehicle v = new Car("GJ01AB1234");
        v.display();

        Driver d = new Driver("Aman", 101, v);
        d.printDetails();

        // MENU
        int choice = 0;
        do {
            System.out.println("\n1. Show spaces");
            System.out.println("2. Park vehicle");
            System.out.println("3. Exit parking");
            System.out.println("4. Report");
            System.out.println("5. Exit");
            System.out.print("Choice: ");

            try {
                choice = sc.nextInt();

                switch (choice) {
                    case 1:
                        lot.showAvailable();
                        break;

                    case 2:
                        System.out.print("Car(1) or Bike(2): ");
                        int type = sc.nextInt();
                        sc.nextLine();

                        System.out.print("Vehicle number: ");
                        String number = sc.nextLine();

                        // POLYMORPHISM: Vehicle reference.
                        Vehicle vehicle = (type == 1)
                                ? new Car(number) : new Bike(number);

                        System.out.print("Space ID: ");
                        int id = sc.nextInt();

                        System.out.print("Hours: ");
                        int hours = sc.nextInt();

                        lot.parkVehicle(vehicle, id, hours);
                        break;

                    case 3:
                        System.out.print("Space ID: ");
                        lot.exitVehicle(sc.nextInt());
                        System.out.println("Space freed!");
                        break;

                    case 4:
                        lot.showReport();
                        break;

                    case 5:
                        System.out.println("Goodbye!");
                        break;

                    default:
                        System.out.println("Invalid choice!");
                }

                // MULTIPLE EXCEPTION HANDLING
            } catch (InputMismatchException e) {
                System.out.println("Enter numbers only!");
                sc.nextLine();
            } catch (VehicleMismatchException e) {
                System.out.println(e.getMessage());
            } catch (ParkingException e) {
                System.out.println(e.getMessage());
            } finally {
                // FINALLY: runs after try/catch.
                System.out.println("Operation completed.");
            }

        } while (choice != 5);

        sc.close();
    }
}