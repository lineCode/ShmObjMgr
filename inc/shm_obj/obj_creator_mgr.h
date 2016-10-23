#ifndef OBJ_CREATOR_MGR_H_
#define OBJ_CREATOR_MGR_H_

#include <new>
#include "singleton.h"
#include "shm_obj.h"

namespace shm_obj {

class IObjectCreator {
public:	
	virtual void* ReplacementNew(void* addr) = 0;	
	virtual void* New() = 0;
	virtual void Deconstructor(void* addr) = 0;
};

template <typename T>
class ObjectCreator: public IObjectCreator, public Singleton<ObjectCreator<T> > {
public:
	DeclareSingleton(ObjectCreator);
	
	virtual void* New() {
		return new T();
	}
	
	virtual void* ReplacementNew(void * addr) {
		return new(addr) T;	
	}

	virtual void Deconstructor(void* addr) {
		T* obj = reinterpret_cast<T*>(addr);
		obj->~T();
	}
};

/// class id 必须大于零
class ClassIdGenerator {
public:
	static const ClassId INVALID_CLASS_ID = 0;
	static ClassId NewClassId() {
		return ++class_id_;
	};
private:
	static ClassId class_id_; 
};

#define DeclareTypeName(ClassName)	static ClassId TYPE
//#define ImplmentTypeName(ClassName) const ClassTypeEnum ClassName::TYPE = ClassTypeEnum::ClassName
#define ImplmentTypeName(ClassName)			ClassId ClassName::TYPE = ( \
												ClassId class_id = ClassIdGenerator::NewClassId(); \
												assert(class_id != ClassIdGenerator::INVALID_CLASS_ID); \
												assert(0 == ObjectCreator::Instance()::RegisterObjectCreator<ClassName>(class_id)); \
												class_id; \
											)

class ObjectCreatorMgr : public Singleton<ObjectCreatorMgr> {
public:
	DeclareSingleton(ObjectCreatorMgr);
public:
	static const int MAX_CLASS_ID = 0xff;
public:
	template <typename T>	
	T* CreateObject(void* addr)	{
		IObjectCreator* creator = obj_creator_[T::TYPE];
		void* obj = creator->ReplacementNew(addr);
		return reinterpret_cast<T>(obj);
	}
	
	template <typename T> 
	int RegisterObjectCreator() {			
		obj_creator_[T::TYPE] = ObjectCreator<T>::InstancePtr();
		return 0;
	}

	template <typename T> 
	int RegisterObjectCreator(ClassId class_id) {			
		if (class_id >= MAX_CLASS_ID) {
			return -1;
		}

		obj_creator_[class_id] = ObjectCreator<T>::InstancePtr();
		return 0;
	}

	void* CreateObject(void* addr, ClassId class_id) {
		IObjectCreator* creator = obj_creator_[class_id];
		void* obj = creator->ReplacementNew(addr);
		return obj;
	}

	void FreeObject(void* addr, ClassId class_id) {
		IObjectCreator* creator = obj_creator_[class_id];
		creator->Deconstructor(addr);
	}
private:
	IObjectCreator* obj_creator_[MAX_CLASS_ID];	
};

} //namespace

#endif //OBJ_CREATOR_MGR_H_
