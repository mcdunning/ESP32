#include "unity.h"
#include "door_fsm.h"

void setUp(void) {}
void tearDown(void) {}

/* ── door_fsm_trigger ────────────────────────────────────────────────── */

void test_trigger_from_closed_goes_to_opening(void)
{
    TEST_ASSERT_EQUAL(DOOR_STATE_OPENING, door_fsm_trigger(DOOR_STATE_CLOSED));
}

void test_trigger_from_opening_goes_to_stopped_opening(void)
{
    TEST_ASSERT_EQUAL(DOOR_STATE_STOPPED_OPENING, door_fsm_trigger(DOOR_STATE_OPENING));
}

void test_trigger_from_stopped_opening_goes_to_closing(void)
{
    TEST_ASSERT_EQUAL(DOOR_STATE_CLOSING, door_fsm_trigger(DOOR_STATE_STOPPED_OPENING));
}

void test_trigger_from_closing_goes_to_stopped_closing(void)
{
    TEST_ASSERT_EQUAL(DOOR_STATE_STOPPED_CLOSING, door_fsm_trigger(DOOR_STATE_CLOSING));
}

void test_trigger_from_stopped_closing_goes_to_opening(void)
{
    TEST_ASSERT_EQUAL(DOOR_STATE_OPENING, door_fsm_trigger(DOOR_STATE_STOPPED_CLOSING));
}

void test_trigger_from_open_goes_to_closing(void)
{
    TEST_ASSERT_EQUAL(DOOR_STATE_CLOSING, door_fsm_trigger(DOOR_STATE_OPEN));
}

void test_trigger_from_unknown_stays_unknown(void)
{
    TEST_ASSERT_EQUAL(DOOR_STATE_UNKNOWN, door_fsm_trigger(DOOR_STATE_UNKNOWN));
}

/* ── OPEN command ────────────────────────────────────────────────────── */

void test_open_cmd_from_closed_fires_trigger(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_CLOSED, DOOR_CMD_OPEN);
    TEST_ASSERT_TRUE(r.trigger);
    TEST_ASSERT_EQUAL(DOOR_STATE_OPENING, r.next);
}

void test_open_cmd_from_stopped_closing_fires_trigger(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_STOPPED_CLOSING, DOOR_CMD_OPEN);
    TEST_ASSERT_TRUE(r.trigger);
    TEST_ASSERT_EQUAL(DOOR_STATE_OPENING, r.next);
}

void test_open_cmd_from_open_does_not_fire(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_OPEN, DOOR_CMD_OPEN);
    TEST_ASSERT_FALSE(r.trigger);
    TEST_ASSERT_EQUAL(DOOR_STATE_OPEN, r.next);
}

void test_open_cmd_from_opening_does_not_fire(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_OPENING, DOOR_CMD_OPEN);
    TEST_ASSERT_FALSE(r.trigger);
    TEST_ASSERT_EQUAL(DOOR_STATE_OPENING, r.next);
}

void test_open_cmd_from_stopped_opening_does_not_fire(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_STOPPED_OPENING, DOOR_CMD_OPEN);
    TEST_ASSERT_FALSE(r.trigger);
}

void test_open_cmd_from_closing_does_not_fire(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_CLOSING, DOOR_CMD_OPEN);
    TEST_ASSERT_FALSE(r.trigger);
}

/* ── CLOSE command ───────────────────────────────────────────────────── */

void test_close_cmd_from_open_fires_trigger(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_OPEN, DOOR_CMD_CLOSE);
    TEST_ASSERT_TRUE(r.trigger);
    TEST_ASSERT_EQUAL(DOOR_STATE_CLOSING, r.next);
}

void test_close_cmd_from_stopped_opening_fires_trigger(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_STOPPED_OPENING, DOOR_CMD_CLOSE);
    TEST_ASSERT_TRUE(r.trigger);
    TEST_ASSERT_EQUAL(DOOR_STATE_CLOSING, r.next);
}

void test_close_cmd_from_closed_does_not_fire(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_CLOSED, DOOR_CMD_CLOSE);
    TEST_ASSERT_FALSE(r.trigger);
    TEST_ASSERT_EQUAL(DOOR_STATE_CLOSED, r.next);
}

void test_close_cmd_from_closing_does_not_fire(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_CLOSING, DOOR_CMD_CLOSE);
    TEST_ASSERT_FALSE(r.trigger);
    TEST_ASSERT_EQUAL(DOOR_STATE_CLOSING, r.next);
}

void test_close_cmd_from_stopped_closing_does_not_fire(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_STOPPED_CLOSING, DOOR_CMD_CLOSE);
    TEST_ASSERT_FALSE(r.trigger);
}

void test_close_cmd_from_opening_does_not_fire(void)
{
    door_fsm_result_t r = door_fsm_process_command(DOOR_STATE_OPENING, DOOR_CMD_CLOSE);
    TEST_ASSERT_FALSE(r.trigger);
}

/* ── TOGGLE command ──────────────────────────────────────────────────── */

void test_toggle_always_fires_from_any_state(void)
{
    door_state_t all_states[] = {
        DOOR_STATE_UNKNOWN,
        DOOR_STATE_OPEN,
        DOOR_STATE_CLOSED,
        DOOR_STATE_OPENING,
        DOOR_STATE_CLOSING,
        DOOR_STATE_STOPPED_OPENING,
        DOOR_STATE_STOPPED_CLOSING,
    };
    for (int i = 0; i < 7; i++) {
        door_fsm_result_t r = door_fsm_process_command(all_states[i], DOOR_CMD_TOGGLE);
        TEST_ASSERT_TRUE_MESSAGE(r.trigger, door_state_to_str(all_states[i]));
    }
}

/* ── Sensor application ──────────────────────────────────────────────── */

void test_sensor_open_overrides_any_state(void)
{
    door_state_t all_states[] = {
        DOOR_STATE_UNKNOWN, DOOR_STATE_CLOSED, DOOR_STATE_OPENING,
        DOOR_STATE_CLOSING, DOOR_STATE_STOPPED_OPENING, DOOR_STATE_STOPPED_CLOSING,
    };
    for (int i = 0; i < 6; i++) {
        TEST_ASSERT_EQUAL(DOOR_STATE_OPEN,
            door_fsm_apply_sensor(all_states[i], DOOR_STATE_OPEN));
    }
}

void test_sensor_closed_overrides_any_state(void)
{
    door_state_t all_states[] = {
        DOOR_STATE_UNKNOWN, DOOR_STATE_OPEN, DOOR_STATE_OPENING,
        DOOR_STATE_CLOSING, DOOR_STATE_STOPPED_OPENING, DOOR_STATE_STOPPED_CLOSING,
    };
    for (int i = 0; i < 6; i++) {
        TEST_ASSERT_EQUAL(DOOR_STATE_CLOSED,
            door_fsm_apply_sensor(all_states[i], DOOR_STATE_CLOSED));
    }
}

void test_sensor_unknown_preserves_current_state(void)
{
    TEST_ASSERT_EQUAL(DOOR_STATE_OPENING,
        door_fsm_apply_sensor(DOOR_STATE_OPENING, DOOR_STATE_UNKNOWN));
    TEST_ASSERT_EQUAL(DOOR_STATE_CLOSING,
        door_fsm_apply_sensor(DOOR_STATE_CLOSING, DOOR_STATE_UNKNOWN));
}

/* ── State-to-string ─────────────────────────────────────────────────── */

void test_state_to_str_returns_correct_strings(void)
{
    TEST_ASSERT_EQUAL_STRING("open",            door_state_to_str(DOOR_STATE_OPEN));
    TEST_ASSERT_EQUAL_STRING("closed",          door_state_to_str(DOOR_STATE_CLOSED));
    TEST_ASSERT_EQUAL_STRING("opening",         door_state_to_str(DOOR_STATE_OPENING));
    TEST_ASSERT_EQUAL_STRING("closing",         door_state_to_str(DOOR_STATE_CLOSING));
    TEST_ASSERT_EQUAL_STRING("stopped_opening", door_state_to_str(DOOR_STATE_STOPPED_OPENING));
    TEST_ASSERT_EQUAL_STRING("stopped_closing", door_state_to_str(DOOR_STATE_STOPPED_CLOSING));
    TEST_ASSERT_EQUAL_STRING("unknown",         door_state_to_str(DOOR_STATE_UNKNOWN));
}

/* ── Runner ──────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();

    /* Trigger transitions */
    RUN_TEST(test_trigger_from_closed_goes_to_opening);
    RUN_TEST(test_trigger_from_opening_goes_to_stopped_opening);
    RUN_TEST(test_trigger_from_stopped_opening_goes_to_closing);
    RUN_TEST(test_trigger_from_closing_goes_to_stopped_closing);
    RUN_TEST(test_trigger_from_stopped_closing_goes_to_opening);
    RUN_TEST(test_trigger_from_open_goes_to_closing);
    RUN_TEST(test_trigger_from_unknown_stays_unknown);

    /* OPEN command */
    RUN_TEST(test_open_cmd_from_closed_fires_trigger);
    RUN_TEST(test_open_cmd_from_stopped_closing_fires_trigger);
    RUN_TEST(test_open_cmd_from_open_does_not_fire);
    RUN_TEST(test_open_cmd_from_opening_does_not_fire);
    RUN_TEST(test_open_cmd_from_stopped_opening_does_not_fire);
    RUN_TEST(test_open_cmd_from_closing_does_not_fire);

    /* CLOSE command */
    RUN_TEST(test_close_cmd_from_open_fires_trigger);
    RUN_TEST(test_close_cmd_from_stopped_opening_fires_trigger);
    RUN_TEST(test_close_cmd_from_closed_does_not_fire);
    RUN_TEST(test_close_cmd_from_closing_does_not_fire);
    RUN_TEST(test_close_cmd_from_stopped_closing_does_not_fire);
    RUN_TEST(test_close_cmd_from_opening_does_not_fire);

    /* TOGGLE command */
    RUN_TEST(test_toggle_always_fires_from_any_state);

    /* Sensor application */
    RUN_TEST(test_sensor_open_overrides_any_state);
    RUN_TEST(test_sensor_closed_overrides_any_state);
    RUN_TEST(test_sensor_unknown_preserves_current_state);

    /* State-to-string */
    RUN_TEST(test_state_to_str_returns_correct_strings);

    return UNITY_END();
}