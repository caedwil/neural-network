#include <node.h>
#include "data-class.hh"
#include "neural-network.hh"

namespace ANN
{
	void init(v8::Local<v8::Object> exports)
	{
		DataClass::Init(exports);
		NeuralNetwork::Init(exports);
	}

	NODE_MODULE(ann, init)
}
