#ifndef EVENT_TABLE_H
#define EVENT_TABLE_H

enum event_level {
  DEBUG_EVENT,
  INFO_EVENT,
  WARN_EVENT,
  ERROR_EVENT,
  FATAL_EVENT
};

enum entity_type {
  ENTITY_OBJECT,
  ENTITY_TASK,
  ENTITY_FUNCTION
};

enum event_type {
  /* Scheduler */
  EVENT_NEW_TASK,
  EVENT_ASSIGN_TO_LOCAL,
  /* Object store */
  EVENT_CREATE_OBJECT,
  EVENT_GET_OBJECT,
  EVENT_SEAL_OBJECT,
  EVENT_START_TRANSFER,
  EVENT_FINISH_TRANSFER,
  /* Driver */
  EVENT_EXPORT_FUNCTION,
  EVENT_EXPORT_REUSABLE,
  /* Workers */
  EVENT_SUBMIT,
  EVENT_IMPORT,
  EVENT_WORKER_DONE,
  EVENT_RAISE_EXCEPTION,
  EVENT_EXECUTE
};

/* Add a new event to the event table. The info parameter contains a event
 * specific information in JSON format. */
void event_table_add_event(db_conn *conn, int event_level, int entity_type,
                           unique_id entity_id, int event_type, const char* info);

#endif
