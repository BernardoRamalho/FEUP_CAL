#pragma once
#ifndef MEAT_WAGONS_APPLICATION_H
#define MEAT_WAGONS_APPLICATION_H
#include <sstream>
#include <fstream>
#include "../MeatWagons/MeatWagons.h"

void readline(string &str) {
    str.clear();
    cin.clear();
    fflush(stdin);
    getline(cin, str);
    while (str.empty()) getline(cin, str);
}
int stoint(const string &str, int &value) {
  // wrapping stoi because it may throw an exception
  try {
    value = stoi(str, nullptr, 10);
    return 0;
  }
  catch (const invalid_argument &ia) { return -1; }
  catch (const out_of_range &oor) { return -2; }
  catch (const exception &e) { return -3; }
}

class Application {
    private:
        MeatWagons *controller = new MeatWagons(10, 100);
        GraphVisualizer *viewer = nullptr;

    public:
        void run();
        void displayMenu();
};


void Application::displayMenu()
{
    cout << "\b---------------------------------------------------" << endl;
    cout << "Menu Options: " << endl;
    cout << "\t1 - Read Graph" << endl;
    cout << "\t2 - Pre Process" << endl;
    cout << "\t3 - Shortest Path (Dijkstra) <{Normal | Oriented | Bidirectional}> <origin node> <destination node>" << endl;
    cout << "\t4 - Deliver (Iteration: <{1 | 2 | 3}>)"<< endl;
    cout << "\t5 - Set Central <node id>"<< endl;
    cout << "\t6 - Wagon Operation <{list | add | remove}> <operands>"<< endl;
    cout << "\t7 - Requests Operation <{list | add | remove}> <operands>"<< endl;
    cout << "\t0 - Exit" << endl << endl;
        
    if(controller->getGraph() == nullptr) cout << "Graph not read yet!" << endl;
    else cout << "Graph read for: '" << controller->getGraphName() << "'" << endl;

    cout << "\b> ";
}


void Application::run() 
{
    int option;
    string argument, input;

    displayMenu();
    while(true) {
        readline(input);
        if(stoint(input, option) == 0 && option >= 0 && option <= 7) 
            break;
        else cout << "\b> ";
    }

    switch (option) {
        // curly brackets are needed to initialize new variables in case scopes
        case 0:
          cout << endl << "Exiting";
          exit(0);

        case 1: {
            cout << "\n--- Reading Graph ---";
            cout << "\nProvide the following: <graph & requests folder path> (Type '0' or 'back' to go back)";
            cout << "\nExample: 'maps/PortugalMaps/Porto'\n> ";

            bool back = false;
            readline(input);

            while(true) {
                if(input == "0" || input == "back") {
                    back = true;
                    break;
                }
                else if(strstr(strdup(input.c_str()), "maps/") == nullptr) {
                    cout << "\nTry again\n> ";
                    readline(input);
                }
                else break;
            }
            if(back) break;

            stringstream line(input);
            controller->setGraph(line.str());

            stringstream name;
            for (size_t i = line.str().find_last_of('/') + 1; i < line.str().size(); ++i)
                name << line.str().at(i);

            controller->setGraphName(name.str());

            break;
        }

        case 2: {
            cout << "\n--- Processing node ---";
            cout << "\nProvide the following: <node id>\n(Type '0' or 'back' to go back)\n> ";

            int node;
            bool back = false;
            readline(input);

            while(true) {
                if(input == "0" || input == "back") {
                    back = true;
                    break;
                }
                else if(stoint(input, node) != 0 || node < 0) {
                    cout << "\nTry again\n> ";
                    readline(input);
                }
                else break;
            }
            if(back) break;

            stringstream line(input);
            if (!(line >> node)) controller->preProcess(controller->getCentral());
            else controller->preProcess(node);

            break;
        }

        case 3: {
            cout << "\n--- Finding Shortest Path ---";
            cout << "\nProvide the following: <Normal | Oriented |Bidirectional> <origin node> <destination node> (Type '0' or 'back' to go back)";
            cout << "\nExample: 'Normal 90379359 411018963'\n> ";

            string variant;
            int origin, dest;

            while(true) {
                readline(input);
                if(input == "0" || input == "back") break;

                stringstream line(input);
                if (line >> variant && line >> origin && line >> dest) 
                {
                    if (variant == "Normal" || variant == "N") {
                        controller->shortestPath(1, origin, dest);
                        break;
                    }
                    else if (variant == "Oriented" || variant == "O") {
                        controller->shortestPath(2, origin, dest);
                        break;
                    }
                    else if (variant == "Bidirectional" || variant == "B") {
                        controller->shortestPath(3, origin, dest);
                        break;
                    }
                    else cout << "\nTry again\n> ";                    
                }
                else cout << "\nTry again\n> ";
            }

            break;
        }

        case 4: {
            cout << "\n--- Delivering ---";
            cout << "\nProvide the following: <delivery id>";
            cout << "\n(Type '0' or 'back' to go back)\n> ";

            int iteration;
            bool back = false;

            while(true) {
                readline(input);
                if(input == "0" || input == "back") {
                    back = true;
                    break;
                }
                else if(input == "1" || input == "2" || input == "3")
                    cout << "\nTry again\n> ";
            }
            if(back) break;
            

            stringstream line(input);
            if(line >> iteration) {
                cout << "This operation is not ready yet!\n";
                exit(0);
                // controller->deliver(iteration);
            }

            break;
        }

        case 5: {
            if(controller->getGraph() == nullptr) {
                cout << "No graph has been read yet!\n";
                break;
            }
            cout << "\n--- Setting Central Node ---";
            cout << "\nProvide the following: <node id>";
            cout << "\n(Type '0' or 'back' to go back)\n> ";

            int centralID;
            bool back = false;

            while(true) {
                readline(input);
                if(input == "0" || input == "back") {
                    back = true;
                    break;
                }
                else if(stoint(input, centralID)) cout << "\nTry again\n> ";
            }
            if(back) break;

            stringstream line(input);
            if (line >> centralID) controller->setCentral(centralID);

            break;
        }

        case 6: {
            cout << "\n--- Wagon Operation ---";
            cout << "\nProvide the following: <list | add | remove> <operands>";
            cout << "\n(Type '0' or 'back' to go back)\n> ";
            cout << "\n> ";

            readline(input); if(input == "0") break;
            stringstream line(input);
            string variant;

            if(line >> variant) {
                if(variant == "list") controller->listWagons();
                else if(variant == "add") {
                    int capacity;
                    if (line >> capacity) controller->addWagon(capacity);
                }
                else if(variant == "remove") {
                    int id, capacity;
                    if (line>> id && line >> capacity) controller->removeWagon(id, capacity);
                }
            }
            break;
        }

        case 7: {
            cout << "\n--- Requests ---";
            cout << "\nProvide the following <{1,2,3}>";
            cout << "\n(Type '0' or 'back' to go back)\n> ";
            cout << "\n> ";

            readline(input); if(input == "0") break;
            stringstream line(input);
            string variant;
            
            if(line >> variant) {
                if(variant == "list") controller->listRequests();
                else if(variant == "add") {
                    string prisoner;
                    int dest, priority, hour, minute, second;
                    if (line >> prisoner >> dest >> priority >> hour >> minute >> second)
                        controller->addRequest(prisoner, dest, priority, Time(hour, minute, second));
                }
                else if(variant == "remove") {
                    string prisoner;
                    int dest, priority, hour, minute, second;
                    if (line >> prisoner >> dest >> priority >> hour >> minute >> second)
                        controller->removeRequest(prisoner, dest, priority, Time(hour, minute, second));
                }
            }
            break;
        }

        default:
            break;
    }
}

#endif //MEAT_WAGONS_APPLICATION_H
