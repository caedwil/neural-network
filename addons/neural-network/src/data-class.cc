#include "data-class.hh"
#include "tools.hh"
#include <fstream>
#include <math.h>

namespace ANN
{
  using v8::Array;
  using v8::Context;
  using v8::Exception;
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Local;
  using v8::Number;
  using v8::Object;
  using v8::Persistent;
  using v8::String;

  // Initialise static member variables.
  Persistent<Function> DataClass::constructor;
  Random DataClass::random = Random();

  void DataClass::Init(Local<Object> exports)
  {
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template.
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
    tmpl->SetClassName(String::NewFromUtf8(isolate, "DataClass"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add methods to prototype.
    NODE_SET_PROTOTYPE_METHOD(tmpl, "getMatrix", GetMatrix);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "showAll", ShowAll);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "showPart", ShowPart);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "normalise", Normalise);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "extractSplit", ExtractSplit);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "splitEvenly", SplitEvenly);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "splitByAmount", SplitByAmount);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "splitByPercentage", SplitByPercentage);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "getExemplar", GetExemplar);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "makeExemplar", MakeExemplar);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "readFromFile", ReadFromFile);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "writeToFile", WriteToFile);

    // Export new item.
    constructor.Reset(isolate, tmpl->GetFunction());
    exports->Set(
      String::NewFromUtf8(isolate, "DataClass"),
      tmpl->GetFunction()
    );
  }

  DataClass::DataClass()
  {
    this->data = NULL;
  }

  DataClass::DataClass(int rows)
  {
    this->rows = rows;
    this->data = Matrix<double>(rows);
  }

  DataClass::DataClass(int rows, int cols)
  {
    this->rows = rows;
    this->cols = cols;
    this->data = Matrix<double>(rows, cols);
  }

  DataClass::DataClass(const std::string& path)
  {
    //this->ReadFromFile(path);
    this->_ReadFromFile(path);
  }

  DataClass::DataClass(Matrix<double>& matrix)
  {
    this->rows = matrix.rows();
    this->cols = matrix.cols();
    this->data = matrix;
  }

  void DataClass::New(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    if (args.IsConstructCall())
    {
      // DataClass invoked as constructor.
      DataClass* cls = NULL;
      // Check and get arguments. Throw exceptions if incorrect arguments.
      if (args.Length() == 2)
      {
        // Both arguments must be numbers (rows, cols).
        if (!args[0]->IsNumber() || !args[1]->IsNumber())
        {
          isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Incorrect argument types.")
          ));
          return;
        }
        int rows = (int)args[0]->NumberValue();
        int cols = (int)args[1]->NumberValue();
        cls = new DataClass(rows, cols);
      }
      else if (args.Length() == 1)
      {
        // Single argument can be either number of string (rows or path).
        if (args[0]->IsNumber())
        {
          int rows = (int)args[0]->NumberValue();
          cls = new DataClass(rows);
        }
        else if (args[0]->IsString())
        {
          std::string path(*String::Utf8Value(args[0]));
          cls = new DataClass(path);
        }
        else
        {
          isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Incorrect argument type.")
          ));
          return;
        }
      }
      else if (args.Length() == 0)
      {
        cls = new DataClass();
      }
      else
      {
        isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Incorrect number of arguments.")
        ));
        return;
      }
      // Wrap DataClass and return it to the user.
      cls->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    }
    else
    {
      // DataClass invoked as plain function 'DataClass(...)'.
      // Turn into constructor call.
      const int argc = 2;
      Local<Value> argv[argc] = { args[0], args[1] };
      Local<Context> context = isolate->GetCurrentContext();
      Local<Function> construct = Local<Function>::New(isolate, constructor);
      Local<Object> result = construct->NewInstance(context, argc, argv).ToLocalChecked();
      args.GetReturnValue().Set(result);
    }
  }

  void DataClass::GetMatrix(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());
    // Return JavaScript representation of matrix.
    args.GetReturnValue().Set(cls->data.toJSArray(isolate));
  }

  void DataClass::ShowAll(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    // Get arguments.
    int precision = 6;
    if (!args[0]->IsUndefined() && args[0]->IsNumber())
    {
      precision = (int)args[0]->NumberValue();
    }

    // Create output string.
    std::string output = "TOTAL > Rows: " + std::to_string(cls->rows);
    output += ", Columns: " + std::to_string(cls->cols) + "\n";
    output += cls->data.toString(precision);

    // Return output string.
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, output.c_str()));
  }

  void DataClass::ShowPart(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    // Get arguments.
    int numRows = 0;
    int numCols = 0;
    int precision = 6;
    std::string header = "";
    // Throw exception if too few arguments.
    if (args.Length() < 2)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Too few arguments.")
      ));
      return;
    }
    // Get numRows and numCols.
    numRows = (int)args[0]->NumberValue();
    numCols = (int)args[1]->NumberValue();
    // Get precision.
    if (!args[2]->IsUndefined() && args[2]->IsNumber())
    {
      precision = (int)args[2]->NumberValue();
    }
    // Get header.
    if (!args[3]->IsUndefined() && args[3]->IsString())
    {
      header = std::string(*String::Utf8Value(args[3]));
    }

    // Create output string.
    std::string output = "";
    if (tools::trim(header) != "") output += header + "\n";
    output += "TOTAL > Rows: " + std::to_string(cls->rows) + ", ";
    output += "Columns: " + std::to_string(cls->cols) + "\n";
    output += "SHOWN > Rows: " + std::to_string(numRows) + ", ";
    output += "Columns: " + std::to_string(numCols) + "\n";
    for (int i = 0; i < numRows; i++)
    {
      output += "[";
      for (int j = 0; j < numCols; j++)
      {
        output += " " + tools::toString(cls->data[i][j], precision);
      }
      output += " ]\n";
    }

    // Return output string.
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, output.c_str()));
  }

  void DataClass::Normalise(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    // Get arguments (first, last).
    if (args.Length() < 2)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Too few arguments.")
      ));
      return;
    }
    if (!args[0]->IsNumber() || !args[1]->IsNumber())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Arguments must be of type int.")
      ));
      return;
    }

    int first = (int)args[0]->NumberValue();
    int last = (int)args[1]->NumberValue();

    // Normalise specified columns by computed (x - mean) / sd for each value.
		if (cls->normMin.empty()) cls->normMin = std::vector<double>(cls->cols);
		if (cls->normMax.empty()) cls->normMax = std::vector<double>(cls->cols);
		if (cls->normMul.empty()) cls->normMul = std::vector<double>(cls->cols);

		for (int i = first; i <= last; i++)
		{
			// For column i, find min and max.
			double min = cls->data[0][i];
			double max = cls->data[0][i];
			for (int j = 0; j < cls->rows; j++)
			{
				double d = cls->data[j][i];
				if (d < min) min = d;
				if (d > max) max = d;
			}

			double mul = 1 / (max - min);
			for (int j = 0; j < cls->rows; j++)
			{
				double d = cls->data[j][i];
				d = (d - min) * mul;
				cls->data[j][i] = d;
			}
		}
  }

  void DataClass::ExtractSplit(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Get arguments: length (of output).
    if (args[0]->IsUndefined() || !args[0]->IsNumber())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "The first argument (length) must be a number.")
      ));
      return;
    }
    int length = (int)args[0]->NumberValue();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    // Setup output DataClasses.
    DataClass* out = new DataClass(length, cls->cols);
    DataClass* rem = new DataClass(cls->rows - length, cls->cols);

    // Create a random sequence of indices.
    std::vector<int> sequence(cls->rows);
    for (int i = 0; i < sequence.size(); i++) sequence[i] = i;
    for (int i = 0; i < sequence.size(); i++)
    {
      int r = random.nextInt(i, sequence.size());
      int tmp = sequence[r];
      sequence[r] = sequence[i];
      sequence[i] = tmp;
    }

    int j = 0;
    for (int i = 0; i < sequence.size(); i++)
    {
      if (i < length) out->set_row(i, cls->data[sequence[i]]);
      else rem->set_row(j++, cls->data[sequence[i]]);
    }

    // Create result object.
    Local<Object> result = Object::New(isolate);

    // Create empty argument set.
    const int argc = 0;
    Local<Value> argv[1] = {};
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> construct = Local<Function>::New(isolate, constructor);
    // Create output and remainder object.
    Local<Object> outObject = construct->NewInstance(context, argc, argv).ToLocalChecked();
    Local<Object> remObject = construct->NewInstance(context, argc, argv).ToLocalChecked();
    // Wrap output and remainder DataClasses.
    out->Wrap(outObject);
    rem->Wrap(remObject);
    // Set properties of result object.
    result->Set(String::NewFromUtf8(isolate, "output"), outObject);
    result->Set(String::NewFromUtf8(isolate, "remainder"), remObject);

    args.GetReturnValue().Set(result);
  }

  void DataClass::SplitEvenly(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Get arguments: int n (number of sets).
    if (args[0]->IsUndefined() || !args[0]->IsNumber())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument n must be a number.")
      ));
      return;
    }
    int n = (int)args[0]->NumberValue();
	//I really love you <3
    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    if (n > cls->rows) n = cls->rows;

    // Create n sets of DataClasses.
    std::vector<DataClass*> sets = std::vector<DataClass*>(n);
    // Get number to place in each set.
    int evenCount = cls->rows / n;
    int rem = cls->rows - (evenCount * n);
    for (int i = 0; i < n - 1; i++) sets[i] = new DataClass(evenCount, cls->cols);
    sets[n - 1] = new DataClass(evenCount + rem, cls->cols);

    // Create a random sequence of indices.
    std::vector<int> sequence = GenerateSequence(cls->rows);

    SplitIntoSets(cls, sets, sequence);
    args.GetReturnValue().Set(CreateArrayFromSets(isolate, sets));
  }

  void DataClass::SplitByAmount(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Get arguments: int ...
    int n = args.Length();
    if (n == 0)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Cannot split into zero sets.")
      ));
      return;
    }

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    // Create vector of sets.
    std::vector<DataClass*> sets = std::vector<DataClass*>(n);
    // Create each DataClass.
    int total = 0;
    for (int i = 0; i < n; i++)
    {
      int rowCount = (int)args[i]->NumberValue();
      sets[i] = new DataClass(rowCount, cls->cols);
      total += rowCount;
    }

    if (total > cls->rows)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Attempt to split into more rows than available.")
      ));
      return;
    }

    // Split into sets.
    SplitIntoSets(cls, sets, GenerateSequence(cls->rows));
    args.GetReturnValue().Set(CreateArrayFromSets(isolate, sets));
  }

  void DataClass::SplitByPercentage(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Get arguments: int ...
    int n = args.Length();
    if (n == 0)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Requires at least one argument.")
      ));
      return;
    }

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    // Convert percentages to number of rows and create vector of sets.
    std::vector<DataClass*> sets = std::vector<DataClass*>(n);
    double totalRows = (double)cls->rows;
    double totalPercentage = 0.0;
    for (int i = 0; i < n; i++)
    {
      // Get percentage.
      double p = args[i]->NumberValue();
      totalPercentage += p;
      // Get row count for this set.
      int rowCount = (int)round(totalRows / 100.0 * p);
      sets[i] = new DataClass(rowCount, cls->cols);
    }

    if (totalPercentage > 100.0)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Cannot split into more than 100 percent.")
      ));
      return;
    }

    SplitIntoSets(cls, sets, GenerateSequence(cls->rows));
    args.GetReturnValue().Set(CreateArrayFromSets(isolate, sets));
  }

  void DataClass::GetExemplar(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Get arguments: int startColumn, int numClasses, int startAt
    if (args.Length() < 3)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Too few arguments.")
      ));
      return;
    }
    if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "All arguments must be numbers.")
      ));
      return;
    }

    int column = (int)args[0]->NumberValue();
    int numClasses = (int)args[1]->NumberValue();
    int startAt = (int)args[2]->NumberValue();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    int newCols = cls->cols + numClasses - 1;
    DataClass* out = new DataClass(cls->rows, newCols);

    for (int r = 0; r < cls->rows; r++)
    {
      std::vector<double> row = std::vector<double>(newCols);
      int cIndex = 0;
      for (int c = 0; c < cls->cols; c++)
      {
        double d = cls->data[r][c];
        if (c == column)
        {
          for (int j = 0; j < numClasses; j++)
          {
            row[cIndex] = 0;
            if (j == (int)d - startAt)
            {
              row[cIndex] = 1;
            }
            cIndex++;
          }
        }
        else
        {
          row[cIndex] = d;
          cIndex++;
        }
      }
      out->set_row(r, row);
    }

    // Return wrapped result.
    out->Wrap(args.This());

    args.GetReturnValue().Set(args.This());
  }

  void DataClass::MakeExemplar(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Get arguments: int startColumn, int numClasses, int startAt
    if (args.Length() < 3)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Too few arguments.")
      ));
      return;
    }
    if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "All arguments must be numbers.")
      ));
      return;
    }

    int column = (int)args[0]->NumberValue();
    int numClasses = (int)args[1]->NumberValue();
    int startAt = (int)args[2]->NumberValue();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    int newCols = cls->cols + numClasses - 1;
    //cls->resize_cols(newCols);
    cls->data.resize_cols(newCols);
    //DataClass* out = new DataClass(cls->rows, newCols);

    for (int r = 0; r < cls->rows; r++)
    {
      std::vector<double> row = std::vector<double>(newCols);
      int cIndex = 0;
      for (int c = 0; c < cls->cols; c++)
      {
        double d = cls->data[r][c];
        if (c == column)
        {
          for (int j = 0; j < numClasses; j++)
          {
            cls->data[r][cIndex] = 0;
            if (j == (int)d - startAt)
            {
              cls->data[r][cIndex] = 1;
            }
            cIndex++;
          }
        }
        else
        {
          cls->data[r][cIndex] = d;
          cIndex++;
        }
      }
    }

    cls->cols = newCols;
  }

  void DataClass::ReadFromFile(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    // Get arguments (path).
    if (args[0]->IsUndefined() || !args[0]->IsString())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 'path' undefined or of wrong type.")
      ));
      return;
    }

    std::string path(*String::Utf8Value(args[0]));

    cls->_ReadFromFile(path, isolate);
  }

  void DataClass::WriteToFile(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Unwrap DataClass.
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args.Holder());

    // Get arguments (path).
    if (args[0]->IsUndefined() || !args[0]->IsString())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument 'path' undefined or of wrong type.")
      ));
      return;
    }

    std::string path(*String::Utf8Value(args[0]));

    // Open file for writing and truncate the file if it already exists.
		std::ofstream file(path, std::ios::out | std::ios::trunc);

		// Generate output string.
		std::string output = "";
		for (int i = 0; i < cls->data.rows(); i++)
		{
			for (int j = 0; j < cls->data.cols(); j++)
			{
				if (j > 0) output += " ";
				output += tools::toString(cls->data[i][j], 2);
			}
			output += "\n";
		}

		// Write output to file.
		file.write(output.c_str(), output.length());

		// Close file.
		file.close();
  }

  void DataClass::_ReadFromFile(const std::string& path, Isolate* isolate)
  {
    // First, count lines.
		int counter = 0;
		std::string line;

		// Open file for reading.
		std::ifstream file(path, std::ios::in);
		// Check if file exists.
		if (!file)
		{
			// File does not exist.
      file.close();
      std::string msg = "File " + path + " does not exist.";
      if (isolate != NULL)
      {
        isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, msg.c_str())
        ));
      }
      else
      {
        throw "File " + path + " does not exist.";
      }
      return;
		}

		// Count the number of lines.
		while (std::getline(file, line))
		{
			if (tools::trim(line) != "") counter++;
		}

		// Seek back to start.
		file.clear();
		file.seekg(0, std::ios::beg);

		rows = counter;
		cols = -1;
		data = Matrix<double>(rows);

		counter = 0;
		while (std::getline(file, line))
		{
			if (tools::trim(line) != "")
			{
				std::vector<std::string> split = tools::split(line, delims);
				int count = 0;
				for (std::string s : split)
				{
					if (tools::trim(s) != "") count++;
				}
				if (cols == -1)
				{
					cols = count;
					data.resize_cols(count);
				}
				else if (cols != count)
				{
					// INCONSISTENT COLUMN COUNT.
					file.close();
					return;
				}
				count = 0;
				for (std::string s : split)
				{
					if (tools::trim(s) != "")
					{
						double d;
						try
						{
							d = std::stod(s);
						}
						catch (std::exception e)
						{
							// COULD NOT CONVERT STRING TO DOUBLE.
							file.close();
							return;
						}
						data[counter][count] = d;
						count++;
					}
				}

				counter++;
			}
		}

		// Close file.
		file.close();
  }

  std::vector<int> DataClass::GenerateSequence(int count)
  {
    // Generate a random sequence of count indices.
    std::vector<int> sequence(count);
    for (int i = 0; i < count; i++) sequence[i] = i;
    for (int i = 0; i < count; i++)
    {
      int r = random.nextInt(i, count);
      int tmp = sequence[r];
      sequence[r] = sequence[i];
      sequence[i] = tmp;
    }
    return sequence;
  }

  void DataClass::SplitIntoSets(DataClass* cls, std::vector<DataClass*>& sets, std::vector<int>& sequence)
  {
    int setIndex = 0;
    int currentSize = 0;
    int targetSize = sets[0]->rows;
    for (int i = 0; i < sequence.size(); i++)
    {
      if (currentSize >= targetSize)
      {
        setIndex++;
        currentSize = 0;
        targetSize = sets[setIndex]->rows;
      }
      sets[setIndex]->set_row(currentSize++, cls->data[sequence[i]]);
    }
  }

  Local<Array> DataClass::CreateArrayFromSets(Isolate* isolate, std::vector<DataClass*>& sets)
  {
    // Create resulting array.
    Local<Array> result = Array::New(isolate, sets.size());
    // Create empty argument set.
    const int argc = 0;
    Local<Value> argv[1] = {};
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> construct = Local<Function>::New(isolate, constructor);
    // Wrap each set, appending it to the array.
    for (int i = 0; i < sets.size(); i++)
    {
      Local<Object> wrapper = construct->NewInstance(context, argc, argv).ToLocalChecked();
      sets[i]->Wrap(wrapper);
      result->Set(i, wrapper);
    }
    // Return array.
    return result;
  }

  bool DataClass::set_row(int index, std::vector<double>& row)
  {
    if (index < 0 || index >= rows) return false;
    data[index] = row;
    return true;
  }

  void DataClass::resize_rows(int rows)
  {
    this->rows = rows;
    this->data.resize_rows(rows);
  }

  void DataClass::resize_cols(int cols)
  {
    this->cols = cols;
    this->data.resize_cols(cols);
  }
}
