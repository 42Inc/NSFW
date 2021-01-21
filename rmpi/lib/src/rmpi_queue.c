#include "./../include/rmpi.h"

static char log_buffer[LOG_BUFFER_SIZE];

rmpi_queue_t rmpi_queue_init() {
  rmpi_queue_t q = NULL;
  q = (rmpi_queue_t)malloc(sizeof(struct rmpi_queue));
  if (!q) {
    rmpi_log_fatal("Cannot allocate queue");
  }
  q->end = NULL;
  q->head = NULL;
  q->len = 0;
  pthread_mutex_init(&q->locker, NULL);
  return q;
}

void rmpi_queue_free(rmpi_queue_t q) {
  if (!q) {
    return;
  }
  int64_t i = 0;
  rmpi_q_el_t el = q->head;
  rmpi_q_el_t prev = NULL;
  if (q->len == 1) {
    free(el->message);
    free(el);
  } else  if (q->len > 0) {
    prev = el;
    el = el->next;
    while (el != q->end) {
      if (!prev) {
        free(prev);
        prev = el;
        el = el->next;
      }
    }
    free(el);
  }
  free(q);
}

void rmpi_queue_push(rmpi_queue_t q, void* message, int64_t rank, size_t size) {
  if (!q) {
    rmpi_log_fatal("Cannot push queue");
  }
  rmpi_q_el_t el = (rmpi_q_el_t)malloc(sizeof(struct rmpi_q_el));
  if (!el) {
    rmpi_log_fatal("Cannot push queue");
  }
  pthread_mutex_lock(&q->locker);
  el->message = message;
  el->rank = rank;
  el->size = size;
  el->next = NULL;
  if (q->len <= 0) {
    q->head = el;
    q->end = el;
    q->len = 1;
  } else {
    q->end->next = el;
    q->end = el;
    ++q->len;
  }
  pthread_mutex_unlock(&q->locker);
}

void* rmpi_queue_pop(rmpi_queue_t q, int64_t rank) {
  if (!q) {
    rmpi_log_fatal("Cannot pop queue");
  }
  void* msg = NULL;
  pthread_mutex_lock(&q->locker);
  rmpi_q_el_t cursor = q->head;
  rmpi_q_el_t prev = NULL;
  int64_t i = 0;
  if (q->len <= 0 || !q->head) {
    pthread_mutex_unlock(&q->locker);
    return msg;
  }

  if (cursor->rank == rank) {
    msg = cursor->message;
    q->head = cursor->next;
    free(cursor);
  } else {
    cursor = q->head->next;
    prev = q->head;
    for (i = 0; i < q->len - 1; ++i) {
      if (!cursor && cursor->rank == rank) {
        msg = cursor->message;
        prev->next = cursor->next;
        if (cursor == q->end) {
          q->end = prev;
        }
        free(cursor);
        break;
      }
    }
  }
        --q->len;
  pthread_mutex_unlock(&q->locker);

  return msg;
}
