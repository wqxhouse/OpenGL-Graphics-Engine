#pragma once
#include "assert.h"

template <class Type> 
class FactoryCreator
{
public:
	explicit FactoryCreator(int type) 
	{
		Factory<Type>::addCreator(type, this);
	}

	virtual ~FactoryCreator() 
	{
		Factory<Type>::delCreator(type);
	}
	
	virtual Type *create() const
	{
		return new Type();
	}

};

template <class Type>
class Factory
{
public:
	static const int NUM_TYPES = 20;

	Factory(void) {}
	~Factory(void) {}

private:
	friend class FactoryCreator<Type>;
	static FactoryCreator<Type> *creatorArr[NUM_TYPES];

	static void addCreator(int type, FactoryCreator *pCreator)
	{
		assert(type >= 0 && type < NUM_TYPES && "Factory::addCreator(): bad type");
		assert(creators[type] == 0 && "Factory::addCreator(): creator is already defined");
		creatorArr[type] = creator;
	}

	static void delCreator(int type)
	{
		assert(type >= 0 && type < NUM_TYPES && "Factory::delCreator(): bad type");
		assert(creators[type] != 0 && "Factory::delCreator(): creator is NULL");
		creatorArr[type] = 0;
	}
};

template <class Type>
FactoryCreator* Factory::creatorArr[Factory<Type>::NUM_TYPES] = {0, };


