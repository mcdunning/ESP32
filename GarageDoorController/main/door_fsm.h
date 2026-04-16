#pragma once

#include <stdbool.h>

/**
 * @brief All possible door states.
 *
 * Stable positions: OPEN, CLOSED
 * Active transitions: OPENING, CLOSING
 * Interrupted transitions: STOPPED_OPENING, STOPPED_CLOSING
 * Indeterminate: UNKNOWN
 */
typedef enum {
    DOOR_STATE_UNKNOWN = 0,
    DOOR_STATE_OPEN,
    DOOR_STATE_CLOSED,
    DOOR_STATE_OPENING,
    DOOR_STATE_CLOSING,
    DOOR_STATE_STOPPED_OPENING,
    DOOR_STATE_STOPPED_CLOSING,
} door_state_t;

typedef enum {
    DOOR_CMD_OPEN,
    DOOR_CMD_CLOSE,
    DOOR_CMD_TOGGLE,
} door_command_t;

/** @brief Result of processing a command against the current state. */
typedef struct {
    bool         trigger;   /* true = relay should fire          */
    door_state_t next;      /* state to adopt after the decision */
} door_fsm_result_t;

/**
 * @brief Returns the state that follows a relay trigger from @p current.
 *
 * Cycle:
 *   CLOSED → OPENING → STOPPED_OPENING → CLOSING → STOPPED_CLOSING → OPENING
 *   OPEN   → CLOSING
 *   UNKNOWN → UNKNOWN
 */
door_state_t door_fsm_trigger(door_state_t current);

/**
 * @brief Determines whether a command fires the relay and the resulting state.
 *
 *   OPEN  — fires only from CLOSED or STOPPED_CLOSING
 *   CLOSE — fires only from OPEN or STOPPED_OPENING
 *   TOGGLE — always fires
 */
door_fsm_result_t door_fsm_process_command(door_state_t current, door_command_t cmd);

/**
 * @brief Applies a sensor reading to the current state.
 *        If the sensor reports OPEN or CLOSED the returned state reflects that;
 *        otherwise the current state is unchanged.
 */
door_state_t door_fsm_apply_sensor(door_state_t current, door_state_t sensor);

/** @brief Convert a door_state_t to its MQTT payload string. */
const char *door_state_to_str(door_state_t state);