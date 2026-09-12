#pragma  once
#include "pipe.hpp"
#include"when.hpp"
#include "then.hpp"
#include "select.hpp"
#include "record.hpp"

// study
// barrier: -> helpful when, if all the process focus on calcuating one frame
// latch: -> helpful in the scenario like; meaning it keeps the process open
// once's it is hit
// end

// pipe: concurrent commnucation channel
// record: minimal concurrent hash-map [todo]
// select: implements the shared-concurrent commnucation
// then: implements the param method for fetching the value
// trait: core class followed by the rest [todo]
// types: required classes
// when: manual conccurent calls