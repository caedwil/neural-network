#ifndef DATA_CLASS_NEW_HH
#define DATA_CLASS_NEW_HH

#include "matrix.hh"
#include "random.hh"
#include <node.h>
#include <node_object_wrap.h>
#include <string>
#include <vector>

namespace ANN
{
  using v8::Array;
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::Persistent;
  using v8::Value;

  class DataClass : public node::ObjectWrap
  {
  public:
    // Used for initiating DataClass in NodeJS.
    static void Init(Local<Object> exports);

    Matrix<double> data = NULL;
  private:
    // Used for constructing new instances of DataClass.
    static Persistent<Function> constructor;
    static void New(const FunctionCallbackInfo<Value>& args);

    // Used for random number generation.
    static Random random;

    //
    int rows = 0;
    int cols = 0;
    int attributes = 0;
    std::vector<char> delims = std::vector<char>({ ' ', ',', '\t' });

    // Original minimum of each column.
    std::vector<double> normMin;
    // Original maximum of each column.
		std::vector<double> normMax;
		// Original multiplier after subtracting min.
		std::vector<double> normMul;

    // Constructors.
    explicit DataClass();
    explicit DataClass(int rows);
    explicit DataClass(int rows, int cols);
    explicit DataClass(const std::string& path);
    explicit DataClass(Matrix<double>& matrix);

    // Returns the data matrix as a rectangular JavaScript array.
    static void GetMatrix(const FunctionCallbackInfo<Value>& args);
    // Returns a string of all the data.
    static void ShowAll(const FunctionCallbackInfo<Value>& args);
    // Returns a string of the specified range of rows and columns.
    static void ShowPart(const FunctionCallbackInfo<Value>& args);
    // Normalises the data.
    static void Normalise(const FunctionCallbackInfo<Value>& args);
    // Splits the data into output and remainder. A specified number
    // of them go into output, the rest into remainder.
    static void ExtractSplit(const FunctionCallbackInfo<Value>& args);
    // Splits the data into the specified number of sets with an even
    // amount of data in each (the last set holds any remainder as well).
    static void SplitEvenly(const FunctionCallbackInfo<Value>& args);
    // Splits the data into sets of sizes specified.
    // e.g. splitByAmount(100, 50) -> splits into 2 sets, first has 100
    // points, second has 50.
    static void SplitByAmount(const FunctionCallbackInfo<Value>& args);
    // Splits the data into sets holding the specified percentage of data.
    // e.g. splitByPercentage(66, 34).
    static void SplitByPercentage(const FunctionCallbackInfo<Value>& args);
    // Returns an exemplared DataClass.
    static void GetExemplar(const FunctionCallbackInfo<Value>& args);
    // Alters the inner data of the DataClass to an exemplar set.
    static void MakeExemplar(const FunctionCallbackInfo<Value>& args);
    // Reads data in from a file.
    static void ReadFromFile(const FunctionCallbackInfo<Value>& args);
    // Writes data (matrix) out to a file (truncating if necessary).
    static void WriteToFile(const FunctionCallbackInfo<Value>& args);

    // Generates a sequence of random indices of length count.
    static std::vector<int> GenerateSequence(int count);
    // Splits data from the parent DataClass into the sets vector using
    // the sequence vector to determine order.
    static void SplitIntoSets(DataClass* cls, std::vector<DataClass*>& sets, std::vector<int>& sequence);
    // Generates a JavaScript correspondent array from a vector of sets.
    static Local<Array> CreateArrayFromSets(Isolate* isolate, std::vector<DataClass*>& sets);

    void _ReadFromFile(const std::string& path, Isolate* isolate = NULL);

    bool set_row(int index, std::vector<double>& row);
    void resize_rows(int rows);
    void resize_cols(int cols);
  };
}

#endif
