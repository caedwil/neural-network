var get = require('./../modules/get');
var {dialog} = require('electron').remote;
var moment = require('moment');
//var {DataClass, NeuralNetwork} = require('./../modules/neural-network');
var {DataClass, NeuralNetwork} = require('./../addons/neural-network/build/Release/neural-network');
var Chart = require('chart.js');

var ANN = function() {
  var data, nn;
  var accuracy;
  this.run = null;
};

var networks = {
  iris: new ANN(),
  abalone: new ANN(),

  displayed: 'iris'
};

networks.iris.run = function() {
  // log file prefix.
  var logPrefix = 'logs/iris/' + moment().format('YYYY-MM-DDTHH-mm-ss') + '-';
  // Create data class.
  this.data = new DataClass('data/iris-original.dat');
  // Normalise data.
  this.data.normalise(0, 3);
  // Form exemplats.
  this.data.makeExemplar(4, 3, 1);
  // Split data into sets: training, testing, validation.
  var sets = this.data.splitEvenly(3);
  // Create neural network.
  this.nn = new NeuralNetwork(4, 4, 3);
  // The number of epochs to run.
  var epochs = 100;
  // The learning constant.
  var eta = 0.1;
  // Train the network.
  this.nn.train(sets[0], sets[1], epochs, eta, logPrefix + 'training.log');
  // Preserve the final weights of the network.
  this.nn.save(logPrefix + 'weights.log');
  this.nn.save(logPrefix + 'weights-verbose.log', true);
  // Get accuracy of each set.
  this.accuracy = {
    training: this.nn.accuracy(sets[0]) * 100,
    testing: this.nn.accuracy(sets[1]) * 100,
    validation: this.nn.accuracy(sets[2]) * 100
  };

  console.log(this.accuracy);
};

window.on('load', () => {
  get('#btn-run-all').on('click', () => {
    for (var attr in networks) {
      if (networks[attr].run !== null) networks[attr].run();
    }
  });

  get('#btn-iris').on('click', () => {
    if (networks['displayed'] !== 'iris') {
      // Display Iris data.
      
    }
  });

  /*
  get('#btn-run').on('click', () => {
    var file = '../data/iris-original.dat';
    // Create DataClass using the file.
    //var data = new DataClass(file);
    var data = new DataClass('data/iris-original.dat')
    // Normalise.
    data.normalise(0, 3);
    // From exemplars.
    data.makeExemplar(4, 3, 1);
    // Split data into sets: training, testing, validation.
    var sets = data.splitByAmount(50, 50, 50);
    // Create NeuralNetwork.
    var nn = new NeuralNetwork(4, 4, 3);
    nn.initialiseWeights();
    var epochs = 100;
    nn.train(sets[0], sets[1], epochs, 0.1, 'logs/neural-network.log');
    // Display accuracy of neural network.
    var output = 'Accuracy\n';
    output += 'Training: ' + (nn.accuracy(sets[0]) * 100) + '\n';
    output += nn.confusionToString() + '\n';
    output += 'Testing: ' + (nn.accuracy(sets[1]) * 100) + '\n';
    output += nn.confusionToString() + '\n';
    output += 'Validation: ' + (nn.accuracy(sets[2]) * 100) + '\n';
    output += nn.confusionToString() + '\n';

    get('#output').innerText = output;

    nn.save("logs/ann-weights.log");
    nn.save("logs/ann-weights-verbose.log", true);

    // Create chart.
    var trainingData = [];
    var trainingAccuracy = nn.trainingAccuracy();
    var testingData = [];
    var testingAccuracy = nn.testingAccuracy();
    for (var i = 0; i < trainingAccuracy.length; i++) {
      trainingData.push({ x: i + 1, y: trainingAccuracy[i] });
      testingData.push({ x: i + 1, y: testingAccuracy[i] });
    }

    var chart = new Chart(get('#chart'), {
      type: 'line',
      data: {
        datasets: [{
          label: 'Training Set',
          data: trainingData,
          backgroundColor: 'transparent',
          borderColor: 'rgba(1, 158, 152, 0.5)',
          pointRadius: 0
        }, {
          label: 'Testing Set',
          data: testingData,
          backgroundColor: 'transparent',
          borderColor: 'rgba(232, 11, 34, 0.5)',
          pointRadius: 0
        }]
      },
      options: {
        maintainAspectRatio: false,
        scales: {
          xAxes: [{
            type: 'linear',
            position: 'bottom',
            scaleLabel: {
              display: true,
              labelString: 'Number of Epochs'
            },
            ticks: {
              min: 1
            }
          }],
          yAxes: [{
            scaleLabel: {
              display: true,
              labelString: 'Accuracy (%)'
            },
            ticks: {
              min: 0,
              max: 100
            }
          }]
        }
      }
    });
  });
  */
});
