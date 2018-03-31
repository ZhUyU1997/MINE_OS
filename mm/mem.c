#include <sys/types.h>
#include <list.h>
#include <sys/mm.h>
#include <assert.h>
#include <trace.h>
struct kmem_cache {
	unsigned int obj_size;
	unsigned int obj_nr;
	unsigned int page_order;
	unsigned int flags;
	struct page *head_page;
	struct page *end_page;
	void *nf_block;
};

struct page {
	unsigned int vaddr;
	unsigned int flags;
	int order;
	struct kmem_cache *cachep;
	struct list_head list;//to string the buddy member
};



#define MAX_BUDDY_PAGE_NUM	(9)	//finally I desided to choose a fixed value,which means users could alloc 1M space at most,those who need more than 1M should change the macro to reserve enough space for private use.

#define AVERAGE_PAGE_NUM_PER_BUDDY	(KERNEL_PAGE_NUM/MAX_BUDDY_PAGE_NUM)
#define PAGE_NUM_FOR_EACH_BUDDY(j) ((AVERAGE_PAGE_NUM_PER_BUDDY>>(j))*(1<<(j)))
#define PAGE_NUM_FOR_MAX_BUDDY	((1<<MAX_BUDDY_PAGE_NUM)-1)//=0xff=255，最大页面减1

struct list_head page_buddy[MAX_BUDDY_PAGE_NUM];


struct page *virt_to_page(unsigned int addr) {
	unsigned int i = ((addr) - KERNEL_PAGING_START) >> PAGE_SHIFT;
	if (i > KERNEL_PAGE_NUM)
		return NULL;
	return (struct page *)KERNEL_PAGE_START + i;
}

void init_page_buddy(void) {
	for (int i = 0; i < MAX_BUDDY_PAGE_NUM; i++) {
		INIT_LIST_HEAD(&page_buddy[i]);
	}
}
//获取最低位1所在位
static int position_of_the_lowest_bit_set(unsigned int addr) {
	unsigned int r = 0;
	if(!addr)
		return -1;
	while (addr <<= 1)
		r++;
	assert(r < 32);
	return 31-r;
}
//获取最高位1所在位
static int position_of_the_highest_bit_set(unsigned int addr) {
	unsigned int r = 0;
	while (addr >>= 1)
		r++;
	assert(r < 32);
	return r;
}
//由page的size计算order值
static int get_order(unsigned int size){
	int pos = position_of_the_highest_bit_set(size >> PAGE_SHIFT);
	assert(pos < 20);
	if ((1 << pos) < (size >> PAGE_SHIFT)){
		if(pos >= 19)//超过了分配上限
			return NULL;
		pos++;
	}
	return pos;
}
static unsigned int align_value(int vaddr){//vaddr 4k*n 字节对齐，返回n
	assert(vaddr != 0);
	unsigned int i = position_of_the_lowest_bit_set(vaddr >> PAGE_SHIFT);
	if(i > 0xff)
		i = 0xff;
	return i;
}
void init_page_map(void) {
	struct page *pg = (struct page *)KERNEL_PAGE_START;
	init_page_buddy();
	for (int i = 0; i < (KERNEL_PAGE_NUM); pg++, i++) {
		/*fill struct page first*/
		pg->vaddr = KERNEL_PAGING_START + i * PAGE_SIZE;
		pg->flags = PAGE_AVAILABLE;
		pg->flags &= ~(PAGE_ALIGN_MASK);
		pg->flags |= (align_value(pg->vaddr) << PAGE_ALIGN_SHIFT);
		INIT_LIST_HEAD(&(pg->list));

		/*make the memory max buddy as possible*/
		if (i < (KERNEL_PAGE_NUM & (~PAGE_NUM_FOR_MAX_BUDDY))) {	//如果够分配1M，放到page_buddy[MAX_BUDDY_PAGE_NUM-1]
			/*the following code should be dealt carefully,we would change the order field of a head struct page to the corresponding order,and change others to -1*/
			if ((i & PAGE_NUM_FOR_MAX_BUDDY) == 0) { //如果为1M大页第一个4k页
				pg->order = MAX_BUDDY_PAGE_NUM - 1;
			} else {
				pg->order = -1; //表示这个 page 结构体不是 buddy 的上边界
			}
			list_add_tail(&(pg->list), &page_buddy[MAX_BUDDY_PAGE_NUM - 1]);
			/*the remainder not enough to merge into a max buddy is done as min buddy*/
		} else {
			pg->order = 0;
			list_add_tail(&(pg->list), &page_buddy[0]);
		}
	}
}

/*we can do these all because the page structure that represents one page aera is continuous*/
#define BUDDY_END(x,order)			((x)+(1<<(order))-1)
#define NEXT_BUDDY_START(x,order)	((x)+(1<<(order)))
#define PREV_BUDDY_START(x,order)	((x)-(1<<(order)))


void *page_address(struct page *pg) {
	return (void *)(pg->vaddr);
}

/*the logic of this function seems good,no bug reported yet*/
struct page *get_pages_from_list(unsigned int flags, int order) {
	unsigned int vaddr;
	int neworder = order;
	struct page *pg, *pgt;
	struct list_head *tlst, *tlst1, *plist;
	for (; neworder < MAX_BUDDY_PAGE_NUM; neworder++) { //从order遍历page_buddy
		if (list_empty(&page_buddy[neworder])) {
			continue;
		} else {
			int i = 0;
			plist = (&page_buddy[neworder]);
			for(int i = 0;i < 12;i++){
				trace(KERN_DEBUG, "%X ",plist);
				plist = plist->next;
			}
			trace(KERN_DEBUG, "\n");
			for (plist = (&page_buddy[neworder])->next;
					plist != (&page_buddy[neworder]);
					plist = (BUDDY_END(pg, neworder)->list.next)){
				pg = list_entry(plist, struct page, list);
				trace(KERN_DEBUG, "*** i = %d  ",i);
				trace(KERN_DEBUG, "plist = %X ,vaddr = %X, order = %d, align = %d\n",plist,page_address(pg),order,(pg->flags & PAGE_ALIGN_MASK)>>4);
				//检查是否满足字节对齐条件
				//TODO:可以优化检查加快速度
				//TODO:现在只检查页面其实地址，可以进一步检查页面地址范围内有无合适地址
				if((flags & PAGE_ALIGN_MASK) <= (pg->flags & PAGE_ALIGN_MASK)){
					tlst = plist;
					tlst1 = &(BUDDY_END(pg, neworder)->list);
					list_remove_chain(tlst, tlst1);
					goto OUT_OK;
				}
				i++;
			}
			continue;//如果循环结束，则增大页面尺寸
		}
	}
	return NULL;

OUT_OK:
	//将大页面分为小页面
	for (neworder--; neworder >= order; neworder--) {
		pgt = NEXT_BUDDY_START(pg, neworder);
		tlst = &(pgt->list);
		tlst1 = &(BUDDY_END(pgt, neworder)->list);
		list_entry(tlst, struct page, list)->order = neworder;
		list_add_chain_tail(tlst, tlst1, &page_buddy[neworder]);
	}
	if(pg->flags & PAGE_BUDDY_BUSY){
		trace(KERN_ERR, "something must be wrong when you see this message,that probably means you are forcing to release a page that was not alloc at all\n");
		assert(0);
	}
	pg->flags |= PAGE_BUDDY_BUSY;
	pg->order = order;
	pg->cachep = NULL;
	return pg;
}

void put_pages_to_list(struct page *pg, int order) {
	struct page *tprev, *tnext;
	struct list_head *plist;
	trace(KERN_DEBUG, "put_pages_to_list:start\n");
	if (!(pg->flags & PAGE_BUDDY_BUSY)) {
		trace(KERN_DEBUG, "put_pages_to_list:vaddr = %X, order = %d\n",pg->vaddr,order);
		trace(KERN_ERR, "something must be wrong when you see this message,that probably means you are forcing to release a page that was not alloc at all\n");
		assert(0);
		return;
	}
	pg->flags &= ~PAGE_BUDDY_BUSY;
	//TODO:如果put第一个页面和最后一个页面，可能会出问题
	for (; order < MAX_BUDDY_PAGE_NUM; order++) {
		tnext = NEXT_BUDDY_START(pg, order);
		tprev = PREV_BUDDY_START(pg, order);
		if ((tnext < KERNEL_PAGE_END) && (!(tnext->flags & PAGE_BUDDY_BUSY)) && (tnext->order == order)) {
			trace(KERN_DEBUG, "*** merge: order = %d, list = %X\n",order,&(tnext->list));
			pg->order++;
			tnext->order = -1;
			list_remove_chain(&(tnext->list), &(BUDDY_END(tnext, order)->list));
			BUDDY_END(pg, order)->list.next = &(tnext->list);
			tnext->list.prev = &(BUDDY_END(pg, order)->list);
			continue;
		} else if ((tprev >= KERNEL_PAGE_START) && (!(tprev->flags & PAGE_BUDDY_BUSY)) && (tprev->order == order)) {
			trace(KERN_DEBUG, "*** merge: order = %d, list = %X\n",order,&(tprev->list));
			pg->order = -1;

			list_remove_chain(&(tprev->list), &(BUDDY_END(tprev, order)->list));
			BUDDY_END(tprev, order)->list.next = &(pg->list);
			pg->list.prev = &(BUDDY_END(tprev, order)->list);

			pg = tprev;
			pg->order++;
			continue;
		} else {
			break;
		}
	}
	trace(KERN_DEBUG, "*** free: order = %d\n",order);
	list_add_chain(&(pg->list), &(BUDDY_END(pg, order)->list), &page_buddy[order]);
	trace(KERN_DEBUG, "put_pages_to_list:end\n");
}




struct page *alloc_pages(unsigned int flags, int order) {
	trace(KERN_DEBUG, "alloc_pages:start\n");
	struct page *pg = get_pages_from_list(flags, order);
	if (pg == NULL)
		return NULL;
	for (int i = 0; i < (1 << order); i++) {
		(pg + i)->flags |= PAGE_DIRTY;
	}
	trace(KERN_DEBUG, "alloc_pages:vaddr = %X, order = %d\n",page_address(pg),order);
	trace(KERN_DEBUG, "alloc_pages:end\n");
	return pg;
}

void free_pages(struct page *pg, int order) {
	trace(KERN_DEBUG, "free_pages:start\n");
	for (int i = 0; i < (1 << order); i++) {
		(pg + i)->flags &= ~PAGE_DIRTY;
	}
	put_pages_to_list(pg, order);
	trace(KERN_DEBUG, "free_pages:end\n");
}

void *get_free_pages(unsigned int flags, int order) {
	struct page *page;
	trace(KERN_DEBUG, "get_free_pages:start\n");
	page = alloc_pages(flags, order);
	if (!page)
		return NULL;
	trace(KERN_DEBUG, "get_free_pages:vaddr = %X, order = %d\n",page_address(page),order);
	return	page_address(page);
}

void put_free_pages(void *addr, int order) {
	trace(KERN_DEBUG, "put_free_pages:vaddr = %X\n",virt_to_page((unsigned int)addr));
	free_pages(virt_to_page((unsigned int)addr), order);
}



#define KMEM_CACHE_DEFAULT_ORDER	(0)
#define KMEM_CACHE_MAX_ORDER		(5)			//cache can deal with the memory no less than 32*PAGE_SIZE
#define KMEM_CACHE_SAVE_RATE		(0x5a)
#define KMEM_CACHE_PERCENT			(0x64)
#define KMEM_CACHE_MAX_WAST			(PAGE_SIZE-KMEM_CACHE_SAVE_RATE * PAGE_SIZE / KMEM_CACHE_PERCENT)


int find_right_order(unsigned int size) {
	int order;
	for (order = 0; order <= KMEM_CACHE_MAX_ORDER; order++) {
		if (size <= (KMEM_CACHE_MAX_WAST) * (1 << order)) {
			return order;
		}
	}
	if (size > (1 << order))
		return -1;
	return order;
}


int kmem_cache_line_object(void *head, unsigned int size, int order) {
	void **pl;
	char *p;
	pl = (void **)head;
	p = (char *)head + size;
	int i, s = PAGE_SIZE * (1 << order);
	for (i = 0; s > size; i++, s -= size) {
		*pl = (void *)p; //保存下个内存块首地址于当前内存块
		pl = (void **)p;
		p = p + size;
	}
	if (s == size)
		i++;
	return i;
}

struct kmem_cache *kmem_cache_create(struct kmem_cache *cache, unsigned int size, unsigned int flags) {
	void **nf_block = &(cache->nf_block);

	int order = find_right_order(size);
	if (order == -1)
		return NULL;
	if ((cache->head_page = alloc_pages(0, order)) == NULL)
		return NULL;
	*nf_block = page_address(cache->head_page);

	cache->obj_nr = kmem_cache_line_object(*nf_block, size, order);
	cache->obj_size = size;
	cache->page_order = order;
	cache->flags = flags;
	cache->end_page = BUDDY_END(cache->head_page, order);
	cache->end_page->list.next = NULL;

	return cache;
}

/*FIXME:I dont understand it now*/
void kmem_cache_destroy(struct kmem_cache *cache) {
	int order = cache->page_order;
	struct page *pg = cache->head_page;
	struct list_head *list;
	while (1) {
		list = BUDDY_END(pg, order)->list.next;
		free_pages(pg, order);
		if (list) {
			pg = list_entry(list, struct page, list);
		} else {
			return;
		}
	}
}

void kmem_cache_free(struct kmem_cache *cache, void *objp) {
	*(void **)objp = cache->nf_block; //保存下个内存块首地址于当前内存块
	cache->nf_block = objp;
	cache->obj_nr++;
}

//TODO:flags
void *kmem_cache_alloc(struct kmem_cache *cache, unsigned int flags) {
	void *p;
	struct page *pg;
	if (cache == NULL)
		return NULL;
	void **nf_block = &(cache->nf_block);
	unsigned int *nr = &(cache->obj_nr);
	int order = cache->page_order;

	if (!*nr) { //剩余个数为0
		//再申请slab
		if ((pg = alloc_pages(0, order)) == NULL)
			return NULL;
		*nf_block = page_address(pg);
		cache->end_page->list.next = &pg->list;
		cache->end_page = BUDDY_END(pg, order);
		cache->end_page->list.next = NULL;
		*nr += kmem_cache_line_object(*nf_block, cache->obj_size, order);
	}

	(*nr)--;
	//获取对应分配内存块地址
	p = *nf_block;
	//使kmem_cache 结构体的 nf_block 成员指向下一个空闲子内存块
	*nf_block = *(void **)p;
	//获取对应分配内存块所在page结构体
	pg = virt_to_page((unsigned int)p);
	//确保每个 struct page 结构只属于一个 kmem_cache
	pg->cachep = cache;
	return p;
}



#define KMALLOC_BIAS_SHIFT					(5)				//32byte minimal
#define KMALLOC_MAX_SIZE					(4096)
#define KMALLOC_MINIMAL_SIZE_BIAS			(1 << KMALLOC_BIAS_SHIFT)
#define KMALLOC_CACHE_SIZE					(KMALLOC_MAX_SIZE / KMALLOC_MINIMAL_SIZE_BIAS)
#define kmalloc_cache_size_to_index(size)	((size) >> KMALLOC_BIAS_SHIFT)

struct kmem_cache kmalloc_cache[KMALLOC_CACHE_SIZE] = {0};


int kmalloc_init(void) {
	for (int i = 0; i < KMALLOC_CACHE_SIZE; i++) {
		//flags = 0
		if (kmem_cache_create(&kmalloc_cache[i], (i + 1)*KMALLOC_MINIMAL_SIZE_BIAS, 0) == NULL)
			return -1;
	}
	return 0;
}

static void *_kmalloc(unsigned int size) {
	int index = kmalloc_cache_size_to_index(size);
	if (index >= KMALLOC_CACHE_SIZE)
		return NULL;
	return kmem_cache_alloc(&kmalloc_cache[index], 0);
}

static void _kfree(void *addr) {
	struct page *pg;
	pg = virt_to_page((unsigned int)addr);
	kmem_cache_free(pg->cachep, addr);
}

void *kmalloc(unsigned int size) {
	void *this;
	if (size < KMALLOC_MAX_SIZE)
		this = _kmalloc(size);
	else {
		int order = get_order(size);
		this = get_free_pages(0, order);
	}
	return this;
}

void kfree(void *addr) {
	struct page *pg;
	if (addr == NULL)
		return;
	pg = virt_to_page((unsigned int)addr);
	if (pg->cachep != NULL) {
		_kfree(addr);
	} else {
		free_pages(pg, pg->order);
	}
}
