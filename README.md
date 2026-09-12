## Gaj
implements the cpp-async library focusing on concurrent call communication architecture

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

// then
then(()[auto v]{
    // excepted 12
    cout<<"value"<<v<<endl;}
    )(12);
// end


// ace
auto ch1=make_shared<pipe<int>>();
ace(ch1,then(()[auto v]{
                // excepted 11
    auto ch2=make_shared(pipe<int>);
}))
ch1->send(11);
// end

// select
auto ch1=make_shared<pipe<string>>();
auto ch2=make_shared<pipe<int>>();
select(
    ace(ch1,then(()[auto v]{
        // excepted hey
            cout<<"value"<<v<<endl
    })),
    ace(ch2,then(()[auto v]{
                // excepted 11
            cout<<"value"<<v<<endl
    })),
)
ch1->send("hey");
ch2->send(11);
// end

// when
auto task = when([]() { cout << "yo" << endl; }, "not succeeded :");
task(1); // invoke as many times you feel like to
// end

// when_all
auto task = when_all("unsucceed 😲", []() { cout << "crazy" << endl; },
[]() { cout << "play" << endl; });
task(1);
this_thread::sleep_for(chrono::seconds(2));
task(2); // play final method
// end
```

## Walkthrough ✊
note: the bat files are written only for windows
- use ./cpp_set to setup the meson
- pick or modify any example to play in the main 
- use ./cpp_run to run the main.cpp
