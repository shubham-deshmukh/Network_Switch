/*
Conventions :
1. input or output port indexing start from zero (0).
2. time starts from 1

Input :
1. for input, provide hypen from keyboard not from given in assignment (do not copy paste for input from assignement)
because "-" (keyboard) and "−"(assignment) are different
2. Sample Inputs
tested input : T = 10000; N = 4, queueType = "KOUQ", K = 3, B = 8;

./routing -N switchportcount -B buffersize -p packetgenprob -queue INQ -K knockout -out outputfile -T maxtimeslots

for example:
./routing -N 5 -B 6 -p 0.8 -queue ISLIP -K knockout -out outputfile -T 8000
./routing -N 8 -B 4 -p 0.5 -queue INQ -out output.txt -T 10000

1. ./routing -N 8 -B 4 -p 0.5 -queue INQ -out output.txt -T 10000

*/

# include <iostream>
# include <bits/stdc++.h>
# include <string.h>
# include <random>

using namespace std;

unsigned long long int id = 0; // global variable for packet id

class Packet {
    private :
        unsigned long long int id;
        int src_port, dest_port;
        double start_time, trans_comp_time;
    public :
        Packet(unsigned long long int id, int src_port) {
            this->id = id;
            this->src_port = src_port;
            dest_port = -1;
            start_time = -1;
            trans_comp_time = -1;
        }
        unsigned long long int getId() {return id;} // get the id of the packet
        int getSrcPort() {return src_port;} // get the source port of the packet
        int getDestPort() {return dest_port;} // get the destination port of the packet
        double getStartTime() {return start_time;} // get the start time of the packet
        double getCompletionTime() {return trans_comp_time;} // get the transmission completion time of packet
        void setDestPort(int port) {dest_port = port;} // set the destination port of the packet
        void setStartTime(double t_time) {start_time = t_time;} // set the start time of the packet
        void setCompletionTime(double t_time) {trans_comp_time = t_time;} // set the transmission completion time of the packet
};



bool sortBasedOnStartTime(Packet * &a, Packet * &b) {
    return a->getStartTime() < b->getStartTime();
}
int main(int argc, char ** argv) {
    // reading argument list from cmd

    // Initialized with default values
    int N = 8, B = 4, T = 10000;
    double p = 0.5, K = 0.6 * N;
    string queueType = "INQ", opFile = "output.txt";
    for(int i = 1; i < argc; i = i + 2) {
        if(strcmp(argv[i],"-N") == 0) sscanf(argv[i+1], "%d", &N);
        else if(strcmp(argv[i],"-B") == 0) sscanf(argv[i+1], "%d",&B);
        else if(strcmp(argv[i],"-p") == 0) sscanf(argv[i+1],"%lf",&p);
        else if(strcmp(argv[i],"-queue") == 0) queueType = argv[i+1];
        else if(strcmp(argv[i],"-K") == 0){
            sscanf(argv[i+1],"%lf",&K);
        }
        else if(strcmp(argv[i],"-out") == 0) opFile = argv[i+1];
        else if(strcmp(argv[i],"-T") == 0) sscanf(argv[i+1],"%d", &T);
    }
    // all three phases occure at the beginning of each timeslots
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);//uniform distribution between 0 and 1
    std::uniform_real_distribution<> stime(0.001, 0.01);   // uniform distributioin between 0.001 and 0.01
    std::uniform_int_distribution<> port_gen(0,N-1); // uniform distribution between 0 and N-1

    vector<queue<Packet*>> in_buffer(N), out_buffer(N); // vector of input and output buffer/queue
    // vector to store the pointers to generated packets in a timeslot,
    // and to transmitted packets in a timeslot
    vector<Packet *> queue_slot, pkt_transmitted;
    vector<vector<Packet *>> pkt_hash(N); // hash to store the generated packets in a timeslot
    vector<int> link_established(N, 0); // store the total link established in all timeslots

    if(queueType == "INQ") {

        for(int t = 1; t <= T; t++) {

        // phase 1 : traffic generation

            // each port generate a packet with prob p
            // and place the packet in the input buffer
            for(int in_port = 0; in_port < N; in_port++) {
                double ran_num = dis(gen);
                if(ran_num <= p) {
                    // selection of packet destination's port (random generation)
                    int out_port = port_gen(gen);

                    // setting start time between t + 0.001 and t + 0.01 for the generated packet
                    double start_time = (double)t + stime(gen);

                    Packet * pkt = new Packet(id++, in_port);
                    pkt->setDestPort(out_port);
                    pkt->setStartTime(start_time);

                    // enqueue the generated packet in the corresponding input buffer, if input buffer has sufficient size
                    if(in_buffer[in_port].size() < B) in_buffer[in_port].push(pkt);
                    // else discard the packet (do nothing)
                }
            }


        // phase 2: packet scheduling

            // selection of a packet from input buffers for the transmission

            // populate the hash data structure
            for(int in_port = 0; in_port < in_buffer.size(); in_port++) {
                int sz = in_buffer[in_port].size();
                if(sz) {
                    // pkt from the front of the input buffer
                    Packet * pkt = in_buffer[in_port].front();
                    int out_port = pkt->getDestPort();
                    int size = pkt_hash[out_port].size();
                    if(!size) {
                        pkt_hash[out_port].push_back(pkt);
                    }
                    else {
                        // compare with the 1st pkt's arrival time in the vector
                        if(floor(pkt->getStartTime()) == floor(pkt_hash[out_port][0]->getStartTime())) {
                            pkt_hash[out_port].push_back(pkt);
                        }
                        else if(floor(pkt->getStartTime()) < floor(pkt_hash[out_port][0]->getStartTime())) {
                            // remove all the pkts (already present in the vector) as all of them are invalid for contention
                            pkt_hash[out_port].clear();
                            // insert the pkt with less start time
                            pkt_hash[out_port].push_back(pkt);
                        }
                    }
                }
            }

            // selection of packet for transmission
            for(int out_port = 0; out_port < pkt_hash.size(); out_port++) {
                int sz = pkt_hash[out_port].size();
                if(sz) { // packets are available in the input buffer for corresponding output buffer
                    if(sz == 1) { // no contention for desired output port
                        // dequeue the packet from input buffer and place into output buffer
                        Packet * pkt = in_buffer[pkt_hash[out_port][0]->getSrcPort()].front();
                        in_buffer[pkt_hash[out_port][0]->getSrcPort()].pop();
                        if(out_buffer[out_port].size() < B) // output buffer has sufficient space
                            out_buffer[out_port].push(pkt);
                    }
                    else { // contention for desired output port
                        // select one of the packet randomly
                        std::uniform_int_distribution<> pkt_selection(0, sz - 1);
                        int selected_pkt = pkt_selection(gen);
                        Packet * pkt = in_buffer[pkt_hash[out_port][selected_pkt]->getSrcPort()].front();
                        in_buffer[pkt_hash[out_port][selected_pkt]->getSrcPort()].pop();
                        if(out_buffer[out_port].size() < B) // output buffer has sufficient space
                            out_buffer[out_port].push(pkt);
                    }
                    // link is established to the corresponding output port
                    link_established[out_port] += 1;
                }
            }


        // Phase 3: packet transmission

            // packet at the head of the output buffer/queue is transmitted
            for(int out_port = 0; out_port < out_buffer.size(); out_port++) {
                int sz = out_buffer[out_port].size();
                if(sz) { // packet is available in output buffer
                    Packet *pkt = out_buffer[out_port].front();
                    pkt->setCompletionTime(t); // set the packet trasmission completion time
                    out_buffer[out_port].pop(); // remove the packet from the output buffer
                    pkt_transmitted.push_back(pkt);
                }
            }

            // maintainance required at the end of each timeslot

            // reset the distribution
            dis.reset(); stime.reset(); port_gen.reset();
            // clear the pkt_hash
            int t_sz = pkt_hash.size();
            for(int i = 0; i < t_sz; i++) pkt_hash[i].clear();

        }

        // Computation of performance matrics


        // 1. Average Packet Delay
        double avg = 0;
        vector<int> pkt_delay; // contains the delay of transmitted packets
        int delay = 0;
        for(int i = 0; i < pkt_transmitted.size(); i++) {
            delay = floor(pkt_transmitted[i]->getCompletionTime()) - floor(pkt_transmitted[i]->getStartTime()) + 1;
            avg += delay;
            pkt_delay.push_back(delay);
        }
        avg /= pkt_transmitted.size();

        // mean of square of difference
        double mean_sq_diff = 0;
        for(int i = 0; i < pkt_delay.size(); i++) {
            mean_sq_diff += (double)(pkt_delay[i] - avg) * (pkt_delay[i] - avg);
        }
        mean_sq_diff = (double) mean_sq_diff / pkt_delay.size();

        //std deviation
        double std_dev_pkt_delay = 0;
        std_dev_pkt_delay = sqrt(mean_sq_diff);


        // 2. Average Link Utilization
        int total_links = 0;
        double avg_link_utilization = 0;
        for(int i = 0; i < link_established.size(); i++) total_links += link_established[i];

        avg_link_utilization = (double) total_links / (N * T);

        // Display the result
        cout << "B: " << B << endl;
        cout << "T: " << T << endl;
        cout << "N: " << N << endl;
        cout << "p: " << p << endl;
        cout << "Queue Type: " << queueType << endl;
        cout << "Avg PD: " << avg << endl;
        cout << "Std Dev of PD: " << std_dev_pkt_delay << endl;
        cout << "Avg link utilization: " << avg_link_utilization << endl;


        // appending the result to the output file
        ofstream fout;
        string rst = to_string(N) + "\t" + to_string(p) +"\t" + queueType + "\t" + to_string(avg) + "\t" + to_string(std_dev_pkt_delay) + "\t" + to_string(avg_link_utilization);

        fout.open(opFile, ios::app);
        if(fout) fout << endl << rst;
        fout.close();


    }
    else if(queueType == "KOUQ") {
        // correction mentioned by Ma'am
        B = K;
        double drop_prob_sum = 0;
        vector<int> knockout_port; // output ports for which more than K packets are arrived in a timeslot
        for(int t = 1; t <= T; t++) {
        // Phase 1: traffic generation
            // each port generate a packet with prob p
            for(int in_port = 0; in_port < N; in_port++) {
                double ran_num = dis(gen);
                if(ran_num <= p) {
                    // selection of packet destination's port (random generation)
                    int out_port = port_gen(gen);

                    // setting start time between t + 0.001 and t + 0.01 for the generated packet
                    double start_time = (double)t + stime(gen);

                    Packet * pkt = new Packet(id++, in_port);
                    pkt->setDestPort(out_port);
                    pkt->setStartTime(start_time);

                    // insert a packet at corresponding DestPort index in hash
                    pkt_hash[out_port].push_back(pkt);
                }
            }


        // Phase 2: packet scheduling

            for(int out_port = 0; out_port < pkt_hash.size(); out_port++) {
                int sz = pkt_hash[out_port].size();
                if(sz) { // packets are contending for corresponding output port
                    // sort the packets based on start time
                    sort(pkt_hash[out_port].begin(), pkt_hash[out_port].end(), sortBasedOnStartTime);

                    if(sz <= K) { // max of K packets are contending for a given output port
                        // queue the packets based on arrival time (with offset) at corresponding output port
                        // if sufficient space is available at the buffer

                        // sort the packets based on start time (arrival time with offset)
                        sort(pkt_hash[out_port].begin(), pkt_hash[out_port].end(), sortBasedOnStartTime);

                        for(int i = 0; i < sz; i++) {
                            if(out_buffer[out_port].size() < B) {
                                out_buffer[out_port].push(pkt_hash[out_port][i]);
                            }
                            else break;
                        }
                    }
                    else { // more than K packets are contending for a given output port

                        // select K packets randomly
                        shuffle(pkt_hash[out_port].begin(), pkt_hash[out_port].end(), default_random_engine(time(0)));

                        // 1st K packets will be random, so sort it (1st K's) and then queue in the buffer if sufficient space is available
                        sort(pkt_hash[out_port].begin(), pkt_hash[out_port].begin() + K, sortBasedOnStartTime);

                        for(int i = 0; i < K; i++) {
                            if(out_buffer[out_port].size() < B) {
                                out_buffer[out_port].push(pkt_hash[out_port][i]);
                            }
                            else break;
                        }
                        knockout_port.push_back(out_port);
                    }

                    // packets are scheduled for corresponding output port i.e. link is established
                    link_established[out_port] += 1;
                }
            }

        // Phase 3: packet transmission

            // packet at the head of the output buffer/queue is transmitted
            for(int out_port = 0; out_port < out_buffer.size(); out_port++) {
                int sz = out_buffer[out_port].size();
                if(sz) { // packet is available in output buffer
                    Packet *pkt = out_buffer[out_port].front();
                    pkt->setCompletionTime(t); // set the packet trasmission completion time
                    out_buffer[out_port].pop(); // remove the packet from the output buffer
                    pkt_transmitted.push_back(pkt);
                }
            }

            // maintainance required at the end of each timeslot

            // reset the distribution
            dis.reset(); stime.reset(); port_gen.reset();
            // clear the pkt_hash
            int t_sz = pkt_hash.size();
            for(int i = 0; i < t_sz; i++) pkt_hash[i].clear();

            drop_prob_sum += (double) knockout_port.size();
            knockout_port.clear();
        }

        // Computation of performance matrics

        // 1. Average Packet Delay
        double avg = 0;
        vector<int> pkt_delay; // contains the delay of transmitted packets
        int delay = 0;
        for(int i = 0; i < pkt_transmitted.size(); i++) {
            delay = floor(pkt_transmitted[i]->getCompletionTime()) - floor(pkt_transmitted[i]->getStartTime()) + 1;
            avg += delay;
            pkt_delay.push_back(delay);
        }
        avg /= pkt_transmitted.size();

        // mean of square of difference
        double mean_sq_diff = 0;
        for(int i = 0; i < pkt_delay.size(); i++) {
            mean_sq_diff += (double)(pkt_delay[i] - avg) * (pkt_delay[i] - avg);
        }
        mean_sq_diff = (double) mean_sq_diff / pkt_delay.size();

        //std deviation
        double std_dev_pkt_delay = 0;
        std_dev_pkt_delay = sqrt(mean_sq_diff);

        // 2. Average Link Utilization
        int total_links = 0;
        double avg_link_utilization = 0;
        for(int i = 0; i < link_established.size(); i++) {
            total_links += link_established[i];
        }

        avg_link_utilization = (double) total_links / (N * T);

        // 3. KOUQ drop probability
        double avg_drop_prob = (double) drop_prob_sum / (N*T);

        // Display the result
        cout << "B: " << B << endl;
        cout << "T: " << T << endl;
        cout << "N: " << N << endl;
        cout << "p: " << p << endl;
        cout << "Queue Type: " << queueType << endl;
        cout << "Avg PD: " << avg << endl;
        cout << "Std Dev of PD: " << std_dev_pkt_delay << endl;
        cout << "Avg link utilization: " << avg_link_utilization << endl;
        cout << "Avg Drop Probability: " << avg_drop_prob << endl;

        // appending the result to the output file
        ofstream fout;
        string rst = to_string(N) + "\t" + to_string(p) +"\t" + queueType + "\t" + to_string(avg) + "\t" + to_string(std_dev_pkt_delay) + "\t" + to_string(avg_link_utilization);

        fout.open(opFile, ios::app);
        if(fout) fout << endl << rst;
        fout.close();


    }
    else { // for ISLIP
        double lu=0;    //link utilization
        double pd=0;    //pkt delay
        int ctr=0;
        srand(time(0));
        vector<double> dt;
        vector<vector<Packet*>> inbuff(N);
        for(int t = 1; t <= T; t++) {
                //cout << "***********************************************************For Time Slot: " << t << "***********************"<<endl;

            // phase 1 : traffic generation

                // each port generate a packet with prob p
                // and place the packet in the input buffer
                for(int in_port = 0; in_port < N; in_port++) {
                    int r1=rand()%51;
                    int r2=0;
                    while(r2==0)
                        r2=rand()%3;
                    double ran_num = ((double)(r1*r2))/100;
                    if(ran_num <= p) {
                        // selection of packet destination's port (random generation)
                        int out_port = port_gen(gen);

                        // setting start time between t + 0.001 and t + 0.01 for the generated packet
                        double start_time = (double)t + stime(gen);

                        Packet * pkt = new Packet(id++, in_port);
                        pkt->setDestPort(out_port);
                        pkt->setStartTime(start_time);

                        // enqueue the generated packet in the corresponding input buffer, if input buffer has sufficient size
                        if(inbuff[in_port].size() < B) inbuff[in_port].push_back(pkt);
                        // else discard the packet (do nothing)
                    }
                }

            //phase 2 : packet scheduling
            vector<bool> ip_v(N,false),op_v(N,false);
            bool flag=true;
            map<int,int> grant;
            map<int,int> accept;
            map<int,int> att;
            while(flag==true)
            {
                flag=false;
                //grant phase
                for(int in_port = 0; in_port < N; in_port++)
                {
                    if(ip_v[in_port]==true) continue;
                    vector<Packet*> pc=inbuff[in_port];
                    for(int op_port=0;op_port<pc.size();op_port++)
                    {
                        int dest=pc[op_port]->getDestPort();
                        int source=pc[op_port]->getSrcPort();
                        if(op_v[dest]==false)
                        {
                            op_v[dest]=true;
                            grant[dest]=source;
                        }
                    }
                }
                //accept phase
                for(auto it:grant)
                {
                    int src=it.second;
                    if(ip_v[src]==false)
                    {
                        ip_v[src]=true;
                        accept[src]=it.first;
                    }
                }
                //removing accepted request
                for(auto it:accept)
                {
                    vector<Packet*>::iterator it2;
                    it2=inbuff[it.first].begin();
                    while(it2!=inbuff[it.first].end())
                    {
                        int dest=(*it2)->getDestPort();
                        if(dest==it.second)
                        {
                            ctr++;
                            //cout<<(double)(1+abs((int)(*it2)->getStartTime()-(double)t))<<endl;
                            pd+=(double)(1+abs((int)(*it2)->getStartTime()-(double)t));
                            dt.push_back((double)(abs((int)(*it2)->getStartTime()-(double)t)));
                            (inbuff[it.first]).erase(it2);
                            break;
                        }
                        it2++;
                    }


                }
                att.insert(accept.begin(),accept.end());
                ip_v.resize(N,false);
                op_v.resize(N,false);
                for(auto it:att)
                {
                    ip_v[it.first]=true;
                    op_v[it.second]=true;
                }
                //check if other iteration is needed
                for(int i=0;i<N;i++)
                {
                    if(ip_v[i]==false)
                    {
                        for(int j=0;j<inbuff[i].size();j++)
                        {
                            int dest=inbuff[i][j]->getDestPort();
                            if(op_v[dest]==false)
                                flag=true;
                        }
                    }
                }

            }
            int r=8;
            accept.clear();
            grant.clear();
        }
        pd=pd/ctr;
        double sd=0;
        for(int i=0;i<dt.size();i++)
            sd+=pow(dt[i]-pd,2);
        sd=sqrt(sd/ctr);
        // cout<<"ctr="<<ctr<<endl;
        
        double avg_link_utilization = ((double)ctr/(double)(N*T));
        // cout<<"Link utilization: "<<avg_link_utilization<<endl;
        // cout<<"Pkt delay: "<<pd<<endl;
        // cout<<"Standard deviation: "<<sd<<endl;

        // Display the result
        cout << "B: " << B << endl;
        cout << "T: " << T << endl;
        cout << "N: " << N << endl;
        cout << "p: " << p << endl;
        cout << "Queue Type: " << queueType << endl;
        cout << "Avg PD: " << pd << endl;
        cout << "Std Dev of PD: " << sd << endl;
        cout << "Avg link utilization: " << avg_link_utilization << endl;

        // appending the result to the output file
        ofstream fout;
        string rst = to_string(N) + "\t" + to_string(p) +"\t" + queueType + "\t" + to_string(pd) + "\t" + to_string(sd) + "\t" + to_string(avg_link_utilization);

        fout.open(opFile, ios::app);
        if(fout) fout << endl << rst;
        fout.close();

    }


    return 0;
}
