#include "tools.hh"
#include <node.h>
#include <string>
#include <vector>

// Definitions of Matrix functions.

template <typename T>
Matrix<T>::Matrix(int rows, int cols)
{
	this->row_count = rows;
	this->col_count = cols;

	// Create matrix representation.
	this->matrix = std::vector<std::vector<T>>();
	// Explicitly set number of rows.
	this->matrix.resize(rows);
	// Explicitly set number of columns for each row.
	for (int i = 0; i < rows; i++) {
		this->matrix[i].resize(cols);
	}
}

template <typename T>
int Matrix<T>::rows()
{
	return this->row_count;
}

template <typename T>
void Matrix<T>::resize_rows(int n)
{
	this->row_count = n;
	this->matrix.resize(n);
}

template <typename T>
int Matrix<T>::cols()
{
	return this->col_count;
}

template <typename T>
void Matrix<T>::resize_cols(int m)
{
	this->col_count = m;
	for (std::vector<T>& vector : this->matrix)
	{
		vector.resize(m);
	}
}

template <typename T>
void Matrix<T>::set(int i, int j, T value)
{
	this->matrix[i][j] = value;
}

template <typename T>
T Matrix<T>::get(int i, int j)
{
	return this->matrix[i][j];
}

template <typename T>
std::string Matrix<T>::toString(int precision, bool verbose, int padding)
{
	std::string output = "";
	for (int i = 0; i < this->row_count; i++)
	{
		std::string row = "";
		for (int j = 0; j < this->col_count; j++)
		{
			if (j > 0) row += " ";
			std::string item = tools::toString(matrix[i][j], precision);
			if (padding > item.length()) item.insert(item.begin(), padding - item.length(), ' ');
			row += item;
		}
		if (verbose) row = "[ " + row + " ]";
		output += row + "\n";
	}
	return output;
}

template <typename T>
v8::Local<v8::Array> Matrix<T>::toJSArray(v8::Isolate* isolate)
{
	v8::Local<v8::Array> output = v8::Array::New(isolate, row_count);
	for (int i = 0; i < row_count; i++)
	{
		v8::Local<v8::Array> row = v8::Array::New(isolate, col_count);
		for (int j = 0; j < col_count; j++)
		{
			row->Set(j, v8::Number::New(isolate, matrix[i][j]));
		}
		output->Set(i, row);
	}
	return output;
}

template <typename T>
std::vector<T>& Matrix<T>::operator[](int i)
{
	return this->matrix[i];
}
