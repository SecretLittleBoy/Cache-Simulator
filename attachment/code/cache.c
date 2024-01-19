// cache simulator (put all your edits here)
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "main.h"
#include "debug.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE;
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

void set_cache_param(int param, int value) {
    switch (param) {
    case CACHE_PARAM_BLOCK_SIZE:
        cache_block_size = value;
        words_per_block = value / WORD_SIZE;
        break;
    case CACHE_PARAM_USIZE:
        cache_split = FALSE;
        cache_usize = value;
        break;
    case CACHE_PARAM_ISIZE:
        cache_split = TRUE;
        cache_isize = value;
        break;
    case CACHE_PARAM_DSIZE:
        cache_split = TRUE;
        cache_dsize = value;
        break;
    case CACHE_PARAM_ASSOC:
        cache_assoc = value;
        break;
    case CACHE_PARAM_WRITEBACK:
        cache_writeback = TRUE;
        break;
    case CACHE_PARAM_WRITETHROUGH:
        cache_writeback = FALSE;
        break;
    case CACHE_PARAM_WRITEALLOC:
        cache_writealloc = TRUE;
        break;
    case CACHE_PARAM_NOWRITEALLOC:
        cache_writealloc = FALSE;
        break;
    default:
        printf("error set_cache_param: bad parameter value\n");
        exit(-1);
    }
}

static void init_single_cache(cache *c, int cache_size) {
    Dprintf("init_single_cache %s start\n", c == &c1 ? "c1" : "c2");
    c->size = cache_size;
    c->associativity = cache_assoc;
    c->n_sets = cache_usize / (cache_assoc * cache_block_size);
    c->index_mask_offset = LOG2(cache_block_size);
    c->index_mask = (unsigned)(c->n_sets - 1) << c->index_mask_offset;
    c->LRU_head = (Pcache_line *)malloc(sizeof(Pcache_line) * c->n_sets);
    c->LRU_tail = (Pcache_line *)malloc(sizeof(Pcache_line) * c->n_sets);
    c->set_contents = (int *)malloc(sizeof(int) * c->n_sets);
    for (int i = 0; i < c->n_sets; i++) {
        c->LRU_head[i] = NULL;
        c->LRU_tail[i] = NULL;
        c->set_contents[i] = 0;
    }
}

void init_cache() {
    /* initialize the cache, and cache statistics data structures */
    Dprintf("cache init start\n");
    if (cache_split) {
        init_single_cache(&c1, cache_isize);
        init_single_cache(&c2, cache_dsize);
    } else {
        init_single_cache(&c1, cache_usize);
    }
    Dprintf("cache init done\n");
}

static void perform_load(Pcache c, unsigned addr, cache_stat *stat) {
    stat->accesses++;
    unsigned tag = addr >> (c->index_mask_offset + LOG2(c->n_sets));
    unsigned index = (addr & c->index_mask) >> c->index_mask_offset;

    int hit = FALSE;
    Pcache_line line = c->LRU_head[index];
    while (line) {
        if (line->tag == tag) {
            hit = TRUE;
            break;
        }
        line = line->LRU_next;
    }

    if (hit) {
        delete (&c->LRU_head[index], &c->LRU_tail[index], line);
        insert(&c->LRU_head[index], &c->LRU_tail[index], line);
    } else {
        stat->misses++;
        stat->demand_fetches += words_per_block;
        if (c->set_contents[index] == c->associativity) {
            stat->replacements++;
            if (c->LRU_tail[index]->dirty) {
                stat->copies_back += words_per_block;
            }
            line = c->LRU_tail[index];
            delete (&c->LRU_head[index], &c->LRU_tail[index], line);
            line->tag = tag;
            line->dirty = FALSE;
            insert(&c->LRU_head[index], &c->LRU_tail[index], line);
        } else {
            line = (Pcache_line)malloc(sizeof(cache_line));
            line->tag = tag;
            line->dirty = FALSE;
            insert(&c->LRU_head[index], &c->LRU_tail[index], line);
            c->set_contents[index]++;
        }
    }
}

static void perform_store(Pcache c, unsigned addr, cache_stat *stat) {
    stat->accesses++;
    unsigned tag = addr >> (c->index_mask_offset + LOG2(c->n_sets));
    unsigned index = (addr & c->index_mask) >> c->index_mask_offset;

    int hit = FALSE;
    Pcache_line line = c->LRU_head[index];
    while (line) {
        if (line->tag == tag) {
            hit = TRUE;
            break;
        }
        line = line->LRU_next;
    }

    if (hit) {
        delete (&c->LRU_head[index], &c->LRU_tail[index], line);
        insert(&c->LRU_head[index], &c->LRU_tail[index], line);
        if (!cache_writeback) { // write through
            stat->copies_back += 1;
            line->dirty = FALSE;
        } else { // write back
            line->dirty = TRUE;
        }
    } else {
        stat->misses++;
        if (cache_writealloc) { // write allocate
            stat->demand_fetches += words_per_block;
            if (c->set_contents[index] == c->associativity) {
                stat->replacements++;
                if (c->LRU_tail[index]->dirty) {
                    stat->copies_back += words_per_block;
                }
                line = c->LRU_tail[index];
                delete (&c->LRU_head[index], &c->LRU_tail[index], line);
                line->tag = tag;
                insert(&c->LRU_head[index], &c->LRU_tail[index], line);
                if(cache_writeback) {
                    line->dirty = TRUE;
                } else {
                    line->dirty = FALSE;
                    stat->copies_back += 1;
                }
            } else {
                line = (Pcache_line)malloc(sizeof(cache_line));
                line->tag = tag;
                insert(&c->LRU_head[index], &c->LRU_tail[index], line);
                c->set_contents[index]++;
                if(cache_writeback) {
                    line->dirty = TRUE;
                } else {
                    line->dirty = FALSE;
                    stat->copies_back += 1;
                }
            }
        } else {
            stat->copies_back += 1;
        }
    }
}

void perform_access(unsigned addr, unsigned access_type) {
    /* handle an access to the cache */
    if (cache_split) {
        switch (access_type) {
        case TRACE_DATA_LOAD:
            perform_load(&c2, addr, &cache_stat_data);
            break;
        case TRACE_DATA_STORE:
            perform_store(&c2, addr, &cache_stat_data);
            break;
        case TRACE_INST_LOAD:
            perform_load(&c1, addr, &cache_stat_inst);
            break;
        default:
            break;
        }
    } else {
        switch (access_type) {
        case TRACE_DATA_LOAD:
            perform_load(&c1, addr, &cache_stat_data);
            break;
        case TRACE_DATA_STORE:
            perform_store(&c1, addr, &cache_stat_data);
            break;
        case TRACE_INST_LOAD:
            perform_load(&c1, addr, &cache_stat_inst);
            break;
        default:
            break;
        }
    }
}

void flush() {
    /* flush the cache */
    if (cache_split) {
        for (int i = 0; i < c1.n_sets; i++) {
            Pcache_line line = c1.LRU_head[i];
            while (line) {
                if (line->dirty) {
                    cache_stat_inst.copies_back += words_per_block;
                    line->dirty = FALSE;
                }
                line = line->LRU_next;
            }
        }
        for (int i = 0; i < c2.n_sets; i++) {
            Pcache_line line = c2.LRU_head[i];
            while (line) {
                if (line->dirty) {
                    cache_stat_data.copies_back += words_per_block;
                    line->dirty = FALSE;
                }
                line = line->LRU_next;
            }
        }
    } else {
        for (int i = 0; i < c1.n_sets; i++) {
            Pcache_line line = c1.LRU_head[i];
            while (line) {
                if (line->dirty) {
                    cache_stat_data.copies_back += words_per_block;
                    line->dirty = FALSE;
                }
                line = line->LRU_next;
            }
        }
    }
}

// delete item from the list. But item is not freed.
void delete(Pcache_line *head, Pcache_line *tail, Pcache_line item) {
    if (item->LRU_prev) {
        item->LRU_prev->LRU_next = item->LRU_next;
    } else {
        /* item at head */
        *head = item->LRU_next;
    }

    if (item->LRU_next) {
        item->LRU_next->LRU_prev = item->LRU_prev;
    } else {
        /* item at tail */
        *tail = item->LRU_prev;
    }
}

/* inserts at the head of the list */
void insert(Pcache_line *head, Pcache_line *tail, Pcache_line item) {
    item->LRU_next = *head;
    item->LRU_prev = (Pcache_line)NULL;

    if (item->LRU_next)
        item->LRU_next->LRU_prev = item;
    else
        *tail = item;

    *head = item;
}

// print cache settings
void dump_settings() {
    printf("*** CACHE SETTINGS ***\n");
    if (cache_split) {
        printf("  Split I- D-cache\n");
        printf("  I-cache size: \t%d\n", cache_isize);
        printf("  D-cache size: \t%d\n", cache_dsize);
    } else {
        printf("  Unified I- D-cache\n");
        printf("  Size: \t%d\n", cache_usize);
    }
    printf("  Associativity: \t%d\n", cache_assoc);
    printf("  Block size: \t%d\n", cache_block_size);
    printf("  Write policy: \t%s\n",
           cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
    printf("  Allocation policy: \t%s\n",
           cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}

void print_stats() {
    printf("\n*** CACHE STATISTICS ***\n");

    printf(" INSTRUCTIONS\n");
    printf("  accesses:  %d\n", cache_stat_inst.accesses);
    printf("  misses:    %d\n", cache_stat_inst.misses);
    if (!cache_stat_inst.accesses)
        printf("  miss rate: 0 (0)\n");
    else
        printf("  miss rate: %2.4f (hit rate %2.4f)\n",
               (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses,
               1.0 - (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
    printf("  replace:   %d\n", cache_stat_inst.replacements);

    printf(" DATA\n");
    printf("  accesses:  %d\n", cache_stat_data.accesses);
    printf("  misses:    %d\n", cache_stat_data.misses);
    if (!cache_stat_data.accesses)
        printf("  miss rate: 0 (0)\n");
    else
        printf("  miss rate: %2.4f (hit rate %2.4f)\n",
               (float)cache_stat_data.misses / (float)cache_stat_data.accesses,
               1.0 - (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
    printf("  replace:   %d\n", cache_stat_data.replacements);

    printf(" TRAFFIC (in words)\n");
    printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches +
                                        cache_stat_data.demand_fetches);
    printf("  copies back:   %d\n", cache_stat_inst.copies_back +
                                        cache_stat_data.copies_back);
}
