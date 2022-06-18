typedef struct spinlock
{
    int flag;
} spinlock;

typedef struct mutexlock
{
    spinlock sl;
    int flag;
} mutexlock;

mutexlock LL_lock, initializer_lock;

static int test_and_set(int *value);

void spinlock_init(spinlock *sl);
void thread_lock(spinlock *sl);
void thread_unlock(spinlock *sl);

void mutexlock_init(mutexlock *lock);
void thread_mutex_lock(mutexlock *cm);
void thread_mutex_unlock(mutexlock *cm);