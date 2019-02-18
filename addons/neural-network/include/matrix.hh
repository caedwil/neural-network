#ifndef MATRIX_HH
#define MATRIX_HH

#include <node.h>
#include <string>
#include <vector>

template <typename T>
class Matrix
{
public:
	Matrix(int rows = 1, int cols = 1);

	// Returns the number of rows (n).
	int rows();
	// Sets the number of rows (n).
	void resize_rows(int n);
	// Returns the number of columns (m).
	int cols();
	// Sets the number of columns (m).
	void resize_cols(int m);
	// Sets the value of an item at position (i, j).
	void set(int i, int j, T value);
	// Returns the item at position (i, j).
	T get(int i, int j);

	// Returns a string representation of the matrix.
	std::string toString(int precision = 6, bool verbose = false, int padding = 0);
	// Returns a JS array representation of the matrix.
	v8::Local<v8::Array> toJSArray(v8::Isolate* isolate);

	// Array subscript overloading.
	std::vector<T>& operator[](int i);
private:
	// The vector representation of the matrix.
	std::vector<std::vector<T>> matrix;

	// The number of rows.
	int row_count;
	// The number of columns.
	int col_count;
};

#include "matrix.inl"

#endif
