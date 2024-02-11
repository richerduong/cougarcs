#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <math.h>
#include <sstream>
using namespace std;

struct task{
    string cpu;
    string s;
    int count;
    char taskName;
    int wcet;
    int period;
    //overriding priority queue statement
    bool operator>(const task& rhs)const {
        // cout << "comparing: " << taskName << " " << rhs.taskName << endl;
        if(period > rhs.period){
            //  cout << " swapping: " << period << " " << rhs.taskName << endl;
            return true;
        }
        else if(period == rhs.period){
            if(taskName > rhs.taskName){
                // cout << " swapping: " << taskName << " " << rhs.taskName << endl;
                return true;
            }
        }
        return false;
    }
};

//function to get greatest common divisor 
int gcd(int a, int b){
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

//function to calculate the least common multiple of only two numbers
int lcm(int a, int b){
    return (a * b) / gcd(a, b);
}

//function to calculate the least common multiple of a vector of numbers
int calculateLCM(const vector<int>& numbers) {
    if (numbers.empty()){
        return 0; 
    }

    int result = numbers[0];

    for (size_t i = 1; i < numbers.size(); ++i){
        result = lcm(result, numbers[i]);
    }

    return result;
}

//actually doing the threading part
void * threadAction(void *x_void_ptr){
    
	task *x_ptr = (task *)x_void_ptr;
    
    cout << x_ptr->s << endl;
	x_ptr->s += "CPU " + to_string(x_ptr->count+1);
	x_ptr->s += "\nTask scheduling information: ";
	
	//create vector of tasks information
	vector<task> tasks;
	
    //for task utilization
    double oneTask = 0; 
    double setUtilization = 0;
    
    //creating a vector to store the lcm
    vector<int> allPeriods;
	
	stringstream ss(x_ptr->cpu);
	
    //trying to store everything in vector of tasks
    task eachTask;
    while (ss >> eachTask.taskName >> eachTask.wcet >> eachTask.period) {
	    //push information back into the vector of tasks
        tasks.push_back(eachTask);
        
        //storing all the periods for 
        allPeriods.push_back(eachTask.period);

        x_ptr->s += string(1, eachTask.taskName) + " (WCET: " + to_string(eachTask.wcet) + ", Period: " + to_string(eachTask.period) + "), ";
    
        //task utilization 
        //this is only dividing the wcet by the period for one task
        oneTask = static_cast<double>(eachTask.wcet)/static_cast<double>(eachTask.period);
        
        //adding all of them together to get the overall task set utilization
        setUtilization = setUtilization + oneTask;
        
    }
    //getting rid of the last space and comma
    x_ptr->s.pop_back();
    x_ptr->s.pop_back();
    
    x_ptr->s += "\nTask set utilization: ";

    //use oss to get setprecision to 2
    ostringstream oss;
    oss << fixed << setprecision(2) << setUtilization;
    x_ptr->s += oss.str();
    
    int hyperperiod; 
    hyperperiod = calculateLCM(allPeriods);
   
    x_ptr->s += "\nHyperperiod: " + to_string(hyperperiod) + "\nRate Monotonic Algorithm execution for CPU" + to_string(x_ptr->count+1) + ":"; 
    
    // sort tasks based on period, if same period, then by ASCII
    priority_queue<task, vector<task>, greater<task> > min_queue;
    // cout<<tasks.size()<<endl;
    string scheduling = "";
    double setCheck = 0.0;
    
    setCheck = tasks.size() * (pow(2, (double)1 / tasks.size()) - 1);
    // cout<<setUtilization << "<=" << setCheck <<endl;
    
    if (setUtilization >= 1){
        scheduling += "\nTask set schedulability is not schedulable";
    } 
    else if (setUtilization > setCheck) {
        scheduling += "\nTask set schedulability is unknown";
    } 
    else{
        int useTime = 0;
        x_ptr->s += "\nScheduling Diagram for CPU " + to_string(x_ptr->count+1) + ":";
        for(int curtime = 0; curtime < hyperperiod; curtime++){
            for(int j = 0; j < tasks.size(); j++){
                if(curtime % tasks[j].period == 0){
                    if(curtime != 0){
                        
                        if(useTime > 0){
                            if(min_queue.empty()){
                               scheduling += "Idle (" + to_string(useTime) + "), ";
                            }
                            else{
                                scheduling += " " + string(1, min_queue.top().taskName) + "(" + to_string(useTime) + "), ";
                            }
                            
                        }
                       useTime = 0;     
                            
                        
                        }
                    min_queue.push(tasks[j]);
                }
            }
            if(!min_queue.empty()){
                task tempTask = min_queue.top();
                tempTask.wcet--;
                min_queue.pop();
                min_queue.push(tempTask);
                useTime++;
                if(min_queue.top().wcet == 0){
                    scheduling += " " + string(1, min_queue.top().taskName) + "(" + to_string(useTime) + "), ";
                    min_queue.pop();
                    useTime = 0;
                }
            }
            else{
                useTime++; //this tells you that you've done one second of doing nothing
                
            }
        }
        if(min_queue.empty()){
            scheduling += "Idle (" + to_string(useTime) + "), ";
        }
        else{
            scheduling += string(1, min_queue.top().taskName) + "(" + to_string(useTime) + "), ";
        }
        scheduling.pop_back();
        scheduling.pop_back();
    }
    
        x_ptr->s += scheduling;
   
    x_ptr->s += "\n";
	return NULL;
}


int main(){
    
    //store the input into element  of vector name cpus
    vector <task> cpus;
    
    //receive standard input
    string userInput;

    int count = 0; 
    for (int i=0; i<4; i++){
        getline(cin, userInput);
        task temp; 
        temp.cpu = userInput;
        temp.count = count++;
        //storing each cpu's information into the vector cpus
        cpus.push_back(temp);
    }
    /*
    while(getline(cin, userInput)){
        
        task temp; 
        temp.cpu = userInput;
        temp.count = count++;
        //storing each cpu's information into the vector cpus
        cpus.push_back(temp);
       
    }
    */
    //number of threads using size of vector
    int NTHREADS = cpus.size();
    
    //declaring an array of pthread_t type of size NTHREADS
    //pthread is thread id
    pthread_t tid[NTHREADS];
    
    //creating threads using a for loop that runs NTHREADS time
    for (int i = 0; i < NTHREADS; i++){
        if (pthread_create(&tid[i], NULL, threadAction, (void *) &cpus[i]) != 0){
			
			cerr << "Error creating thread" << endl;
			return 1;
			
		}
	}
    
	for (int i = 0; i < NTHREADS; i++){
		pthread_join(tid[i], NULL);
	}
	for (int i = 0; i < NTHREADS; i++) {
        cout << cpus[i].s << endl;
    }
    
    return 0;
}