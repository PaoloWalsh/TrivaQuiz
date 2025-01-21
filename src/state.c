#include "../include/state.h"

const char* stateToString(State s) {
    switch (s) {
        case UNREGISTERED: return "UNREGISTERED";
        case CHOOSING_NAME: return "CHOOSING_NAME";
        case REQUEST_THEME: return "REQUEST_THEME";
        case CHOOSING_THEME: return "CHOOSING_THEME";
        case ANSWERING: return "ANSWERING";
        default: return "UNKNOWN_STATE"; 
    }
}
