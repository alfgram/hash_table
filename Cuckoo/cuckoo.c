#include "specific.h"
#include "../assoc.h"
#include <math.h>

/* Inserts key_value pair into table n.
Displaces current cell to opposing table if cell is full.
If displacement count exceeds log2 of capacity, resize */
void _insert(assoc** a, void* key, void* data, table_num n);
/* Resizes table by factor of SCALEFACTOR and reinserts
entries using the updated hash functions */
void _resize(assoc** a);
/* Writes key_value pair to cell */
void _put_data(assoc* a, void* key, void* data, table_num n, hash_value h);
/* Returns corresponding hash for given table number n */
hash_value _hash(assoc* a, void* key, table_num n);
/* Hash function based on djb2.
Source: "http://www.cse.yorku.ca/~oz/hash.html" */
int _t1_hash(assoc* a, void* key);
/* Hash function based on sdbm.
Source: "http://www.cse.yorku.ca/~oz/hash.html" */
int _t2_hash(assoc* a, void* key);
/* Returns 1 if the data pointed at by a and b is equal */
bool _is_same(void* a, void* b, int keysize);
/* Returns log base 2 of num */
int log_2(int num);

assoc* assoc_init(int keysize)
{
   assoc* hash_table = (assoc*)ncalloc(1, sizeof(assoc));
   if (keysize < 0) {
      on_error("Invalid keysize");
   }
   hash_table->t[t1] = (key_value*)ncalloc(INIT_CAP/2, sizeof(key_value));
   hash_table->t[t2] = (key_value*)ncalloc(INIT_CAP/2, sizeof(key_value));
   hash_table->capacity = INIT_CAP;
   hash_table->keysize = keysize;
   return hash_table;
}

void assoc_insert(assoc** a, void* key, void* data)
{
   hash_value h;
   if (!a || !(*a) || !key) {
      return;
   }
   /* If same key previously inserted into t1 update data */
   h = _hash(*a, key, t1);
   if (_is_same(key, (*a)->t[t1][h].key, (*a)->keysize)) {
      (*a)->t[t1][h].data = data;
      return;
   }
   /* If same key previously inserted into t2 update data */
   h = _hash(*a, key, t2);
   if (_is_same(key, (*a)->t[t2][h].key, (*a)->keysize)) {
      (*a)->t[t2][h].data = data;
      return;
   }
   (*a)->disp_cnt = 0;
   _insert(a, key, data, t1);
}

void* assoc_lookup(assoc* a, void* key)
{
   hash_value h;

   h = _hash(a, key, t1);
   if (a->t[t1][h].key == NULL) {
      return NULL;
   }
   if (_is_same(key, a->t[t1][h].key, a->keysize)) {
      return a->t[t1][h].data;
   }

   h = _hash(a, key, t2);
   if (a->t[t2][h].key == NULL) {
      return NULL;
   }
   if (_is_same(key, a->t[t2][h].key, a->keysize)) {
      return a->t[t2][h].data;
   }
   return NULL;
}

unsigned int assoc_count(assoc* a)
{
   if (a) {
      return a->entries;
   }
   return 0;
}

void assoc_free(assoc* a)
{
   free(a->t[t1]);
   free(a->t[t2]);
   free(a);
}

void _insert(assoc** a, void* key, void* data, table_num n)
{
   void *tmp_key, *tmp_data;
   hash_value h;
   if ((*a)->disp_cnt > log_2((*a)->capacity)) {
      _resize(a);
   }
   /* If cell is empty insert key_data pair */
   h = _hash(*a, key, n);
   if ((*a)->t[n][h].key == NULL) {
      _put_data(*a, key, data, n, h);
      return;
   }
   /* Displace current cell to opposing table.
   Insert key_data pair into current cell */
   (*a)->disp_cnt++;
   tmp_key = (*a)->t[n][h].key;
   tmp_data = (*a)->t[n][h].data;
   (*a)->t[n][h].key = key;
   (*a)->t[n][h].data = data;
   _insert(a, tmp_key, tmp_data, !n);
}

int log_2(int num)
{
   return (int) (log((float)num) / LOG_2);
}

void _put_data(assoc* a, void* key, void* data, table_num n, hash_value h)
{
   a->t[n][h].key = key;
   a->t[n][h].data = data;
   a->entries++;
}

void _resize(assoc** a)
{
   assoc* new = (assoc*)ncalloc(1, sizeof(assoc));
   assoc* tmp;
   unsigned int i;

   new->capacity = (*a)->capacity * SCALEFACTOR;
   new->keysize = (*a)->keysize;
   new->t[t1] = (key_value*)ncalloc(new->capacity/2, sizeof(key_value));
   new->t[t2] = (key_value*)ncalloc(new->capacity/2, sizeof(key_value));

   for (i = 0; i < (*a)->capacity/2; i++)
   {
      if ((*a)->t[t1][i].key != NULL) {
         assoc_insert(&new, (*a)->t[t1][i].key, (*a)->t[t1][i].data);
      }
      if ((*a)->t[t2][i].key != NULL) {
         assoc_insert(&new, (*a)->t[t2][i].key, (*a)->t[t2][i].data);
      }
   }
   tmp = *a;
   *a = new;
   assoc_free(tmp);
}

hash_value _hash(assoc* a, void* key, table_num n)
{
   if (n == t1) {
      return (hash_value)_t1_hash(a, key);
   }
   return (hash_value)_t2_hash(a, key);
}

int _t1_hash(assoc* a, void* key)
{
   unsigned char* key_str = (unsigned char*)key;
   unsigned long hash = T1_CONST_1;
   unsigned int c, i, len;
   if (a->keysize) {
      len = a->keysize;
   }
   else {
      len = strlen(key);
   }
   for (i = 0; i < len; i++)
   {
      c = *key_str++;
      hash = ((hash << T1_CONST_2) + hash) + c;
   }
   return hash % a->capacity/2;
}

int _t2_hash(assoc* a, void* key)
{
   unsigned char* key_str = (unsigned char*)key;
   unsigned long hash = 0;
   unsigned int c, i, len;
   if (a->keysize) {
      len = a->keysize;
   }
   else {
      len = strlen(key);
   }
   for (i = 0; i < len; i++)
   {
      c = *key_str++;
      hash = c + (hash << T2_CONST_1) + (hash << T2_CONST_2) - hash;
   }
   return hash % a->capacity/2;
}

bool _is_same(void* a, void* b, int keysize)
{
   if (!a || !b) {
      return false;
   }
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
