/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include <vaddr.h>

/* global frame table & table start arr */
static struct list frame_table; // 프레임 테이블을 나타내는 리스트 구조체
static struct list_elem *frame_start; // 프레임 테이블의 시작 요소를 가리키는 포인터

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */

		/* TODO: Insert the page into the spt. */
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
/* 파라미터로 
*  spt: supplemental_page_table 구조체의 포인터(해시 테이블)
*  va: 가상 주소
*  spt에서 va에 해당하는 페이지를 찾아 반환한다. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function. */

	// 새 페이지 생성: 가상 주소를 해싱하기 위한 페이지. 
    struct page *page = (struct page *)malloc(sizeof(struct page));     
    page->va = pg_round_down(va);                  // 가상 주소의 시작 주소 저장

	// 해시 테이블에서 해당 페이지 찾기
    struct hash_elem *e = hash_find(&spt->spt_hash, &page->hash_elem); 
   
    free(page);

	return e != NULL ? hash_entry(e, struct page, hash_elem) : NULL;     // 찾은 페이지 반환, 없다면 NULL 반환

}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED, struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function. */

	if(!hash_insert(&spt->spt_hash, &page->hash_elem))
        succ = true;

	return succ;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* 하나의 페이지를 내보내고 해당 프레임 반환.
 * 오류가 발생하면 NULL을 반환. */
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */
	
	swap_out(victim->page);	// 스왑 아웃, 페이지 교체

	return NULL;
}


/* palloc()을 호출하고 프레임을 가져옵니다. 사용 가능한 페이지가 없으면
 * 페이지를 내보내고 반환합니다. 이 함수는 항상 유효한 주소를 반환합니다.
 * 즉, 사용자 풀 메모리가 가득 차 있는 경우, 이 함수는 프레임을 내보내
 * 사용 가능한 메모리 공간을 확보한다. */
static struct frame *
vm_get_frame (void) {
	/* TODO: Fill this function. 수정 */

    struct frame *frame = (struct frame *)malloc(sizeof(struct frame));
    ASSERT (frame != NULL);	// 프레임이 NULL이 아닌지 확인

    frame->kva = palloc_get_page(PAL_USER);  // 실제 물리 페이지 할당
    
    if (frame->kva == NULL) {
        free(frame);
        frame = vm_evict_frame(); // 스왑 아웃
        ASSERT (frame != NULL); // vm_evict_frame이 NULL을 반환하지 않는지 확인
    } else {
        list_push_back(&frame_table, &frame->frame_elem);	// 프레임 테이블에 추가
    }
    
    frame->page = NULL;

    ASSERT (frame->page == NULL);
    return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */

	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */

	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	hash_init(&spt->spt_hash, hash_func, less_func, NULL); // 해시 테이블 초기화
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}
