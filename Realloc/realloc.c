#include "specific.h"
#include "../assoc.h"

/* Resizes table by factor of SCALEFACTOR and reinserts
entries using the updated hash functions */
void _resize(assoc** a);
/* Inserts key and data into table a.
If entry is found with same key only data is updated */
void _insert_data(assoc* a, void* key, void* data);
/* Returns 1 if the data pointed at by a and b is equal */
bool _is_same(void* a, void* b, int keysize);
/* Returns next prime number after num */
unsigned int _next_prime(int num);
/* Returns 1 if num is prime */
bool _is_prime(int num);
/* Primary hash function based on djb2.
Source: "http://www.cse.yorku.ca/~oz/hash.html" */
int _p_hash(assoc* a, void* key);
/* Secondary hash function, returns probe step size */
int _s_hash(assoc* a, void* key);

void assoc_test(void);

assoc* assoc_init(int keysize)
{
   assoc* new = (assoc*)ncalloc(1, sizeof(assoc));
   if (keysize < 0) {
      on_error("Invalid keysize");
   }
   new->table = (key_value*)ncalloc(INIT_CAP, sizeof(key_value));
   new->capacity = INIT_CAP;
   new->keysize = keysize;
   return new;
}

void assoc_insert(assoc** a, void* key, void* data)
{
   if (!a || !(*a) || !key) {
      return;
   }
   if ((float)(*a)->entries / (float)(*a)->capacity > LOAD_FACTOR) {
      _resize(a);
   }
   _insert_data(*a, key, data);
}

unsigned int assoc_count(assoc* a)
{
   if (a) {
      return a->entries;
   }
   return 0;
}

void* assoc_lookup(assoc* a, void* key)
{
   int hash_val = _p_hash(a, key);
   int index = hash_val;
   int step_size = _s_hash(a, key);
   if (!a || !key) {
      return NULL;
   }
   do
   {
      if (a->table[index].key == NULL) {
         return NULL;
      }
      if (_is_same(key, a->table[index].key, a->keysize)) {
         return a->table[index].data;
      }
      index = (index + step_size) % a->capacity;
   } while (index != hash_val);
   return NULL;
}

void _insert_data(assoc* a, void* key, void* data)
{
   int index = _p_hash(a, key);
   int step_size = _s_hash(a, key);

   while (a->table[index].key != NULL)
   {
      if (_is_same(key, a->table[index].key, a->keysize)) {
         a->table[index].data = data;
         return;
      }
      index = (index + step_size) % a->capacity;
   }
   a->table[index].key = key;
   a->table[index].data = data;
   a->entries++;
}

void _resize(assoc** a)
{
   assoc* new = (assoc*)ncalloc(1, sizeof(assoc));
   assoc* tmp;
   unsigned int i;
   new->capacity = _next_prime((*a)->capacity * SCALEFACTOR);
   new->table = (key_value*)ncalloc(new->capacity, sizeof(key_value));
   new->keysize = (*a)->keysize;
   for (i = 0; i < (*a)->capacity; i++)
   {
      if ((*a)->table[i].key) {
         assoc_insert(&new, (*a)->table[i].key, (*a)->table[i].data);
      }
   }
   tmp = *a;
   *a = new;
   assoc_free(tmp);
}

int _p_hash(assoc* a, void* key)
{
   unsigned char* key_str = (unsigned char*)key;
   unsigned long hash = HASH_SEED;
   unsigned int c;
   int i, len;
   if (a->keysize) {
      len = a->keysize;
   }
   else {
      len = strlen((char*)key);
   }
   for (i = 0; i < len; i++)
   {
      c = *key_str++;
      hash = ((hash << HASH_PRIME) + hash) + c;
   }
   return hash % a->capacity;
}

int _s_hash(assoc* a, void* key)
{
   int hash = _p_hash(a, key);
   return (HASH_PRIME - (hash % HASH_PRIME));
}

bool _is_same(void* a, void* b, int keysize)
{
   if (keysize) {
      if (memcmp(a, b, keysize)) {
         return false;
      }
      return true;
   }
   else {
      if (strcmp(a, b)) {
         return false;
      }
      return true;
   }
}

bool _is_prime(int num)
{
   int i;
   for (i = 2; i*i < num; i++)
   {
      if (num % i == 0) {
         return false;
      }
   }
   return true;
}

unsigned int _next_prime(int num)
{
   while (!_is_prime(num))
   {
      num++;
   }
   return num;
}

void assoc_free(assoc* a)
{
   if (a) {
      free(a->table);
      free(a);
   }
}

void assoc_test(void)
{
   int i1 = 1, i2 = 99, i3 = 12345, i4 = 2, i5 = 98, i6 = 12346;
   int int_test_keys[8] = {1, 0, 123, 10, 2, -7892, -90, 28};
   int int_test_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
   unsigned int non_primes[25] = {18, 20, 26, 30, 35, 38, 42, 44, 50, 100, 102, 110, 114, 194, 198, 200, 962, 974, 978, 986, 992, 5106, 7898, 8068, 10008};
   unsigned int primes[25] =     {19, 23, 29, 31, 37, 41, 43, 47, 53, 101, 103, 113, 121, 197, 199, 211, 967, 977, 983, 991, 997, 5107, 7901, 8069, 10009};
   unsigned int i;
   assoc* a = assoc_init(sizeof(int));
   static float test_float_keys[TEST_SIZE];
   static int test_float_data[TEST_SIZE];
   static long test_long_keys[TEST_SIZE];
   static float test_long_data[TEST_SIZE];
   assert(a->capacity == INIT_CAP);
   assert(a->entries == 0);
   assert(a->keysize == sizeof(int));
   /* Testing assoc_insert inserts key-value pair into correct corresponding hash value index,
   int_test_keys have been chosen to all have unique hash values initially and after resizing
   as to avoid linear probing */
   for (i = 0; i < 8; i++)
   {
      assoc_insert(&a, &int_test_keys[i], &int_test_data[i]);
      assert(_is_same( (a->table[_p_hash(a, &int_test_keys[i])]).data , &int_test_data[i], sizeof(int)));
   }
   _resize(&a);
   assert(a->keysize == sizeof(int));
   assert(a->entries == 8);
   assert(a->capacity == _next_prime(INIT_CAP * SCALEFACTOR));
   for (i = 0; i < 8; i++)
   {
      assert(_is_same((a->table[_p_hash(a, &int_test_keys[i])]).data , &int_test_data[i], sizeof(int)));
   }
   _resize(&a);
   assert(a->keysize == sizeof(int));
   assert(a->entries == 8);
   assert(a->capacity == _next_prime(_next_prime(INIT_CAP * SCALEFACTOR) * SCALEFACTOR));
   assoc_free(a);
   /* Testing on assoc_insert and assoc_lookup using floats as keys
   and random ints at data */
   srand(time(NULL));
   a = assoc_init(sizeof(float));
   for (i = 0; i < TEST_SIZE; i++)
   {
      test_float_data[i] = rand()%TEST_SIZE;
      test_float_keys[i] = (float)i/100.0;
      assoc_insert(&a, &test_float_keys[i], &test_float_data[i]);
   }
   for (i = 0; i < TEST_SIZE; i++)
   {
      assert(assoc_lookup(a, &test_float_keys[i]) == &test_float_data[i]);
   }
   assoc_free(a);

   /* Testing on assoc_insert and assoc_lookup using longs as keys
   and random floats at data */
   a = assoc_init(sizeof(long));
   for (i = 0; i < TEST_SIZE; i++)
   {
      test_long_data[i] = (float)(rand()/1000);
      test_long_keys[i] = (long)(i*1000);
      assoc_insert(&a, &test_long_keys[i], &test_long_data[i]);
   }
   for (i = 0; i < TEST_SIZE; i++)
   {
      assert(assoc_lookup(a, &test_long_keys[i]) == &test_long_data[i]);
   }
   assoc_free(a);
   /* Testing _is_prime with known primes/non-primes,
   numbers less than INIT_CAP are out of scope */
   for (i = 0; i < 25; i++)
   {
      assert(_is_prime(primes[i]));
      assert(!_is_prime(non_primes[i]));
      assert(_next_prime(non_primes[i]) == primes[i]);
   }
   /* Test _is_same, assume user responsible for correct keysize */
   assert(_is_same("", "", 0));
   assert(_is_same("t", "t", 0));
   assert(_is_same("test", "test", 0));
   assert(_is_same("test_test_test_test", "test_test_test_test", 0));
   assert(!_is_same("t", "s", 0));
   assert(!_is_same("test", "tess", 0));
   assert(!_is_same("test", "tes", 0));
   assert(_is_same(&i1, &i1, sizeof(int)));
   assert(_is_same(&i2, &i2, sizeof(int)));
   assert(_is_same(&i3, &i3, sizeof(int)));
   assert(!_is_same(&i1, &i4, sizeof(int)));
   assert(!_is_same(&i2, &i5, sizeof(int)));
   assert(!_is_same(&i3, &i6, sizeof(int)));
}
