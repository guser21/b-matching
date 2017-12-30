# b-matching
concurrent b-matching algorithm 

Here you can find an efficient implementation of approximation algorithm of b-matching problem.
The original source of the paper: https://www.cs.purdue.edu/homes/apothen/Papers/bMatching-SISC-2016.pdf

I have tested my program using different configurations. 

For the synchronization of the suitor set I tried several options: std::mutex, std::shared_mutex, and a custom  spinlock implementation with atomic_flag variable.  To my surprise std::mutex outperformed the std::shared_mutex which is a standard readers-writers lock. My best guess is that, the chance of having two threads to run on the same node is really low. The spinlock and the mutex performed similarly so I am sticking with mutexes. This can be explained that the mutexes in practice do not switch context immediately, at first they behave like a spinlock and wait actively.

For the scheduling I tried 3 different tools: bare threads from std::thread, std::async with async policy, and caching threads with custom threadpool. None of those really outperformed the others (difference was about a second or 2) . The only rationale: it is not that expensive to create user threads in linux. 

I have run the tests on my laptop. The input graph was “Road network of Pennsylvania” from
http://snap.stanford.edu/data/ ,run for 20 different b -value configurations. 
Here you can find the processor details.


Thread(s) per core:  2

Core(s) per socket:  4

Socket(s):           1


Model name:          Intel(R) Core(TM) i7-2630QM CPU @ 2.00GHz

CPU MHz:             1995.628

threads- time of pure computation excluded the input processing.  

1 -116s  4 - 40s        40-34s
2 - 71s  8- 32s 
3-  52s  16-35s   
