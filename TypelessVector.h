#pragma once



#ifndef HEXO_UTILS_TYPELESS_VECTOR
#define HEXO_UTILS_TYPELESS_VECTOR



#include <cstring>
#include <utility>
#include <type_traits>
#include <typeindex>




namespace Hexo {


	struct __NoTypeChecker {
		template<typename T>
		constexpr inline void init() const {}
		template<typename T>
		constexpr inline bool compare() const { return true; }
		constexpr inline void reset() const {}
	};

	struct DefaultTypeChecker {
		std::type_index sign;

		DefaultTypeChecker() : sign(typeid(void)) {}

		template<typename T>
		constexpr inline void init(){ sign = std::type_index(typeid(T)); }
		template<typename T>
		constexpr inline bool compare() const { return (std::type_index(typeid(T)) == sign); }
		inline void reset(){ sign = std::type_index(typeid(void)); }
	};


	template<class TypeChecker = __NoTypeChecker>
	struct typeless_vector {
	private:
		using _Uint = size_t;

		void* m_data = nullptr;
		_Uint m_stride = 0;
		_Uint m_size = 0;
		_Uint m_realsize = 0;

		TypeChecker s_checker;

	public:

		constexpr inline void* operator[](const _Uint& i) const {
			return static_cast<uint8_t*>(m_data) + (i*m_stride);
		}

		template<typename T>
		constexpr inline T& operator[](const _Uint& i) const {
			return *cast<T>( static_cast<uint8_t*>(m_data) + (i*m_stride) );
		}



		struct Iterator {
			typeless_vector* parent = nullptr;
			_Uint index = 0;

			constexpr inline Iterator& operator++(){
				if (parent){ index += 1; }
				return *this;
			}
			constexpr inline Iterator& operator--(){
				if (parent){ index -= (index==0 ? 0 : 1); }
				return *this;
			}
			constexpr inline Iterator operator++(int){
				Iterator t = *this;
				++*this; return t;
			}
			constexpr inline Iterator operator--(int){
				Iterator t = *this;
				--*this; return t;
			}
			constexpr inline Iterator operator+(const _Uint& num) const {
				return Iterator{ this->parent, this->index+num };
			}
			constexpr inline Iterator operator-(const _Uint& num) const {
				return Iterator{ this->parent, this->index-num };
			}
			constexpr inline Iterator& operator+=(const _Uint& num){
				this->index += num;
				return *this;
			}
			constexpr inline Iterator& operator-=(const _Uint& num){
				this->index -= num;
				return *this;
			}

			constexpr inline bool operator!=(const Iterator& t) const { return (this->index != t.index); }
			constexpr inline bool operator==(const Iterator& t) const { return (this->index == t.index); }

			constexpr inline void* operator*() const {
				if (parent){ return parent->at(index); }
				return nullptr;
			}

		};

		constexpr inline Iterator begin() { return Iterator{ this, 0 }; }
		constexpr inline Iterator end() { return Iterator{ this, this->m_size }; }



		typeless_vector() : m_stride(0), m_size(0), m_realsize(0), m_data(nullptr){}

		typeless_vector(const _Uint& stride){
			init_raw(stride);
		}

		template<typename T>
		typeless_vector(const T&){
			__Init<T>();
		}

		~typeless_vector(){ reset(); }


		template<typename T>
		constexpr inline bool __Init(){
			s_checker.template init<T>();
			m_stride = sizeof(T);
			m_size = 0;
			m_realsize = 1;
			m_data = std::malloc(m_realsize * m_stride);
			return (m_data != NULL);
		}

		template<typename T>
		constexpr inline bool init(){
			return __Init<T>();
		}
		template<typename T>
		constexpr inline bool init(const T&){
			return __Init<T>();
		}

		constexpr inline bool init_raw(const _Uint& stride){
			m_stride = stride;
			m_size = 0;
			m_realsize = 1;
			m_data = std::malloc(m_realsize * m_stride);
			return (m_data != NULL);
		}

		inline void reset(){
			std::free(m_data);
			m_data = nullptr;
			m_realsize = 0;
			m_size = 0;
			s_checker.reset();
		}



		template<typename T>
		constexpr inline typename std::enable_if<std::is_same<TypeChecker, __NoTypeChecker>::value == false, T*>::type
		cast(void* p) const {
			if ( s_checker.template compare<T>() == false ){
				throw std::runtime_error("typeless_vector's TypeChecker detected a type different from the one given at last init/construction");
				return nullptr;
			}
			return static_cast<T*>(p);
		}

		template<typename T>
		constexpr inline typename std::enable_if<std::is_same<TypeChecker, __NoTypeChecker>::value == true, T*>::type
		cast(void* p) const {
			return static_cast<T*>(p);
		}



		constexpr inline void* __GetNoCheck(const _Uint& i) const {
			return static_cast<uint8_t*>(m_data) + (i*m_stride);
		}

		constexpr inline bool __ValidateIndex(const _Uint& i) const {
			return !(!m_data || i>=m_size);
		}

		constexpr inline void* at(const _Uint& i) const {
			if (!m_data || i >= m_size)return nullptr;
			return static_cast<uint8_t*>(m_data) + (i*m_stride);
		}

		template<typename T>
		constexpr inline T& at(const _Uint& i) const {
			return *(cast<T>(at(i)));
		}



		constexpr inline bool __CheckForReallocate(){
			if (m_size == m_realsize){
				/// python 3 list resize: https://github.com/python/cpython/blob/2.6/Objects/listobject.c#L48
				_Uint newsize = m_size+1;
				m_realsize += (newsize >> 3) + (newsize < 9 ? 3 : 6);
				///
				m_data = std::realloc(m_data, m_realsize * m_stride);
			}
			return (m_data != NULL);
		}




		constexpr inline void* data() const { return this->m_data; }

		template<typename T>
		constexpr inline T* data() const { return cast<T>(data); }

		constexpr inline _Uint size() const { return this->m_size; }
		constexpr inline _Uint stride() const { return this->m_stride; }
		constexpr inline _Uint capacity() const { return this->m_realsize; }



		template<typename T, typename... Args>
		constexpr inline typename std::enable_if<std::is_constructible<T, Args...>::value == true, void>::type
		__ConstructInPlace(Args&&... a) const {
			cast<T>(m_data)[m_size] = T(std::forward<T>(a)...);
		}

		template<typename T, typename... Args>
		constexpr inline typename std::enable_if<std::is_constructible<T, Args...>::value == false, void>::type
		__ConstructInPlace(Args&&... a) const {
			if (std::is_default_constructible<T>::value){
				cast<T>(m_data)[m_size] = T{a...};
			}else{
				/// at this point, the object can never be constructed with the params the user gave it
				/// so just run its trivial default constructor
				cast<T>(m_data)[m_size] = T{};
			}
		}


		template<typename T, typename... Args>
		constexpr inline _Uint emplace_back(Args&&... a){
			if (!__CheckForReallocate())return 0;

			__ConstructInPlace<T>(a...);
			m_size += 1;
			return m_size-1;
		}

		template<typename T>
		constexpr inline _Uint push_back(const T& obj){
			if (!__CheckForReallocate())return 0;

			cast<T>(m_data)[m_size] = obj;
			m_size += 1;
			return m_size-1;
		}



		constexpr inline void erase(const _Uint& i){
			if (!m_data || i >= m_size)return;
			std::memcpy(
				__GetNoCheck(i), __GetNoCheck(i+1), m_stride*(m_size-1-i)
			);
			std::memset(__GetNoCheck(m_size-1), 0, m_stride);
			m_size -= 1;
		}

		template<typename T>
		constexpr inline void erase(const _Uint& i) {
			T* p = cast<T>(at(i));
			if (p){ (*p).~T(); }
			erase(i);
		}

		constexpr inline void erase(const Iterator& t) {
			erase(t.index);
		}

		template<typename T>
		constexpr inline void erase(const Iterator& t) {
			erase<T>(t.index);
		}



		constexpr inline void clear(){
			std::memset(m_data, 0, m_stride*m_size);
			m_size = 0;
		}

		template<typename T>
		constexpr inline void clear(){
			for (_Uint i=0; i<m_size; ++i){
				T* p = cast<T>(at(i));
				if (p){ (*p).~T(); }
			}
			clear();
		}



		constexpr inline bool reserve(const _Uint& size){
			if (m_size+size <= m_realsize)return true;

			m_realsize += size - (m_realsize-m_size);
			void* d = std::realloc(m_data, m_realsize * m_stride);
			if (!d)return false;
			m_data = d;

			return true;
		}

	};

	typedef typeless_vector<__NoTypeChecker> TypelessVector;
	typedef typeless_vector<DefaultTypeChecker> TypesafeTypelessVector;

}




#endif /* end of include guard: HEXO_UTILS_TYPELESS_VECTOR */
