#include "door_fsm.h"

door_state_t door_fsm_trigger(door_state_t current)
{
    switch (current) {
        case DOOR_STATE_CLOSED:          return DOOR_STATE_OPENING;
        case DOOR_STATE_OPENING:         return DOOR_STATE_STOPPED_OPENING;
        case DOOR_STATE_STOPPED_OPENING: return DOOR_STATE_CLOSING;
        case DOOR_STATE_CLOSING:         return DOOR_STATE_STOPPED_CLOSING;
        case DOOR_STATE_STOPPED_CLOSING: return DOOR_STATE_OPENING;
        case DOOR_STATE_OPEN:            return DOOR_STATE_CLOSING;
        default:                         return DOOR_STATE_UNKNOWN;
    }
}

door_fsm_result_t door_fsm_process_command(door_state_t current, door_command_t cmd)
{
    door_fsm_result_t result = {.trigger = false, .next = current};

    switch (cmd) {
        case DOOR_CMD_OPEN:
            if (current == DOOR_STATE_CLOSED ||
                current == DOOR_STATE_STOPPED_CLOSING) {
                result.trigger = true;
                result.next    = door_fsm_trigger(current);
            }
            break;

        case DOOR_CMD_CLOSE:
            if (current == DOOR_STATE_OPEN ||
                current == DOOR_STATE_STOPPED_OPENING) {
                result.trigger = true;
                result.next    = door_fsm_trigger(current);
            }
            break;

        case DOOR_CMD_TOGGLE:
            result.trigger = true;
            result.next    = door_fsm_trigger(current);
            break;
    }

    return result;
}

door_state_t door_fsm_apply_sensor(door_state_t current, door_state_t sensor)
{
    if (sensor == DOOR_STATE_OPEN || sensor == DOOR_STATE_CLOSED) {
        return sensor;
    }
    return current;
}

const char *door_state_to_str(door_state_t state)
{
    switch (state) {
        case DOOR_STATE_OPEN:            return "open";
        case DOOR_STATE_CLOSED:          return "closed";
        case DOOR_STATE_OPENING:         return "opening";
        case DOOR_STATE_CLOSING:         return "closing";
        case DOOR_STATE_STOPPED_OPENING: return "stopped_opening";
        case DOOR_STATE_STOPPED_CLOSING: return "stopped_closing";
        default:                         return "unknown";
    }
}