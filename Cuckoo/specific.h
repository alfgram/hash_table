/* Initial capacity of table */
#define INIT_CAP 16
#define SCALEFACTOR 2
#define LOG_2 log(2)
#define NUM_OF_TABLES 2
/* Constants used in hashing functions */
#define T1_CONST_1 5381
#define T1_CONST_2 5
#define T2_CONST_1 6
#define T2_CONST_2 16

struct key_value
{
   void* key;
   void* data;
};
typedef struct key_value key_value;

struct assoc
{
   key_value* t[NUM_OF_TABLES];
   unsigned int entries;
   unsigned int capacity;
   /* Displacement count */
   int disp_cnt;
   /* sizeof key in bytes, 0 indicates key is a string */
   int keysize;
};
typedef struct assoc assoc;

typedef unsigned int hash_value;
typedef enum bool {false, true} bool;
typedef enum table_num {t1, t2} table_num;
