#pragma once
// self packing freelist based on https://nlguillemot.wordpress.com/2016/11/18/opengl-renderer-design/

#include <limits>
#include <cinttypes>
#include <array>
#include <assert.h>
#include <type_traits>

template <typename T, size_t size>
class PackedFreelist
{
public:

	//==================================================================
	// Ctor
	//=================================================================
	PackedFreelist() : KMax_Objects(size), num_objects_(0), next_tracker_index(0),
		last_tracker_index((size > 0) ? static_cast<uint16_t>(size) - 1 : 0), objects_{}, objects_to_trackers_{}, trackers_{}
	{
		static_assert(size < std::numeric_limits<uint16_t>::max() - 1, "size must be less than max uint16_t");
		//initialize the trackers
		for (size_t i = 0; i < KMax_Objects; ++i)
		{
			trackers_[i].SetIndex(static_cast<uint16_t>(i));
			trackers_[i].SetId(KInvalid);
			trackers_[i].obj_index = KInvalid;
			trackers_[i].next_tracker = static_cast<uint16_t>(i) + 1;
			trackers_[i].prev_id = 0;
		}

		trackers_[last_tracker_index].next_tracker = 0;

	}

	//===============================================================
	// Copy Ctor
	//=================================================================
	PackedFreelist(const PackedFreelist& other) noexcept(std::is_nothrow_copy_constructible<T>::value): 
		KMax_Objects(other.KMax_Objects),
		num_objects_(other.num_objects_),
		next_tracker_index(other.next_tracker_index),
		last_tracker_index(other.last_tracker_index),
		trackers_(other.trackers_),
		objects_to_trackers_(other.objects_to_trackers_),
		objects_{}
	{
		/* 
		std array is not dynamically allocate so it cant throw
		and placement new cant throw, only Ts copy constructor can
		*/
		auto* obj_ptr = objects_.data();
		for (size_t i = 0; i < num_objects_; ++i)
		{
			new(obj_ptr + i) T(other.objects_[i]);
		}
	}

	//=====================================================
	// Copy assingment
	//=====================================================
	PackedFreelist& operator= (const PackedFreelist& other) noexcept(std::is_nothrow_copy_constructible<T>::value) 
	{
		PackedFreelist temp(other);
		Swap(temp);

		return *this;
	}

	//==========================================
	// Move Ctor
	//===========================================
	PackedFreelist(PackedFreelist&& other) noexcept(std::is_nothrow_move_constructible<T>::value) :
		KMax_Objects(size), num_objects_(other.num_objects_),
		next_tracker_index(other.next_tracker_index),
		last_tracker_index(other.last_tracker_index),
		objects_to_trackers_(std::move(other.objects_to_trackers_)),
		trackers_(std::move(other.trackers_)),
		objects_(),
	{

		auto* obj_ptr = objects_.data();
		for (size_t i = 0; i < num_objects_; ++i)
		{
			new(obj_ptr + i) T(std::move(other[i]));
		}

	}

	//============================================
	// Move assignment
	//=============================================
	PackedFreelist& operator=(PackedFreelist&& other) noexcept(std::is_nothrow_move_assignable<T>::value)
	{
		Swap(other);
		return *this;
	}

	//============================================
	// Swap
	//==============================================
	void Swap(PackedFreelist& other) noexcept(noexcept(std::swap(std::declval<T&>(), std::declval<T&>())))
	{
		using std::swap;
		swap(num_objects_, other.num_objects_);
		swap(next_tracker_index, other.next_tracker_index);
		swap(last_tracker_index, other.last_tracker_index);
		swap(trackers_, other.trackers_);
		swap(objects_to_trackers_, other.objects_to_trackers_);

		for (size_t i = 0; i < num_objects_; ++i)
			swap(objects_[i], other.objects_[i]);

	}

	//================================================
	// Check if object and tracker are still valid
	//=================================================
	bool Contains(uint32_t id) const
	{
		auto& tracker = trackers_[GetIndex(id)];
		return tracker.tracker_id_index == id && tracker.obj_index != KInvalid;
	}

	//======================================
	// Return non-const T
	//========================================
	const T& operator[] (uint32_t id) const
	{
		auto& tracker = trackers_[GetIndex(id)];
		return objects_[tracker.obj_index];
	}

	//======================================
	// Return non-const T
	//========================================
	T& operator[] (uint32_t id)
	{
		auto& tracker = trackers_[GetIndex(id)];
		return objects_[tracker.obj_index];
	}

	//==============================================
	// Copy add
	//===============================================
	uint32_t Add(const T& t)
	{
		auto tracker = GetNextTracker();
		auto* obj = objects_.data() + tracker.obj_index;

		new(obj) T(t);
		return tracker.tracker_id_index;
	}

	//===================================
	// Move add
	//===================================
	uint32_t Add(T&& t)
	{
		auto tracker = GetNextTracker();
		auto* obj = objects_.data() + tracker.obj_index;

		new(obj) T(std::move(t));
		return tracker.tracker_id_index;
	}

	//=====================================
	// Construct object in container
	//========================================
	template <typename... Args>
	uint32_t Emplace(Args&&... args)
	{
		auto tracker = GetNextTracker();
		auto* obj = objects_.data() + tracker.obj_index;

		new(obj) T(std::forward<Args>(args)...);
		return tracker.tracker_id_index;
	}

	//=============================================
	// Erase object if id is valid
	//=============================================
	void Erase(uint32_t id)
	{
		if (!Contains(id))
			return;

		auto& erased_tracker = trackers_[GetIndex(id)];
		auto erased_obj_index = erased_tracker.obj_index;
		auto* obj = objects_.data() + erased_obj_index;
		// If obj not last in storage
		if (erased_obj_index != (num_objects_ - 1))
		{
			auto* last_obj = objects_.data() + (num_objects_ - 1);
			*obj = std::move(*last_obj);
			obj = last_obj;

			// We need to update the moved objects tracker
			auto last_tracker_index = objects_to_trackers_[erased_obj_index] = objects_to_trackers_[num_objects_ - 1];
			trackers_[GetIndex(last_tracker_index)].obj_index = erased_obj_index;
		}

		obj->~T();	
		num_objects_ -= 1;
		InvalidateTracker(erased_tracker.tracker_id_index);
	}

	//===========================================
	// Number of objects in container
	//============================================
	size_t Size()
	{
		return num_objects_;
	}

	//=============================================
	// Maximum size of container
	//=============================================
	size_t Capacity()
	{
		return KMax_Objects;
	}

	//===============================
	// Is container empty
	//===============================
	bool Empty()
	{
		return num_objects_ == 0;
	}

	/* forward iterator for fast iteration */
	struct Iterator
	{
		Iterator(T* ptr, uint32_t* obj_id) : ptr_(ptr), obj_id_(obj_id) {}
		Iterator(const Iterator& other) = default;

		T& operator*() { return *ptr_; }
		Iterator& operator++() { ptr_ += 1; obj_id_ += 1; return *this; }

		uint32_t ID() { return *obj_id_; }

		bool operator!=(const Iterator& other) const { return ptr_ != other.ptr_; }
		bool operator==(const Iterator& other) const { return ptr_ == other.ptr_; }

	private:
		T* ptr_;
		uint32_t* obj_id_;
	};

	struct Const_Iterator
	{
		Const_Iterator(T* ptr, uint32_t* obj_id) : ptr_(ptr), obj_id_(obj_id) {}
		Const_Iterator(const Const_Iterator& other) = default;

		const T& operator*() const { return *ptr_; }
		Iterator& operator++() { ptr_ += 1; obj_id_ += 1; return *this; }

		uint32_t ID() { return *obj_id_; }

		bool operator!=(const Const_Iterator& other) const { return ptr_ != other.ptr_; }
		bool operator==(const Const_Iterator& other) const { return ptr_ == other.ptr_; }

	private:
		T* ptr_;
		uint32_t obj_id_;
	};

	//=======================================
	// Return iterator to first element
	//=========================================
	Iterator Begin()
	{
		return Iterator(objects_.data(), objects_to_trackers_.data());
	}

	//=======================================
	// Return const iterator to first element
	//=========================================
	Const_Iterator Begin() const
	{
		return Const_Iterator(objects_.data(), objects_to_trackers_.data());
	}

	//=============================================
	// Return iterator to last element + 1
	//==============================================
	Iterator End()
	{
		return Iterator(objects_.data() + num_objects_, objects_to_trackers_.data() + num_objects_);
	}

	//==================================================
	// Return const iterator to last element + 1
	//==================================================
	Const_Iterator End() const
	{
		return Const_Iterator(objects_.data() + num_objects_, objects_to_trackers_.data() + num_objects_);
	}

private:
	/*
	Tracks the objects location in container.
	Tracks if object is valid, i.e. not erased.
	Tracks if object has been moved i.e packed
	*/
	struct ObjTracker
	{
		uint32_t tracker_id_index;
		uint16_t obj_index;
		uint16_t next_tracker;
		uint16_t prev_id;

		uint16_t GetId()
		{
			return static_cast<uint16_t>(tracker_id_index >> 16);
		}

		uint16_t GetIndex()
		{
			return static_cast<uint16_t>(tracker_id_index);
		}

		void  SetId(uint16_t val)
		{
			// Crate a bitmask with value
			uint32_t val_mask = (static_cast<uint32_t>(val) << 16);
			// apply bitmask to set the id
			tracker_id_index |= val_mask;
		}

		void SetIndex(uint16_t val)
		{
			// Crate a bitmask with value
			uint32_t val_mask = static_cast<uint32_t>(val);
			// apply bitmask to set the id
			tracker_id_index |= val_mask;
		}

	};

	//=======================================================
	// Return next available tracker if container is not full
	//========================================================
	ObjTracker& GetNextTracker()
	{
		assert(num_objects_ < KMax_Objects);

		auto& tracker = trackers_[next_tracker_index];
		next_tracker_index = tracker.next_tracker;
		// Set this tracker with valid id
		tracker.SetId(tracker.prev_id + 1);
		// The trackers object, this will always be from end of storage
		tracker.obj_index = static_cast<uint16_t>(num_objects_);
		num_objects_ += 1;
		// Set the reverse lookup to point to this tracker
		objects_to_trackers_[tracker.obj_index] = tracker.tracker_id_index;
		return tracker;
	}

	//===============================================
	// Set tracker as invalid and put at end of FIFO
	//================================================
	inline void InvalidateTracker(uint32_t id)
	{
		// Set the old tracker as invalid and add to FIFO
		auto& tracker = trackers_[GetIndex(id)];
		tracker.prev_id = tracker.GetId();
		// Make sure we dont give KInvalid as an id
		if (tracker.prev_id = KInvalid - 1)
			tracker.prev_id = KInvalid;

		tracker.SetId(KInvalid);
		tracker.obj_index = KInvalid;

		trackers_[last_tracker_index].next_tracker = tracker.GetIndex();
		last_tracker_index = tracker.GetIndex();
	}

	uint16_t GetId(uint32_t id_index)
	{
		return static_cast<uint16_t>(id_index >> 16);
	}

	uint16_t GetIndex(uint32_t id_index)
	{
		return static_cast<uint16_t>(id_index);
	}

	void  SetId(uint32_t& id_index, uint16_t val)
	{
		// Crate a bitmask with value
		uint32_t val_mask = (static_cast<uint32_t>(val) << 16);
		// apply bitmask to set the id
		id_index |= val_mask;
	}

	void SetIndex(uint32_t& id_index, uint16_t val)
	{
		// Crate a bitmask with value
		uint32_t val_mask = static_cast<uint32_t>(val);
		// apply bitmask to set the id
		id_index |= val_mask;
	}

private:
	static const uint16_t KInvalid = 0xFFFF;
	const uint16_t KMax_Objects;
	size_t num_objects_;
	// Next tracker to use from FIFO
	uint16_t next_tracker_index;
	uint16_t last_tracker_index;

	std::array<T, size> objects_;
	std::array<uint32_t, size> objects_to_trackers_;
	std::array<ObjTracker, size> trackers_;
};

template<typename T, size_t size>
void swap(PackedFreelist<T, size>& rhs, PackedFreelist<T, size>& lhs)
{
	rhs.Swap(lhs);
}