#ifndef NEURAL_NETWORK_HH
#define NEURAL_NETWORK_HH

#include "matrix.hh"
#include "random.hh"
#include <node.h>
#include <node_object_wrap.h>
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

  class NeuralNetwork : public node::ObjectWrap
  {
  public:
    static void Init(Local<Object> exports);
  private:
    static Persistent<Function> constructor;
    static void New(const FunctionCallbackInfo<Value>& args);

    // Used for generating random numbers.
    static Random random;

    double momentum = 0.0;
    double weightDecay = 0.0;

    // Number of input, hidden, and output nodes.
    int numInput;
    int numHidden;
    int numOutput;

    // Vector of inputs.
    std::vector<double> inputs;

    // Input-hidden weights.
    Matrix<double> ihWeights;
    // Hidden biases.
    std::vector<double> hBiases;
    // Hidden output.
    std::vector<double> hOutputs;

    // Hidden-output weights.
    Matrix<double> hoWeights;
    // Output biases.
    std::vector<double> oBiases;

    // Vector of outputs.
    std::vector<double> outputs;

    // Back-propagation specific array.
    // These could be local to function updateWeights().
    // Output and hidden gradients for back-propagation.
    std::vector<double> oGrads;
    std::vector<double> hGrads;

    // Back-propagation momentum specific arrays.
    // These could be local to function train().
    // For momentum with back-propagation.
    Matrix<double> ihPrevWeightsDelta;
    std::vector<double> hPrevBiasesDelta;
    Matrix<double> hoPrevWeightsDelta;
    std::vector<double> oPrevBiasesDelta;

    // Holds training accuracy from the last training run.
    std::vector<double> trainingAccuracy;
    // Holds the testing accuracy from the last training run.
    std::vector<double> testingAccuracy;

    // Used as a temporary holder.
    Matrix<int> confusionMatrix;

    NeuralNetwork(int numInput, int numHidden, int numOutput);

    // :: PUBLICLY AVAILABLE FUNCTIONS :: //
    // Returns a string representation of the Neural Network.
    static void ToString(const FunctionCallbackInfo<Value>& args);
    //static void InitialiseWeights(const FunctionCallbackInfo<Value>& args);
    static void Train(const FunctionCallbackInfo<Value>& args);
    static void ConfusionToString(const FunctionCallbackInfo<Value>& args);
    static void Accuracy(const FunctionCallbackInfo<Value>& args);
    static void MomentumAndDecay(const FunctionCallbackInfo<Value>& args);

    // Returns a JavaScript array containing the training accuracy from
    // the last training run.
    static void TrainingAccuracy(const FunctionCallbackInfo<Value>& args);
    // Returns a JavaScript array containing the testing accuracy from
    // the last training run.
    static void TestingAccuracy(const FunctionCallbackInfo<Value>& args);

    // Saves a basic set of information about the Neural Network to the
    // specified file.
    // If a second argument is passed as the boolean true, a much more
    // verbose set of information will be written.
    static void Save(const FunctionCallbackInfo<Value>& args);

    // :: PRIVATE FUNCTIONS :: //
    void InitialiseWeights();
    std::vector<double> GetWeights();
    void SetWeights(std::vector<double>& weights);
    void UpdateWeights(std::vector<double>& tValues, double learnRate);

    std::vector<double> ComputeOutputs(std::vector<double>& xValues);

    // Private accuracy function.
    double AccuracyHelper(Matrix<double>& testData);
    double MeanSquaredError(Matrix<double>& trainData);

    static double HyperTanFunction(double x);
    static std::vector<double> Softmax(std::vector<double>& oSums);
    static void Shuffle(std::vector<int>& sequence);
    static int MaxIndex(std::vector<double>& v);

    // Helper functions.
    int NumWeights();
    static Local<Array> DoubleVectorToJSArray(Isolate* isolate, std::vector<double>& v);
    static std::string VectorToString(const std::vector<double>& v, int precision = 4, bool verbose = false, int padding = 0);
  };
}

#endif
