#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>

using namespace std;

class processes {
	int PID, e_time, prior;
	bool split;

public:
	processes() {}
	~processes() {}
	processes(int PID, int e_time, int prior, bool split) {
		this->PID = PID;
		this->e_time = e_time;
		this->prior = prior;
		this->split = split;
	}

	processes(const processes &s) {
		PID = s.PID;
		e_time = s.e_time;
		prior = s.prior;
		split = s.split;
	}

	int getPID() { return PID; }
	int getExec() { return e_time; }
	void setExec(int newExec) { e_time = newExec; }
	int getPrior() { return prior; }
	bool getSplit() { return split; }
	void setSplit(bool isSplit) { split = isSplit; }

	friend ostream& operator<<(ostream &out, const processes &pr) {
		out << "Process " << pr.PID << ": exec time = "
			<< pr.e_time << ", priority = " << pr.prior << endl;
		return out;
	}
};


int main() {
	vector<processes> temp; // to keep second split process
	vector<processes> v; 
	int quant = 0, pid = 0, exe_time = 0, priority = 0;

	cin >> quant;
	while(cin >> exe_time >> priority) {
		if(!temp.empty()) { // checking split process priority
			if(temp.front().getPrior() < priority) {
				v.push_back(temp.front());
				temp.erase(temp.begin());
			}
		}

		if(exe_time > quant) { // execution time greater than quantum
			processes p(pid, quant, priority, true);
			v.push_back(p);
			// second split process
			p.setExec(exe_time - quant);
			p.setSplit(false);
			temp.push_back(p);
		}
		else {
			processes p(pid, exe_time, priority, false);
			v.push_back(p);
		}
		pid++;	
	}

	if(!temp.empty()) {
		for(int i = 0; i < temp.size(); ++i)
			v.push_back(temp[i]);
	}
	temp.clear();

	queue<processes> q; // creating scheduling queue
	while(!v.empty()) {
		int min = 0; // index with min priority
		for (int i = 0; i < v.size(); ++i) {
			if(v[min].getPrior() > v[i].getPrior())
				min = i;
		}
		q.push(v[min]);

		// handle same priority clusters
		if(v.size() > 1 && v[min].getPrior() == v[min+1].getPrior()) {
			int count = 1;
			for(int j = min + 1; j < v.size(); j++) {
				if(v[j].getPrior() == v[min].getPrior()) {
					q.push(v[j]);
					count++;
				}
				else
					break;
			}
			v.erase(v.begin()+min, v.begin()+(min+count));
		}
		else
			v.erase(v.begin()+min);
	}
	v.clear();

	queue<processes> final = q;
	cout << "Scheduling queue:\n\t";
	while(!q.empty()) {
		cout << "(" << q.front().getPID() << "," << q.front().getExec() << "," << q.front().getPrior();
		if(q.size() == 1) cout << ")\n\n";
		else cout << "),";
		q.pop();
	}

	pid_t atpid;
	int num_p = final.size(); // needs to create childs for every process
	for(int i = 0; i < num_p; i++) { // even split processes
		if((atpid = fork()) == 0) {
			cout << final.front();
			this_thread::sleep_for(chrono::seconds(final.front().getExec()));
			if(!final.front().getSplit()) 
				cout << "Process " << final.front().getPID() << " ends.\n";
			break;
		}
		else
			wait(NULL);

		final.pop();
	}
		
	return 0;
}
