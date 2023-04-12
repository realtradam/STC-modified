// This implements the multimap c++ example found at:
// https://en.cppreference.com/w/cpp/container/multimap/insert

// Multimap entries
#include <stc/cstr.h>
#define i_val_str
#include <stc/clist.h>

// Map of int => clist_str.
#define i_type Multimap
#define i_key int
#define i_valclass clist_str // set i_val = clist_str, bind clist_str_clone and clist_str_drop
#define i_cmp -c_default_cmp // like std::greater<int>
#include <stc/csmap.h>

void print(const char* lbl, const Multimap mmap)
{
    printf("%s ", lbl);
    c_foreach (e, Multimap, mmap) {
        c_foreach (s, clist_str, e.ref->second)
            printf("{%d,%s} ", e.ref->first, cstr_str(s.ref));
    }
    puts("");
}

void insert(Multimap* mmap, int key, const char* str)
{
    clist_str *list = &Multimap_insert(mmap, key, clist_str_init()).ref->second;
    clist_str_emplace_back(list, str);
}

int main()
{
    Multimap mmap = {0};

    // list-initialize
    typedef struct {int a; const char* b;} pair;
    c_forlist (i, pair, {{2, "foo"}, {2, "bar"}, {3, "baz"}, {1, "abc"}, {5, "def"}})
        insert(&mmap, i.ref->a, i.ref->b);
    print("#1", mmap);

    // insert using value_type
    insert(&mmap, 5, "pqr");
    print("#2", mmap);

    // insert using make_pair
    insert(&mmap, 6, "uvw");
    print("#3", mmap);

    insert(&mmap, 7, "xyz");
    print("#4", mmap);

    // insert using initialization_list
    c_forlist (i, pair, {{5, "one"}, {5, "two"}})
        insert(&mmap, i.ref->a, i.ref->b);
    print("#5", mmap);

    // FOLLOWING NOT IN ORIGINAL EXAMPLE:
    // erase all entries with key 5
    Multimap_erase(&mmap, 5);
    print("+5", mmap);
    
    Multimap_drop(&mmap);
}
