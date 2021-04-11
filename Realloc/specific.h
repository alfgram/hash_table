/* Initial capacity of table */
#define INIT_CAP 17
/* Table is resized if proportion
of table full exceeds LOAD_FACTOR */
#define LOAD_FACTOR 0.6
#define SCALEFACTOR 2
#define HASH_SEED 5381
/* Prime number used in hash functions */
#define HASH_PRIME 5

#define TEST_SIZE 100000

struct key_value
{
   void* key;
   void* data;
};
typedef struct key_value key_value;

struct assoc
{
   key_value* table;
   /* sizeof key in bytes, 0 indicates key is a string */
   int keysize;
   unsigned int entries;
   unsigned int capacity;
};
typedef struct assoc assoc;

typedef enum bool {false, true} bool;
