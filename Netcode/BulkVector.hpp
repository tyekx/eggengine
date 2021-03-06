#pragma once

#include <cstdint>
#include <NetcodeFoundation/Platform.h>

namespace Netcode {

	template<typename T, size_t BULK_SIZE = 512>
	class BulkVector {
	public:
		static_assert(sizeof(T) >= sizeof(void *), "Freelist need at least sizeof(void*) bytes to work properly");

		struct FreeListItem {
			FreeListItem * next;

			FreeListItem() : next{ nullptr } { }
			~FreeListItem() noexcept {
				next = nullptr;
			}
		};

		struct Bulk {
			T * storage;
			Bulk * next;
			size_t nextIndex;

			Bulk() : storage{ nullptr }, next{ nullptr }, nextIndex{ 0 } {
				storage = static_cast<T *>(std::malloc(sizeof(T) * BULK_SIZE));
			}

			~Bulk() noexcept {
				std::free(storage);
				storage = 0;
				nextIndex = 0;
			}

			Bulk(const Bulk &) = delete;
			Bulk & operator=(const Bulk &) = delete;

			Bulk(Bulk &&) = default;
			Bulk & operator=(Bulk &&) = default;

			size_t GetSize() const {
				return nextIndex;
			}

			T * Next() {
				size_t idx = nextIndex;
				nextIndex += 1;
				return storage + idx;
			}

			bool CanHost() const {
				return nextIndex < BULK_SIZE;
			}
		};

		class Iterator {


			FreeListItem * freeListHead;
			Bulk * bulkHead;
			size_t idx;

			bool TrySetIndex(size_t value) {
				for(T * iter = bulkHead->storage + value;
					(static_cast<void *>(iter) == static_cast<void *>(freeListHead)) && value < BULK_SIZE;
					value += 1, iter += 1, freeListHead = freeListHead->next);

				if(value >= bulkHead->nextIndex) {
					return false;
				}

				idx = value;

				return value != BULK_SIZE;
			}


		public:
			Iterator(FreeListItem * flHead, Bulk * bulkHead) : freeListHead{ flHead }, bulkHead{ bulkHead }, idx{ 0 } {
				while(bulkHead != nullptr && !TrySetIndex(0)) {
					bulkHead = bulkHead->next;
				}
			}

			bool operator!=(std::nullptr_t) const {
				return bulkHead != nullptr;
			}

			Iterator & operator++() {
				if(TrySetIndex(idx + 1)) {
					return *this;
				}

				do {
					bulkHead = bulkHead->next;
				} while(bulkHead != nullptr && !TrySetIndex(0));

				return *this;
			}

			T * operator->() {
				return bulkHead->storage + idx;
			}
		};

	protected:
		FreeListItem * freeListHead;
		FreeListItem * freeListTail;

		void AppendFreeListItem_UnsortedImpl(FreeListItem * item) {
			if(freeListHead == nullptr) {
				freeListHead = item;
				freeListTail = item;
			} else {
				freeListTail->next = item;
				freeListTail = item;
			}
		}

		void AppendFreeListItem_SortedImpl(FreeListItem * item) {
			// init case: freeList is empty
			if(freeListHead == nullptr) {
				freeListHead = item;
				freeListTail = item;
				item->next = nullptr;
				return;
			}


			// first case: item will be the next head, tail will stay in place
			if(freeListHead > item) {
				item->next = freeListHead;
				freeListHead = item;
				return;
			}

			// general case: item will be somewhere in the middle or the end
			for(FreeListItem * iter = freeListHead; iter != nullptr; iter = iter->next) {
				if(iter < item && (item < iter->next || iter->next == nullptr)) {

					if(freeListTail == iter) {
						freeListTail = item;
					}

					item->next = iter->next;
					iter->next = item;
					return;
				}
			}

	#if defined(NETCODE_DEBUG)
			// should never reach this code
			throw std::exception("unexpected control flow");
	#endif
		}

		void * GetFreeListHead() {
			if(freeListHead == nullptr) {
				return nullptr;
			}

			FreeListItem * item = freeListHead;
			freeListHead = freeListHead->next;
			item->~FreeListItem();
			return item;
		}

		void AppendFreeListItem(FreeListItem * item) {
			AppendFreeListItem_SortedImpl(item);
		}

		Bulk * head;

		void InsertBulk_SortedImpl(Bulk * bulk) {
			if(head > bulk) {
				bulk->next = head;
				head = bulk;
				return;
			}

			for(Bulk * iter = head; iter != nullptr; iter = iter->next) {
				if(iter < bulk && (bulk < iter->next || iter->next == nullptr)) {
					bulk->next = iter->next;
					iter->next = bulk;
					return;
				}
			}
		}

		void InsertBulk(Bulk * bulk) {
			if(head == nullptr) {
				head = bulk;
				return;
			}

			InsertBulk_SortedImpl(bulk);
		}

		Bulk * GetValidBulk() {
			for(Bulk * iter = head; iter != nullptr; iter = iter->next) {
				if(iter->CanHost()) {
					return iter;
				}
			}

			Bulk * bulk = new Bulk();

			InsertBulk(bulk);

			return bulk;
		}

	public:
		BulkVector() : freeListHead{ nullptr }, freeListTail{ nullptr }, head{ nullptr } { }
		BulkVector(BulkVector && rhs) : BulkVector{} {
			std::swap(freeListHead, rhs.freeListHead);
			std::swap(freeListTail, rhs.freeListTail);
			std::swap(head, rhs.head);
		}
		BulkVector & operator=(BulkVector && rhs) noexcept {
			std::swap(freeListHead, rhs.freeListHead);
			std::swap(freeListTail, rhs.freeListTail);
			std::swap(head, rhs.head);
			return *this;
		}

		BulkVector(const BulkVector &) = delete;
		BulkVector & operator=(const BulkVector &) = delete;
		~BulkVector() noexcept {
			Clear();
		}

		template<typename ... U>
		T * Emplace(U && ... args) {
			void * ptr = GetFreeListHead();

			if(!ptr) {
				Bulk * bulk = GetValidBulk();
				ptr = bulk->Next();
			}

			return new (ptr) T(std::forward<U>(args)...);
		}

		Iterator begin() const {
			return Iterator(freeListHead, head);
		}

		void Clear() {
			if(head == nullptr) {
				return;
			}

			for(auto it = begin(); it != nullptr; ++it) {
				T * ptr = it.operator->();
				ptr->~T();
			}

			Bulk * iter = head;
			while(iter->next != nullptr) {
				Bulk * tmp = iter->next;
				delete iter;
				iter = tmp;
			}

			head = iter;
			head->nextIndex = 0;

			freeListHead = nullptr;
			freeListTail = nullptr;
		}

		void Remove(T * ptr) {
			ptr->~T();

	#if defined(_DEBUG)
			// debug version will zero the memory to make errors a bit more easier to catch
			memset(ptr, 0, sizeof(T));
	#endif

			FreeListItem * item = reinterpret_cast<FreeListItem *>(ptr);

			AppendFreeListItem(new (item) FreeListItem());
		}
	};

}

