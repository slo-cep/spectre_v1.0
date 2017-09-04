# spectre_v1.0
About:
SPECTRE project is a complex event processing framework that uses the speculation to process dependent widnows of events. For detailed information, see our paper http://2017.middleware-conference.org/accepted.html .

Dependencies (Linux):
1- Install Boost C++ Library (not needed if you want to use only execuatble!)
2- Install Google Performance Tools: gperftools

Build & Run: 
SPECTRE consists of several projects: source, splitter, etc. These projects are a QT project and hence to open and build them with QT creator, you need to rename *.pro.template to *.pro.
The building results in several static libraries (one per project). These libraries are linked in the Main project that produces the excutable binary.

Note: Be sure that Main.Pro contains the correct path to the generated libraries, boost library and google library.


Usage Example:
ex._name #thread event_type operator_type initial_event_per_window pattern_size window_size tree_depth checkpointing_frequency port_number alpha event_per_matrix
 step_size num_pre_calc Maxtix_min_factor garbage_state outputfiles... 
./main 4 0 0 10000 13 10000 8 0 4000 0.700000 10000 200 120 1 false latencyDetectionFile eventsProcessedPerWorkersFile throughputFile reschedulingFrequencyFile

*To have more insight about the above parameters, please check the code. It is  well commented :)


Client Project: It is a plain c++ project. See the bash files to have an idea how run it!

Summery: To run the framework, you can either generate events randomly in the framework or use the client project to push events to the framework.



