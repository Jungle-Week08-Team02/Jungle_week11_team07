#ifndef VM_VM_H
#define VM_VM_H
#include <stdbool.h>
#include <hash.h>
#include "threads/palloc.h"

enum vm_type {
	/* page not initialized */
	VM_UNINIT = 0,
	/* page not related to the file, aka anonymous page */
	VM_ANON = 1,
	/* page that realated to the file */
	VM_FILE = 2,
	/* page that hold the page cache, for project 4 */
	VM_PAGE_CACHE = 3,

	/* Bit flags to store state */

	/* Auxillary bit flag marker for store information. You can add more
	 * markers, until the value is fit in the int. */
	VM_MARKER_0 = (1 << 3),
	VM_MARKER_1 = (1 << 4),

	/* DO NOT EXCEED THIS VALUE. */
	VM_MARKER_END = (1 << 31),
};

#include "vm/uninit.h"
#include "vm/anon.h"
#include "vm/file.h"
#ifdef EFILESYS
#include "filesys/page_cache.h"
#endif

struct page_operations;
struct thread;

#define VM_TYPE(type) ((type) & 7) 

/* "페이지"의 표현
 * 이는 "부모 클래스"와 같으며, "자식 클래스"로  
 * uninit_page, file_page, anon_page, page_cache(project4)가 있습니다.
 * 이 구조체의 미리 정의된 멤버를 제거하거나 수정하지 마십시오. */
struct page {
	const struct page_operations *operations;
	void *va;              /* Address in terms of user space */
	struct frame *frame;   /* Back reference for frame: 프레임 역참조 */

	/* Your implementation */
	struct hash_elem hash_elem;	// 해시 객체 추가
	bool writable;			// 페이지가 쓰기 가능한지

	/* 각 유형별 데이터가 union에 바인딩됩니다.
	 * 각 함수는 자동으로 현재 union을 감지합니다. */
	union {
		struct uninit_page uninit;
		struct anon_page anon;
		struct file_page file;
#ifdef EFILESYS
		struct page_cache page_cache;
#endif
	};
};

/* The representation of "frame" */
struct frame {
	void *kva;
	struct page *page;

	struct list_elem frame_elem;	// 프레임 리스트 요소
};

/* 페이지 작업을 위한 함수 테이블
 * 이는 C에서 "인터페이스"를 구현하는 한 가지 방법입니다.
 * "메서드" 테이블을 구조체의 멤버에 넣고 필요할 때 호출합니다. */
struct page_operations {
	bool (*swap_in) (struct page *, void *);
	bool (*swap_out) (struct page *);
	void (*destroy) (struct page *);
	enum vm_type type;
};

#define swap_in(page, v) (page)->operations->swap_in ((page), v)
#define swap_out(page) (page)->operations->swap_out (page)
#define destroy(page) \
	if ((page)->operations->destroy) (page)->operations->destroy (page)


/* 현재 프로세스의 메모리 공간을 표현합니다.
 * 이 구조체에 특정 디자인을 강제하지 않습니다. 
 * All designs up to you for this. */
struct supplemental_page_table {
	struct hash spt_hash; // 해시 테이블 사용
};

#include "threads/thread.h"
void supplemental_page_table_init (struct supplemental_page_table *spt);
bool supplemental_page_table_copy (struct supplemental_page_table *dst,
		struct supplemental_page_table *src);
void supplemental_page_table_kill (struct supplemental_page_table *spt);
struct page *spt_find_page (struct supplemental_page_table *spt,
		void *va);
bool spt_insert_page (struct supplemental_page_table *spt, struct page *page);
void spt_remove_page (struct supplemental_page_table *spt, struct page *page);

void vm_init (void);
bool vm_try_handle_fault (struct intr_frame *f, void *addr, bool user,
		bool write, bool not_present);

#define vm_alloc_page(type, upage, writable) \
	vm_alloc_page_with_initializer ((type), (upage), (writable), NULL, NULL)
bool vm_alloc_page_with_initializer (enum vm_type type, void *upage,
		bool writable, vm_initializer *init, void *aux);
void vm_dealloc_page (struct page *page);
bool vm_claim_page (void *va);
enum vm_type page_get_type (struct page *page);

#endif  /* VM_VM_H */
