# Network Switch
Implementation of basic switch operations such as traffic generation, packet scheduling, and packet transmission.
The switch uses input queues, output queues, and ISLIP scheduling algorithms with virtual output queues.
## Pragram Consists of 
    1. Source Code File : routing.cpp
    2. Sample Output file : output.txt (default)
    3. makefile : to change the permission and compile the source code file
    4. technical report
    5. README.txt

## How to run the program
    1. compile the program :
        $ make
    2. run the program :
        $ ./routing -N 8 -B 4 -p 0.5 -queue INQ -out output.txt -T 10000
        $ ./routing -N 8 -B 4 -p 0.5 -queue KOUQ -K 4 -out output.txt -T 10000
        $ ./routing -N 8 -B 4 -p 0.5 -queue ISLIP -out output.txt -T 10000

## Output Format on the console : 
    1. for INQ :
        B: 4
        T: 10000
        N: 8
        p: 0.5
        Queue Type: INQ
        Avg PD: 2.02631
        Std Dev of PD: 1.07835
        Avg link utilization: 0.500275  

    2. for KOUQ :
        B: 4
        T: 10000
        N: 8
        p: 0.5
        Queue Type: KOUQ
        Avg PD: 1.4148
        Std Dev of PD: 0.684565
        Avg link utilization: 0.403738
        Avg Drop Probability: 6.25e-05

    3. for ISLIP :
        B: 4
        T: 10000
        N: 8
        p: 0.5
        Queue Type: ISLIP
        Avg PD: 4.1038
        Std Dev of PD: 12.2021
        Avg link utilization: 0.587762   

    where,
        B = buffersize
        T = maxtimeslots
        N = switchportcount
        p = packetgenprob
        PD = packet delay

## Output format in the file : 
    N | p | Queue type | Avg PD | Std Dev of PD | Avg link utilization    

    8	0.500000	INQ	2.096721	1.130865	0.499375
    8	0.500000	KOUQ	1.414797	0.684565	0.403738
    8	0.500000	ISLIP	4.209760	14.879145	0.573275    


