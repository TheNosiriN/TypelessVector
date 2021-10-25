#include <chrono>
#include <string>
#include <fstream>
#include <sstream>

template<typename Row_t, typename Col_t, int RowsNum, int ColumnsNum>
struct PandasJSON {
	std::stringstream outputStream;
	std::string name;

	using uint = size_t;
	Row_t rows[RowsNum];
	Col_t cols[ColumnsNum];
	double data[RowsNum][ColumnsNum];


	PandasJSON(std::string filename, Row_t* _rows, Col_t* _cols) : name(filename) {
		for (uint i=0; i<RowsNum; rows[i]=_rows[i], ++i);
		for (uint i=0; i<ColumnsNum; cols[i]=_cols[i], ++i);
		for (uint i=0; i<RowsNum; ++i){
			for (uint j=0; j<ColumnsNum; data[i][j]=0.0, ++j);
		}
	}
	~PandasJSON(){
		Flush();
	}

	inline void Flush(){
		outputStream << "{";

		/// write columns
		outputStream << "\"columns\":[\"columns\",";
		for (uint i=0; i<ColumnsNum-1; ++i){
			outputStream << "\"" << cols[i] << "\",";
		}
		outputStream << "\"" << cols[ColumnsNum-1] << "\"" << "],";

		/// write rows
		outputStream << "\"index\":[";
		for (uint i=0; i<RowsNum-1; ++i){
			outputStream << "\"" << i << "\",";
		}
		outputStream << "\"" << (RowsNum-1) << "\"" << "],";

		/// write data
		outputStream << "\"data\":[";
		for (uint i=0; i<RowsNum; ++i){
			outputStream << "[" << "\"" << rows[i] << "\",";
			for (uint j=0; j<ColumnsNum; ++j){
				outputStream << "\"" << data[i][j] << "\"";
				if (j < ColumnsNum-1){ outputStream << ","; };
			}
			outputStream << "]";
			if (i < RowsNum-1){ outputStream << ","; };
		}
		outputStream << "]";


		outputStream << "}";
		outputStream.flush();

		std::ofstream file;
		file.open(name);
		file << outputStream.str();
		file.close();
	}


	template<typename T>
	inline void _Runfunc(uint row, T&& func, uint& i){
		if (i >= ColumnsNum)return;
		data[row][i] = func(rows[row], cols[i]);
		i += 1;
	}

	template<typename... T>
	inline void InsertRow(uint rowIndex, T&&... funcs){
		uint i = 0;
		using expander = int[];
		(void)expander{0, (void(_Runfunc(rowIndex, std::forward<T>(funcs), i)), 0)...};
	}

};




#include <vector>
#include "../TypelessVector.h"



using namespace std::chrono_literals;
#define MEASURE_TIME(__func) {											\
	auto __start = std::chrono::high_resolution_clock::now(); 			\
	__func 																\
	double __elapsedTime = std::chrono::duration<double, std::milli>(	\
		std::chrono::high_resolution_clock::now() - __start				\
	).count();															\
	return __elapsedTime;												\
};





struct Transform{
	float x,y,z;
};


// Hexo::TypelessVector ta(Transform{});
// Hexo::TypesafeTypelessVector tas(Transform{});
// std::vector<Transform> v;


int main() {


	auto taf = [&](size_t num, std::string name){
		Hexo::TypelessVector v(Transform{});
		MEASURE_TIME(
			for (int i=0; i<num; ++i){
				v.push_back( Transform{i+2.f,i*2.f,i/2.f} );
				auto p = v.size()-1;
				if (i % 4 == 2){
					v.erase(p);
				}
			}
			v.clear();
			return double(v.capacity()*sizeof(Transform));
		);
	};

	auto tasf = [&](size_t num, std::string name){
		Hexo::TypesafeTypelessVector v(Transform{});
		MEASURE_TIME(
			for (int i=0; i<num; ++i){
				v.push_back( Transform{i+2.f,i*2.f,i/2.f} );
				auto p = v.size()-1;
				if (i % 4 == 2){
					v.erase(p);
				}
			}
			v.clear();
			return double(v.capacity()*sizeof(Transform));
		);
	};

	auto vec = [&](size_t num, std::string name){
		std::vector<Transform> v;
		MEASURE_TIME(
			for (int i=0; i<num; ++i){
				v.push_back( Transform{i+2.f,i*2.f,i/2.f} );
				auto p = v.size()-1;
				if (i % 4 == 2){
					v.erase(v.begin()+p);
				}
			}
			v.clear();
			return double(v.capacity()*sizeof(Transform));
		);
	};



	const size_t rcount = 50.;
	size_t r[rcount];
	for (size_t i=0; i<rcount; ++i){
		r[i] = (i+1) * 10'000'000/rcount;
	}


	std::string c[] = {"StandardVector", "TypelessVector", "TypesafeTypelessVector"};


	PandasJSON<size_t, std::string, rcount, 3> p(

#if defined(__clang__)
		"resultsClang_Mem.json",
#elif defined(__GNUC__) || defined(__GNUG__)
		"resultsGCC_Mem.json",
#elif defined(_MSC_VER)
		"resultsMSVC_Mem.json",
#endif

		r, c
	);

	for (int i=0; i<rcount; ++i){
		p.InsertRow(i, vec, taf, tasf);
		printf("recorded row: %u\n", i);
	}

}
