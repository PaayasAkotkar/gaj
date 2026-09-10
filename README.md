## Gaj
implements the cpp-async library

## How did I started & Why 🤔?
At first when I started learning cpp I really much into unreal engine but at some point I paused on cpp and went with web-services; there I learnt Go & JS fundamentals.
Go became my go-to cause I wanted something same as cpp; also with the motivation that I never lose the practice of c++.
After a while, I came back to cpp trying to implement my Go's [Pie-Rum](#https://github.com/PaayasAkotkar/pie-rum) version in C++; but than I realize that I typically have to write the async programming the parllael commnucation and all.
I learnt about CSP, Pie-Calcules stuffs to be honest I read the set's language of those 30% I understood while 30% been covered with the help of Ai.
After that I still won't sure cause the agent both suggested queue also tho the runtime/channel.go also used the same pattern but again like I wanna have control over channel's.
Like again after these much study;yet I was not able have a picture of archiecture for the pipeline; like once I thought a tunnel or channel providing the runaway for many vechincals at one time but again like this much is enough for the start but again I would like to utilize it in something.
Then I came across meta's experimental repo [libunifex](https://github.com/facebookexperimental/libunifex) where my eyes just caught eye directly on when, when_all.
I really love the idea and started building upon; and "accidenclty" I wrote the pipe and it is being modified by the agent and also teh when and wehn_all uses teh pipe to typically invoke teh result.
Also note I know that thread and future are two different things yet I am experinting with different sutffs to get things better for day-to-day use.

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
I mean v1 was horrible don't even ask 😅
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
