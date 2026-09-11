## Gaj
implements the cpp-async library focusing on the send|recieve & hub arichtecture

## Goal 🔥
to build a library with minial installation setup & also ease to modify or use asap

## Glimpse 👀
```c++
// pipe
pipe<int> p; 
auto id = p.send(12);
auto val = p.recv();
if (val) 
    cout << "found: " << val.value() << endl;
// end

// when
auto task = when([]() { cout << "yo" << endl; }, "not succeeded :");
task.signal(); // invoke as many times you feel like to
// end

// when_all
auto task = when_all("unsucceed 😲", []() { cout << "crazy" << endl; },
[]() { cout << "play" << endl; });
task.signal();
this_thread::sleep_for(chrono::seconds(2));
task.signal(); // play final method
// end
```

## Architecure ⚙️
pipe-> Go's channel alike but more vibrant and controlable
when-> perform signal calling to function without needing to worry about race
when_all -> strict binding to function call; meaning perform this much and that's it 

## Note 📓
- both when & when_all is based upon my solely written pipe.hpp where v2 is modified by ai-agent
- The current writings was done on windows

## Walkthrough ✊
note: the bat files are written only for windows
- use ./cpp_set to setup the meson
- pick or modify any example to play in the main 
- use ./cpp_run to run the main.cpp

## Todos :checkered_flag:
- modify when & when_all with respect to memory 😅
- write schdeule
- write hub
- perform http example
