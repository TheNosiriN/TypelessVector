# TypelessVector
The "safe" void* vector.



## Usage
TypelessVector can be used like a standard vector but with some differences

### Construction
These are different ways the vector can be constructed
```c++
using namespace Hexo;

using Data = int;

/// 1. TypelessVector is default_constructible
/// but cannot be used unless initialized with a type or stride
TypelessVector v;

/// 2. Using the stride of the Data
TypelessVector v(sizeof(Data));

/// 3. Constructing a temporary reference of Data to obtain its type and stride
TypelessVector v(Data{});

/// 4. Can be post-initialized with either of the following ways
TypelessVector v;
v.init<Data>();
v.init(Data{});
v.init_raw(sizeof(Data));

/// To re-initialize the vector, you could use
v.reset();

```


### Insertion
TypelessVector uses the same function names as std::vector but not the same semantics
```c++
using namespace Hexo;

TypelessVector v(sizeof(int));

/// 1. Data can be pushed back just like vector
int index = v.push_back(10);

/// 2. It can also be emplaced with arguments but the type must be specified so the constructor can be called
int index = v.emplace_back<int>();

```



### Deletion
```c++
using namespace Hexo;

TypelessVector v(sizeof(int));
int index = v.push_back(10);

/// 1. Erase can be called normally but the destructor will not be called
v.erase(index);

/// 2. To have the destructor called the type must be specified
v.erase<int>(index);

/// 3. Erase can also work with iterators just like vector
v.erase(v.begin()+index);
v.erase<int>(v.begin()+index);

```



### Iteration and Lookup
```c++
using namespace Hexo;

TypelessVector v(sizeof(int));
v.push_back(10);

/// 1. Lookup can be achieved using the [] operator, but will return a void* and will return null if nothing is found
void* d = v[0];

/// 2. if you want the type you can use its template variant, this will return a refernce and throw and exception if nothing is found
int& d = v.operator[]<int>(0);

/// or cast it
int& d = v.cast<int>(v[0]);

/// 3. you could also use .at methods
void* d = v.at(0);
int& d = v.at<int>(0);

/// Iterating is trivial but not like a vector
for (void* i : v){
	int* d = a.cast<int>(i);
	...
}

```



## Safety first...
TypelessVector is safe, but also not safe. It's not typesafe. It never knows the type until you specify it. If you don't, it will assume handling the data is trivial like a POD (Plain Old Datatype). To fix this, I made a variant called `TypesafeTypelessVector`

```c++
using namespace Hexo;

/// TypesafeTypelessVector has the same constructors as TypelessVector except for the ones where only the stride is given
/// Don't do this
TypesafeTypelessVector v(sizeof(int));

/// After it is initialized with a type, it will throw an exception if a different type is ever given
TypesafeTypelessVector v(int{});
v.push_back(10);

auto d = v.at<double>(0); /// This will throw an exception

```
\
`TypesafeTypelessVector` uses runtime type checking which can be slow. You are able to implement your own TypeChecker
```c++
using namespace Hexo;

struct MyOwnTypeChecker {
	template<typename T>
	inline void init() {}
	template<typename T>
	inline bool compare() { return true; }
	inline void reset() {}
};

typeless_vector<MyOwnTypeChecker> v;

```



## But Why did I do all this?
You might ask yourself, will you ever need this? You probably wont. But I did. Recently, during HexoECS dev, I came across a problem. I had a structure similar to this
```c++
template<typename T>
struct ComponentPack {
	Container<T> dense;
	...
};
```
And I was storing ComponentPacks of different types in a constexpr array. But how do you store different types in one array? You store their pointers
```c++
void* Packs[NumOfComponentsTypes];
```
This is terrible. its unnecessary allocations. Yet I already know the index of each ComponentPack and their type which was evaluated at compile time, is static and can be retrieved using `T::Index`. So if I already know each of their types and indices, I can simplify it like this...
```c++
struct ComponentPack {
	TypelessVector dense;
	...
};

ComponentPack Packs[NumOfComponentsTypes];

/// And can be used like so
template<typename T>
constexpr inline ComponentPack GetComponentPack() const {
	...
	return Packs[T::Index];
}
```



## Performance
What if I was faster than std::vector?
