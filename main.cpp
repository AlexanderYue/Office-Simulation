/*
Written by: Alexander Yue
Third Assignment
Cougarnet: AZYue
PID: 2079436
COSC 3360
Instructor: Professor Paris
Code uses input redirection as directed by the assignment. However, compiling on my home computer with 
"g++ main.cpp -lpthread -o hw3 -fpermissive", then running with "./main 2 <input322D.txt" 
causes an error on my home computer. However, when ran on another computer, it presents no errors and runs as normal. Please take that into mind.
*/


#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <list>
#include <cstdlib> // for atoi
using namespace std;

// Struct for Patron
struct Patron {
    string name = "";
    int arrivalDelay, serviceTime, ID;
    Patron* next; // Pointer to the next patron in the linked list
    // Constructor to initialize the patron with given values
    Patron(string x, int y, int z, int a) : name(x), arrivalDelay(y), serviceTime(z), ID(a), next(nullptr) {}
};

// Global variables
pthread_mutex_t clerkCheck = PTHREAD_MUTEX_INITIALIZER; // Mutex for controlling access to shared resources
pthread_cond_t freeClerks = PTHREAD_COND_INITIALIZER; // Condition variable for signaling when a clerk is free
static int PatronCount = 0; // Total number of patrons
static int nFreeClerks = 0; // Number of free clerks
static int waitTotal = 0; // Total number of patrons who had to wait before getting service

// Function for patron thread
void* patron_thread(void* arg) {
    Patron* argptr = static_cast<Patron*>(arg); // Cast the argument back to Patron pointer
    cout << argptr->name << " arrives at post office." << endl; // Output arrival message for the patron
    pthread_mutex_lock(&clerkCheck); // Lock the mutex to access shared resources
    
    // Wait until a clerk is available
    while (nFreeClerks == 0) {
        waitTotal++; // Increment the wait total
        pthread_cond_wait(&freeClerks, &clerkCheck); // Wait on the condition variable
    }
    nFreeClerks--; // Decrement the number of free clerks
    cout << argptr->name << " gets service." << endl; // Output message indicating service
    
    pthread_mutex_unlock(&clerkCheck); // Unlock the mutex
    
    // Simulate service time
    sleep(argptr->serviceTime);
    
    pthread_mutex_lock(&clerkCheck); // Lock the mutex again
    nFreeClerks++; // Increment the number of free clerks
    cout << argptr->name << " leaves the post office." << endl; // Output message indicating departure
    
    // Signal that a clerk is free
    pthread_cond_signal(&freeClerks);
    pthread_mutex_unlock(&clerkCheck); // Unlock the mutex
    
    pthread_exit(nullptr); // Exit the thread
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    nFreeClerks = atoi(argv[1]); // Number of clerks specified in the command line argument
    //string filename = argv[2];  Filename specified in the command line argument
    
    /* Open the file
    ifstream cin(filename);
    if (!cin) {
        cerr << "Error: Cannot open file " << filename << endl;
        return 1;
    }
    */

    // Read patron data from the file and create a linked list of patrons
    string Name = "";
    int Arrival, Service; 
    cin >> Name >> Arrival >> Service;
    Patron* head = new Patron(Name, Arrival, Service, PatronCount); // Create the first patron
    PatronCount++; // Increment the patron count
    Patron* temp = head; // Temporary pointer to traverse the linked list
    while (cin >> Name >> Arrival >> Service) {
        temp->next = new Patron(Name, Arrival, Service, PatronCount); // Create the next patron
        temp = temp->next; // Move to the next patron
        PatronCount++; // Increment the patron count
    }
    
    /* Close the file
    cin.close();
*/
    // Array to hold thread IDs
    pthread_t tid[PatronCount];

    // Display parameters
    cout << "-- The post office has today " << nFreeClerks << " clerk(s) on duty. " << endl;

    // Create threads for each patron
    Patron* iter = head;
    for (int i = 0; i < PatronCount; i++) {
        // Simulate arrival delay
        sleep(iter->arrivalDelay);
        // Create thread
        pthread_create(&tid[i], nullptr, patron_thread, static_cast<void*>(iter));
        iter = iter->next; // Move to the next patron
    }

    // Join threads
    for (int i = 0; i < PatronCount; ++i) {
        pthread_join(tid[i], nullptr);
    }

    // Output summary report
    cout << "\nSUMMARY REPORT" << endl;
    cout << PatronCount << " patron(s) went to the post office." << endl;
    cout << waitTotal << " patron(s) had to wait before getting service." << endl;
    cout << PatronCount - waitTotal << " patron(s) did not have to wait." << endl;

    return 0;
}