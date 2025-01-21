#ifndef STATE_H
#define STATE_H

typedef enum State {
    UNREGISTERED,
    CHOOSING_NAME,
    REQUEST_THEME,
    CHOOSING_THEME,
    ANSWERING,
} State;

const char* stateToString(State s);

#endif
