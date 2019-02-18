#include "neural-network.hh"
#include "data-class.hh"
#include "tools.hh"
#include <fstream>
#include <math.h>
#include <string>

#include <iostream>

namespace ANN
{
  using v8::Array;
  using v8::Boolean;
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
  using v8::Value;

  Persistent<Function> NeuralNetwork::constructor;
  Random NeuralNetwork::random = Random();

  void NeuralNetwork::Init(Local<Object> exports)
  {
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template.
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
    tmpl->SetClassName(String::NewFromUtf8(isolate, "NeuralNetwork"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add methods to prototype.
    NODE_SET_PROTOTYPE_METHOD(tmpl, "toString", ToString);
    //NODE_SET_PROTOTYPE_METHOD(tmpl, "initialiseWeights", InitialiseWeights);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "train", Train);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "confusion", ConfusionToString);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "accuracy", Accuracy);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "momentumAndDecay", MomentumAndDecay);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "save", Save);

    NODE_SET_PROTOTYPE_METHOD(tmpl, "trainingAccuracy", TrainingAccuracy);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "testingAccuracy", TestingAccuracy);

    // Export new item.
    constructor.Reset(isolate, tmpl->GetFunction());
    exports->Set(
      String::NewFromUtf8(isolate, "NeuralNetwork"),
      tmpl->GetFunction()
    );
  }

  void NeuralNetwork::New(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    if (args.IsConstructCall())
    {
      // NeuralNetwork invoked as constructor.
      // Get arguments: int numInput, int numHidden, int numOutput.
      if (args.Length() < 3)
      {
        isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Too few arguments.")
        ));
        return;
      }
      std::vector<int> num = std::vector<int>(3);
      for (int i = 0; i < 3; i++)
      {
        if (!args[i]->IsNumber())
        {
          isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(
              isolate,
              std::string("Argument " + std::to_string(i) + " must be a number.").c_str())
          ));
          return;
        }
        else
        {
          num[i] = (int)args[i]->NumberValue();
        }
      }

      NeuralNetwork* nn = new NeuralNetwork(num[0], num[1], num[2]);
      nn->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    }
    else
    {
      // NeuralNetwork invoked as plain function.
      // e.g. NeuralNetwork(...).
      const int argc = 0;
      Local<Value> argv[1] = {};
      Local<Context> context = isolate->GetCurrentContext();
      Local<Function> construct = Local<Function>::New(isolate, constructor);
      Local<Object> result = construct->NewInstance(context, argc, argv).ToLocalChecked();
      args.GetReturnValue().Set(result);
    }
  }

  NeuralNetwork::NeuralNetwork(int numInput, int numHidden, int numOutput)
  {
    this->numInput = numInput;
  	this->numHidden = numHidden;
  	this->numOutput = numOutput;

  	this->inputs = std::vector<double>(numInput);

  	this->ihWeights = Matrix<double>(numInput, numHidden);
  	this->hBiases = std::vector<double>(numHidden);
  	this->hOutputs = std::vector<double>(numHidden);

  	this->hoWeights = Matrix<double>(numHidden, numOutput);
  	this->oBiases = std::vector<double>(numOutput);

  	this->outputs = std::vector<double>(numOutput);

  	// Back-propagation related arrays below.
  	this->hGrads = std::vector<double>(numHidden);
  	this->oGrads = std::vector<double>(numOutput);

  	this->ihPrevWeightsDelta = Matrix<double>(numInput, numHidden);
  	this->hPrevBiasesDelta = std::vector<double>(numHidden);
  	this->hoPrevWeightsDelta = Matrix<double>(numHidden, numOutput);
  	this->oPrevBiasesDelta = std::vector<double>(numOutput);

    this->InitialiseWeights();
  }

  void NeuralNetwork::ToString(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Unwrap NeuralNetwork.
    NeuralNetwork* nn = ObjectWrap::Unwrap<NeuralNetwork>(args.Holder());

    // Create output string.
    std::string s = "";
  	s += "------------------------------------------\n";

  	s += "numInput = " + std::to_string(nn->numInput) + ", ";
  	s += "numHidden = " + std::to_string(nn->numHidden) + ", ";
  	s += "numOutput = " + std::to_string(nn->numOutput) + "\n\n";

  	s += "inputs: \n";
  	for (int i = 0; i < nn->inputs.size(); i++)
  	{
  		s += tools::toString(nn->inputs[i], 2) + " ";
  	}
  	s += "\n\n";

  	s += "ihWeights: \n";
  	for (int i = 0; i < nn->ihWeights.rows(); i++)
  	{
  		for (int j = 0; j < nn->ihWeights.cols(); j++)
  		{
  			s += tools::toString(nn->ihWeights[i][j], 4) + " ";
  		}
  		s += "\n";
  	}
  	s += "\n";

  	s += "hBiases: \n";
  	for (int i = 0; i < nn->hBiases.size(); i++)
  	{
  		s += tools::toString(nn->hBiases[i], 4) + " ";
  	}
  	s += "\n\n";

  	s += "hOutputs: \n";
  	for (int i = 0; i < nn->hOutputs.size(); i++)
  	{
  		s += tools::toString(nn->hOutputs[i], 4) + " ";
  	}
  	s += "\n\n";

  	s += "hoWeights: \n";
  	for (int i = 0; i < nn->hoWeights.rows(); i++)
  	{
  		for (int j = 0; j < nn->hoWeights.cols(); j++)
  		{
  			s += tools::toString(nn->hoWeights[i][j], 4) + " ";
  		}
  		s += "\n";
  	}
  	s += "\n";

  	s += "oBiases: \n";
  	for (int i = 0; i < nn->oBiases.size(); i++)
  	{
  		s += tools::toString(nn->oBiases[i], 4) + " ";
  	}
  	s += "\n\n";

  	s += "hGrads: \n";
  	for (int i = 0; i < nn->hGrads.size(); i++)
  	{
  		s += tools::toString(nn->hGrads[i], 4) + " ";
  	}
  	s += "\n\n";

  	s += "oGrads: \n";
  	for (int i = 0; i < nn->oGrads.size(); i++)
  	{
  		s += tools::toString(nn->oGrads[i], 4) + " ";
  	}
  	s += "\n\n";

  	s += "ihPrevWeightsDelta: \n";
  	for (int i = 0; i < nn->ihPrevWeightsDelta.rows(); i++)
  	{
  		for (int j = 0; j < nn->ihPrevWeightsDelta.cols(); j++)
  		{
  			s += tools::toString(nn->ihPrevWeightsDelta[i][j], 4) + " ";
  		}
  		s += "\n";
  	}
  	s += "\n";

  	s += "hPrevBiasesDelta: \n";
  	for (int i = 0; i < nn->hPrevBiasesDelta.size(); i++)
  	{
  		s += tools::toString(nn->hPrevBiasesDelta[i], 4) + " ";
  	}
  	s += "\n\n";

  	s += "hoPrevWeightsDelta: \n";
  	for (int i = 0; i < nn->hoPrevWeightsDelta.rows(); i++)
  	{
  		for (int j = 0; j < nn->hoPrevWeightsDelta.cols(); j++)
  		{
  			s += tools::toString(nn->hoPrevWeightsDelta[i][j], 4) + " ";
  		}
  		s += "\n";
  	}
  	s += "\n";

  	s += "oPrevBiasesDelta: \n";
  	for (int i = 0; i < nn->oPrevBiasesDelta.size(); i++)
  	{
  		s += tools::toString(nn->oPrevBiasesDelta[i], 4) + " ";
  	}
  	s += "\n\n";

  	s += "outputs: \n";
  	for (int i = 0; i < nn->outputs.size(); i++)
  	{
  		s += tools::toString(nn->outputs[i], 2) + " ";
  	}
  	s += "\n";

  	s += "------------------------------------------\n";

    // Convert and return output string.
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, s.c_str()));
  }

  void NeuralNetwork::InitialiseWeights()
  {
    // Initialise weights and biases to small random values.
    std::vector<double> initialWeights = std::vector<double>(NumWeights());
  	double lower = -0.01;
  	double upper = 0.01;
  	for (int i = 0; i < initialWeights.size(); i++)
  	{
  		initialWeights[i] = (upper - lower) * random.nextDouble() + lower;
  	}
    // Set weights.
    SetWeights(initialWeights);
  }

  void NeuralNetwork::Train(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Get arguments: training data, testing dating, maximum epochs,
    // learning rate, and log file path.
    if (args.Length() < 5)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Too few arguments.")
      ));
      return;
    }
    if (!args[2]->IsNumber() || !args[3]->IsNumber() || !args[4]->IsString())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument of wrong type passed.")
      ));
      return;
    }

    DataClass* train = ObjectWrap::Unwrap<DataClass>(args[0]->ToObject());
    DataClass* test = ObjectWrap::Unwrap<DataClass>(args[1]->ToObject());
    int maxEpochs = (int)args[2]->NumberValue();
    double learnRate = args[3]->NumberValue();
    std::string logFileName(*String::Utf8Value(args[4]));

    // Unwrap NeuralNetwork.
    NeuralNetwork* nn = ObjectWrap::Unwrap<NeuralNetwork>(args.Holder());

    // Initialise accuracy vectors.
    nn->trainingAccuracy = std::vector<double>(maxEpochs);
    nn->testingAccuracy = std::vector<double>(maxEpochs);

    // Train a back-propagation style NN classifier using learning rate
  	// and momentum. Weight decay reduces the magnitude of a weight
  	// value over time unless that value is constantly increased.
  	int epoch = 0;
  	std::vector<double> xValues = std::vector<double>(nn->numInput);
  	std::vector<double> tValues = std::vector<double>(nn->numOutput);

  	std::vector<int> sequence = std::vector<int>(train->data.rows());
  	for (int i = 0; i < sequence.size(); i++) sequence[i] = i;

  	// Train the NN while writing results to the log file.
  	// Open and truncate output log file for writing.
  	std::ofstream log;
  	log.open(logFileName, std::ios::out | std::ios::trunc);

  	while (epoch < maxEpochs)
  	{
  		// Visit each training data in random order.
  		Shuffle(sequence);
  		for (int i = 0; i < train->data.rows(); i++)
  		{
  			int idx = sequence[i];
  			xValues.assign(train->data[idx].begin(), train->data[idx].begin() + nn->numInput);
  			tValues.assign(train->data[idx].begin() + nn->numInput, train->data[idx].end());
  			// Copy xValues in, compute outputs (store them internally).
  			nn->ComputeOutputs(xValues);
  			// Find better weights.
  			nn->UpdateWeights(tValues, learnRate);
  		}

  		// To convert to percent: x * 100.
  		double trainAccuracy = nn->AccuracyHelper(train->data) * 100;
  		double testAccuracy = nn->AccuracyHelper(test->data) * 100;
  		double trainMSE = nn->MeanSquaredError(train->data);
  		double testMSE = nn->MeanSquaredError(test->data);

      // Push training and testing accuracy.
      nn->trainingAccuracy[epoch] = trainAccuracy;
      nn->testingAccuracy[epoch] = testAccuracy;

  		// Put together output string.
  		std::string output = "";
  		output += std::to_string(epoch + 1) + " ";
  		output += std::to_string(trainMSE) + " " + std::to_string(testMSE) + " ";
  		output += tools::toString(trainAccuracy, 2) + "% ";
  		output += tools::toString(testAccuracy, 2) + "%\n";

  		// Write output to log file.
  		log << output;

  		// Increment counter epoch.
  		epoch++;
  	}

  	// Close output log file.
  	log.close();
  }

  void NeuralNetwork::ConfusionToString(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Unwrap NeuralNetwork.
    NeuralNetwork* nn = ObjectWrap::Unwrap<NeuralNetwork>(args.Holder());

    std::string output = "";

    std::string divider = "";
    for (int i = 0; i < nn->numOutput * 7; i++)
    {
      divider += "-";
    }

    for (int y = 0; y < nn->numOutput; y++)
    {
      for (int x = 0; x < nn->numOutput; x++)
      {
        std::string temp = std::to_string(nn->confusionMatrix[x][y]);
        temp.insert(temp.begin(), 4 - temp.length(), ' ');
        //temp.append(8 - temp.length(), 'X');
        //temp = tools::pad(temp, 4);
        output += temp + " ";
        if (x != nn->numOutput - 1) output += "| ";
      }
      output += "\n";
      if (y != nn->numOutput - 1) output += divider + "\n";
    }

    // Return confusion matrix as a string.
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, output.c_str()));
  }

  void NeuralNetwork::Accuracy(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Check if the passed argument is an array.
    /*
    if (!args[0]->IsArray())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Argument must be an array.")
      ));
      return;
    }
    // Get argument as array.
    Array* input = Array::Cast(*args[0]);
    // Convert input array to matrix.
    Matrix<double> testData = Matrix<double>(input->Length());
    int cols = 0;
    for (int i = 0; i < input->Length(); i++)
    {
      Local<Value> val = input->Get(i);
      if (val->IsArray())
      {
        Array* row = Array::Cast(*val);
        if (cols <= 0)
        {
          cols = row->Length();
          testData.resize_cols(cols);
        }
        for (int j = 0; j < row->Length(); j++)
        {
          testData[i][j] = row->Get(j)->NumberValue();
        }
      }
    }
    */
    DataClass* cls = ObjectWrap::Unwrap<DataClass>(args[0]->ToObject());

    // Unwrap NeuralNetwork.
    NeuralNetwork* nn = ObjectWrap::Unwrap<NeuralNetwork>(args.Holder());
    args.GetReturnValue().Set(nn->AccuracyHelper(cls->data));
  }

  double NeuralNetwork::AccuracyHelper(Matrix<double>& testData)
  {
    // Percentage correct using winner takes all.
  	int numCorrect = 0;
  	int numWrong = 0;
  	// Inputs.
  	std::vector<double> xValues = std::vector<double>(numInput);
  	// Targets.
  	std::vector<double> tValues = std::vector<double>(numOutput);
  	// Computed y.
  	std::vector<double> yValues;

    confusionMatrix = Matrix<int>(numOutput, numInput);

  	for (int i = 0; i < testData.rows(); i++)
  	{
  		xValues.assign(testData[i].begin(), testData[i].begin() + numInput);
  		tValues.assign(testData[i].begin() + numInput, testData[i].end());
  		yValues = ComputeOutputs(xValues);
  		// Which cell in yValues has the largest value?
  		int maxIndexOut = MaxIndex(yValues);
      // Which cell is tValues had largest value?
      int maxIndexExpected = MaxIndex(tValues);

  		//if (tValues[max] == 1.0) ++numCorrect;
      if (maxIndexOut == maxIndexExpected) numCorrect++;
  		else ++numWrong;

      confusionMatrix[maxIndexExpected][maxIndexOut] += 1;
  	}

  	if (numCorrect == 0 && numWrong == 0) return 0;
  	else return (numCorrect * 1.0) / (numCorrect + numWrong);
  }

  void NeuralNetwork::MomentumAndDecay(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();

    // Get arguments: double momentum, double weightDecay
    if (args.Length() < 2)
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Too few arguments.")
      ));
      return;
    }
    for (int i = 0; i < 2; i++)
    {
      if (!args[i]->IsNumber())
      {
        std::string msg = "Argument " + std::to_string(i) + " must be a number.";
        isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, msg.c_str())
        ));
        return;
      }
    }

    // Unwrap NeuralNetwork.
    NeuralNetwork* nn = ObjectWrap::Unwrap<NeuralNetwork>(args.Holder());
    nn->momentum = args[0]->NumberValue();
    nn->weightDecay = args[1]->NumberValue();
  }

  void NeuralNetwork::TrainingAccuracy(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    NeuralNetwork* nn = ObjectWrap::Unwrap<NeuralNetwork>(args.Holder());
    args.GetReturnValue().Set(DoubleVectorToJSArray(isolate, nn->trainingAccuracy));
  }

  void NeuralNetwork::TestingAccuracy(const FunctionCallbackInfo<Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    NeuralNetwork* nn = ObjectWrap::Unwrap<NeuralNetwork>(args.Holder());
    args.GetReturnValue().Set(DoubleVectorToJSArray(isolate, nn->testingAccuracy));
  }

  void NeuralNetwork::Save(const FunctionCallbackInfo<Value>& args)
  {
    // Get isolate.
    Isolate* isolate = args.GetIsolate();
    // Unwrap NeuralNetwork.
    NeuralNetwork* nn = ObjectWrap::Unwrap<NeuralNetwork>(args.Holder());
    // Get arguments:
    //   path       the path to the file to write information to
    //   verbose    whether or not to write a verbose set of information
    //   precision  the number of decimal places to show
    if (args[0]->IsUndefined() || !args[0]->IsString())
    {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "First argument must be a defined string.")
      ));
      return;
    }
    std::string path(*String::Utf8Value(args[0]));
    bool verbose = false;
    int precision = 4;
    int padding = 0;
    if (!args[1]->IsUndefined() && args[1]->IsBoolean())
    {
      verbose = args[1]->BooleanValue();
    }
    if (!args[2]->IsUndefined() && args[2]->IsNumber())
    {
      precision = (int)args[2]->NumberValue();
    }
    if (verbose) padding = precision + 3;

    // Open file for writing, truncating it if it already exists.
    std::ofstream file(path, std::fstream::out | std::fstream::trunc);
    std::string line = "";

    // Write setup information.
    if (verbose)
    {
      file << "Input Nodes : " << nn->numInput << "\n";
      file << "Hidden Nodes: " << nn->numHidden << "\n";
      file << "Output Nodes: " << nn->numOutput << "\n";
    }
    else
    {
      file << nn->numInput << " " << nn->numHidden << " " << nn->numOutput << "\n";
    }

    // Write hidden layer.
    if (verbose) file << "Input/Hidden Weights:\n";
    file << nn->ihWeights.toString(precision, verbose, padding);
    if (verbose) file << "Hidden Layer Biases:\n";
    file << VectorToString(nn->hBiases, precision, verbose, padding) << "\n";

    // Write output layer.
    if (verbose) file << "Hidden/Output Weights:\n";
    file << nn->hoWeights.toString(precision, verbose, padding);
    if (verbose) file << "Output Layer Biases:\n";
    file << VectorToString(nn->oBiases, precision, verbose, padding) << "\n";

    // Close the file.
    file.close();
  }

  std::string NeuralNetwork::VectorToString(const std::vector<double>& v, int precision, bool verbose, int padding)
  {
    std::string output = "";
    for (int i = 0; i < v.size(); i++)
    {
      if (i > 0) output += " ";
      std::string item = tools::toString(v[i], precision);
      if (padding > item.length()) item.insert(item.begin(), padding - item.length(), ' ');
      output += item;
    }
    if (verbose) output = "[ " + output + " ]";
    return output;
  }

  std::vector<double> NeuralNetwork::GetWeights()
  {
    // Returns the current set of weights, presumably after training.
  	std::vector<double> result = std::vector<double>(NumWeights());
  	int k = 0;
  	for (int i = 0; i < ihWeights.rows(); i++)
  	{
  		for (int j = 0; j < ihWeights.cols(); j++)
  		{
  			result[k++] = ihWeights[i][j];
  		}
  	}
  	for (int i = 0; i < hBiases.size(); i++)
  	{
  		result[k++] = hBiases[i];
  	}
  	for (int i = 0; i < hoWeights.rows(); i++)
  	{
  		for (int j = 0; j < hoWeights.cols(); j++)
  		{
  			result[k++] = hoWeights[i][j];
  		}
  	}
  	for (int i = 0; i < oBiases.size(); i++)
  	{
  		result[k++] = oBiases[i];
  	}
  	return result;
  }

  void NeuralNetwork::SetWeights(std::vector<double>& weights)
  {
    // Copy weights and biases in weights vector to i-h weights,
  	// i-h biases, h-o weights, and h-o biases.
  	if (weights.size() != NumWeights())
  	{
  		// ADD IN THROW EXCEPTION.
  		throw "";
  		return;
  	}

  	// Points into weights param.
  	int k = 0;

  	for (int i = 0; i < numInput; i++)
  	{
  		for (int j = 0; j < numHidden; j++)
  		{
  			ihWeights[i][j] = weights[k++];
  		}
  	}
  	for (int i = 0; i < numHidden; i++)
  	{
  		hBiases[i] = weights[k++];
  	}
  	for (int i = 0; i < numHidden; i++)
  	{
  		for (int j = 0; j < numOutput; j++)
  		{
  			hoWeights[i][j] = weights[k++];
  		}
  	}
  	for (int i = 0; i < numOutput; i++)
  	{
  		oBiases[i] = weights[k++];
  	}
  }

  void NeuralNetwork::UpdateWeights(std::vector<double>& tValues, double learnRate)
  {
    // Update the weights and biases using back-propagation, with target
  	// values, eta (learning values), and alpha (momentum).
  	// Assumes the setWeights and computeOutputs have been called and so
  	// all the internal arrays and matrices have values other than 0.0.
  	if (tValues.size() != numOutput)
  	{
  		// THROW EXCEPTION.
  		throw "";
  		return;
  	}

  	// 1. Compute output gradients.
  	for (int i = 0; i < oGrads.size(); i++)
  	{
  		// Derivative of softmax = (1 - y) * y (same as log-sigmoid).
  		double derivative = (1 - outputs[i]) * outputs[i];
  		// Mean squared error version, includes (1-y)(y) derivative.
  		oGrads[i] = derivative * (tValues[i] - outputs[i]);
  	}

  	// 2. Compute hidden gradients.
  	for (int i = 0; i < hGrads.size(); i++)
  	{
  		// Derivative of tanh = (1 - y) * (1 + y).
  		double derivative = (1 - hOutputs[i]) * (1 + hOutputs[i]);
  		double sum = 0.0;
  		for (int j = 0; j < numOutput; j++)
  		{
  			double x = oGrads[j] * hoWeights[i][j];
  			sum += x;
  		}
  		hGrads[i] = derivative * sum;
  	}

  	// 3a. Update hidden weights (gradients must be computed right-to-left
  	// but weights can be updated in any order).
  	for (int i = 0; i < ihWeights.rows(); i++)
  	{
  		for (int j = 0; j < ihWeights.cols(); j++)
  		{
  			// Compute the new delta.
  			double delta = learnRate * hGrads[j] * inputs[i];
  			// Update, note: we use '+' instead of '-'. This can be very
  			// tricky. Now, add momentum using previous delta. On first
  			// pass old value will be 0.0 but that is okay.
  			ihWeights[i][j] += delta;
  			if (momentum > 0) ihWeights[i][j] += momentum * ihPrevWeightsDelta[i][j];
  			// Weight decay.
  			if (weightDecay > 0) ihWeights[i][j] -= (weightDecay * ihWeights[i][j]);
  			// Don't forget to save the delta for momentum.
  			ihPrevWeightsDelta[i][j] = delta;
  		}
  	}

  	// 3b. Update hidden biases.
  	for (int i = 0; i < hBiases.size(); i++)
  	{
  		double delta = learnRate * hGrads[i] * 1.0;
  		hBiases[i] += delta;
  		if (momentum > 0) hBiases[i] += momentum * hPrevBiasesDelta[i];
  		if (weightDecay > 0) hBiases[i] -= weightDecay * hBiases[i];
  		// Don't forget to save the delta.
  		hPrevBiasesDelta[i] = delta;
  	}

  	// 4a. Update hidden-output weights.
  	for (int i = 0; i < hoWeights.rows(); i++)
  	{
  		for (int j = 0; j < hoWeights.cols(); j++)
  		{
  			double delta = learnRate * oGrads[j] * hOutputs[i];
  			hoWeights[i][j] += delta;
  			if (momentum > 0) hoWeights[i][j] += momentum * hoPrevWeightsDelta[i][j];
  			if (weightDecay > 0) hoWeights[i][j] -= weightDecay * hoWeights[i][j];
  			// Save delta.
  			hoPrevWeightsDelta[i][j] = delta;
  		}
  	}

  	// 4b. Update output biases.
  	for (int i = 0; i < oBiases.size(); i++)
  	{
  		double delta = learnRate * oGrads[i] * 1.0;
  		oBiases[i] += delta;
  		if (momentum > 0) oBiases[i] += momentum * oPrevBiasesDelta[i];
  		if (weightDecay > 0) oBiases[i] -= weightDecay * oBiases[i];
  		// Save delta.
  		oPrevBiasesDelta[i] = delta;
  	}
  }

  std::vector<double> NeuralNetwork::ComputeOutputs(std::vector<double>& xValues)
  {
    if (xValues.size() != numInput)
  	{
  		// THROW EXCEPTION.
  		throw "";
  		return std::vector<double>();
  	}

  	// Hidden nodes sums scratch array.
  	std::vector<double> hSums = std::vector<double>(numHidden);
  	// Output nodes sums.
  	std::vector<double> oSums = std::vector<double>(numOutput);

  	// Copy x-values to inputs.
  	/*
  	for (int i = 0; i < xValues.size(); i++)
  	{
  		inputs[i] = xValues[i];
  	}
  	*/
  	inputs.assign(xValues.begin(), xValues.end());

  	// Compute i-h sum of weights * inputs.
  	for (int j = 0; j < numHidden; j++)
  	{
  		for (int i = 0; i < numInput; i++)
  		{
  			// Note: +=
  			hSums[j] += inputs[i] * ihWeights[i][j];
  		}
  	}

  	// Add biases to input-to-hidden sums.
  	for (int i = 0; i < numHidden; i++)
  	{
  		hSums[i] += hBiases[i];
  	}

  	// Apply activation.
  	for (int i = 0; i < numHidden; i++)
  	{
  		hOutputs[i] = HyperTanFunction(hSums[i]);
  	}

  	// Compute h-o sum of weights * hOutputs.
  	for (int j = 0; j < numOutput; j++)
  	{
  		for (int i = 0; i < numHidden; i++)
  		{
  			oSums[j] += hOutputs[i] * hoWeights[i][j];
  		}
  	}

  	// Add biases to input-to-hidden sums.
  	for (int i = 0; i < numOutput; i++)
  	{
  		oSums[i] += oBiases[i];
  	}

  	// Softmax activation does all outputs at once for efficiency.
  	std::vector<double> softOut = Softmax(oSums);
  	outputs = std::vector<double>(softOut);

  	// Could define a getOutputs method instead.
  	std::vector<double> result = std::vector<double>(outputs);
  	return result;
  }

  double NeuralNetwork::MeanSquaredError(Matrix<double>& trainData)
  {
    // Average squared error per training tuple.
  	double sumSquaredError = 0.0;
  	// First numInput values in trainData.
  	std::vector<double> xValues = std::vector<double>(numInput);
  	// Last numOutput values.
  	std::vector<double> tValues = std::vector<double>(numOutput);

  	// Walk through each training case.
  	// Looks like: (6.9 3.2 5.7 2.3) (0 0 1).
  	for (int i = 0; i < trainData.rows(); i++)
  	{
  		xValues.assign(trainData[i].begin(), trainData[i].begin() + numInput);
  		tValues.assign(trainData[i].begin() + numInput, trainData[i].end());
  		// Compute output using current weights.
  		std::vector<double> yValues = ComputeOutputs(xValues);
  		for (int j = 0; j < numOutput; j++)
  		{
  			double err = tValues[j] - yValues[j];
  			sumSquaredError += err * err;
  		}
  	}

  	return sumSquaredError / trainData.rows();
  }

  double NeuralNetwork::HyperTanFunction(double x)
  {
    if (x < -20.0) return -1.0;
    else if (x > 20.0) return 1.0;
    else return tanh(x);
  }

  std::vector<double> NeuralNetwork::Softmax(std::vector<double>& oSums)
  {
    // Determine max output sum.
		// Does all output nodes at once so scale doesn't have to be
		// re-computed each time.
		double max = oSums[0];
		for (int i = 0; i < oSums.size(); i++)
		{
			if (oSums[i] > max) max = oSums[i];
		}

		// Determine scaling factor -- sum of exp(each val - max).
		double scale = 0.0;
		for (int i = 0; i < oSums.size(); i++)
		{
			scale += exp(oSums[i] - max);
		}

		std::vector<double> result = std::vector<double>(oSums.size());
		for (int i = 0; i < oSums.size(); i++)
		{
			result[i] = exp(oSums[i] - max) / scale;
		}

		// Now scaled so that xi sum to 1.0.
		return result;
  }

  void NeuralNetwork::Shuffle(std::vector<int>& sequence)
  {
    for (int i = 0; i < sequence.size(); i++)
    {
      int r = random.nextInt(i, sequence.size());
      int tmp = sequence[r];
      sequence[r] = sequence[i];
      sequence[i] = tmp;
    }
  }

  int NeuralNetwork::MaxIndex(std::vector<double>& v)
  {
    int index = 0;
    double largestValue = v[0];
    for (int i = 0; i < v.size(); i++)
    {
      if (v[i] > largestValue)
      {
        index = i;
        largestValue = v[i];
      }
    }
    return index;
  }

  int NeuralNetwork::NumWeights()
  {
    return (numInput * numHidden) + (numHidden * numOutput) + numHidden + numOutput;
  }

  Local<Array> NeuralNetwork::DoubleVectorToJSArray(Isolate* isolate, std::vector<double>& v)
  {
    // Create output array.
    Local<Array> output = Array::New(isolate, v.size());
    for (int i = 0; i < v.size(); i++)
    {
      output->Set(i, Number::New(isolate, v[i]));
    }
    return output;
  }
}
