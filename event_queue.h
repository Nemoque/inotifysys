#ifndef __EVENT_QUEUE_H
#define __EVENT_QUEUE_H

#include <stdint.h>
#include <sys/inotify.h>

//declare a node specified with inotify_event element
struct queue_entry;

struct queue_entry
{
  struct queue_entry * next_ptr;   /* Pointer to next entry */
  struct inotify_event inot_ev;
};

typedef struct queue_entry * queue_entry_t;

//declare the queue with head and tail node 
struct queue_struct
{
  struct queue_entry * head;
  struct queue_entry * tail;
};
typedef struct queue_struct *queue_t;

//declare the basical methods for the queue
int queue_empty (queue_t q);
queue_t queue_create ();
void queue_destroy (queue_t q);
void queue_enqueue (queue_entry_t d, queue_t q);
queue_entry_t queue_dequeue (queue_t q);

#endif
