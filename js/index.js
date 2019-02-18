var {DataClass, NeuralNetwork} = require('./../modules/neural-network');
var Chart = require('chart.js');
var moment = require('moment');
var get = require('./../modules/get');
var createElement = require('./../modules/create-element');
var Stopwatch = require('./../modules/stopwatch');

var Page = function(name, run) {
  // Set initial name.
  this.name = name;
  // The DataClass for this page.
  this.data = null;
  // The NeuralNetwork for this page.
  this.nn = null;
  // Holds visualised output from the data class and neural network.
  this.output = {
    text: createElement('pre', {
      classname: 'text'
    }),
    chart: createElement('div', {
      classname: 'chart',
      children: [
        createElement('canvas')
      ]
    })
  };
  //
  this.chart = null;
  // An overwriteable function used to display the page. This is a
  // default implementation but should be good enough for most pages.
  // parent   the parent element in which to display the page
  this.display = function(parent) {
    // If parent is a string, use it as a selector to retrieve the
    // actual parent element.
    if (typeof parent === 'string') {
      parent = get(parent);
    }
    // Create page elements to display.
    var controls = createElement('div', {
      classname: 'controls',
      children: [
        createElement('div', {
          classname: 'group',
          children: [
            createElement('button', {
              text: 'Run ' + this.name,
              on: {
                click: (function(page) {
                  return function() {
                    page.run();
                  };
                })(this)
              }
            })
          ]
        })
      ]
    });
    // Display the created page elements.
    parent.innerHTML = "";
    parent.appendChild(controls);
    parent.appendChild(createElement('div', {
      classname: 'output',
      children: [
        this.output.chart,
        this.output.text
      ]
    }));
  };
  // Updates the display of output, rather than redisplaying the entire
  // page. This function can also be overwritten if necessary.
  this.update = function() {

  };
  // Populates chart with the supplied testing and training data.
  this.populateChart = function() {
    if (this.chart === null) {
      // Create chart.
      this.chart = new Chart(this.output.chart.querySelector('canvas'), {
        type: 'line',
        data: {
          datasets: [{
            label: 'Training Set',
            data: [],
            backgroundColor: 'transparent',
            borderColor: 'rgba(1, 158, 152, 0.5)',
            pointRadius: 0
          }, {
            label: 'Testing Set',
            data: [],
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
                labelString: 'Epochs'
              },
              ticks: {
                min: 1
              }
            }],
            yAxes: [{
              scaleLabel: {
                display: true,
                labelString: 'Percentage'
              },
              ticks: {
                min: 0,
                max: 100
              }
            }]
          }
        }
      });
    }
    // Now, (re)populate chart.
    var training = this.nn.trainingAccuracy();
    var testing = this.nn.testingAccuracy();
    var epochs = training.length;
    var trainingPoints = [];
    var testingPoints = [];
    for (var i = 0; i < epochs; i++) {
      trainingPoints.push({ x: i + 1, y: training[i] });
      testingPoints.push({ x: i + 1, y: testing[i] });
    }

    this.chart.data.datasets[0].data = trainingPoints;
    this.chart.data.datasets[1].data = testingPoints;

    this.chart.update();
  };
  //
  this.appendAccuracyAndConfusion = function(sets) {
    this.log('');
    var acc = 'Training accuracy:   ' + (this.nn.accuracy(sets[0]) * 100).toFixed(2) + '%\n';
    var confusionTraining = this.nn.confusion();
    acc += 'Testing accuracy:    ' + (this.nn.accuracy(sets[1]) * 100).toFixed(2) + '%\n';
    var confusionTesting = this.nn.confusion();
    acc += 'Validation accuracy: ' + (this.nn.accuracy(sets[2]) * 100).toFixed(2) + '%\n\n';
    var confusionValidation = this.nn.confusion();
    acc += 'Training Confusion Matrix:\n\n' + confusionTraining + '\n';
    acc += 'Testing Confusion Matrix:\n\n' + confusionTesting + '\n';
    acc += 'Validation Confusion Matrix:\n\n' + confusionValidation + '\n';
    this.output.text.innerText += acc;
  };
  //
  this.log = function(message) {
    if (this.output.text !== null) {
      this.output.text.innerText += '[' + moment().format('HH:mm:ss') + '] ' + message + '\n';
    }
  };
  // This function is called when the 'Run' or 'Run All' buttons are
  // clicked. This must be implemented.
  this.run = run || function() {
    console.warn(`Unimplemented run function for ${this.name} page.`);
  };
  // Return the Page object.
  return this;
};

var pages = {
  list: [
    new Page('Iris', function() {
      // Empty output.
      this.output.text.innerText = '';

      // The number of training epochs.
      var epochs = 100;
      // The training constant.
      var eta = 0.1;

      this.log(`Epochs: ${epochs}`);
      this.log(`Learning constant: ${eta}\n`);

      // Create new DataClass.
      this.data = new DataClass('data/iris-original.dat');
      // Normalise data.
      this.data.normalise(0, 3);
      // Make exemplars.
      this.data.makeExemplar(4, 3, 1);
      // Split into three even sets: training, testing, and validation.
      var sets = this.data.splitEvenly(3);
      // Create and train neural network.
      this.nn = new NeuralNetwork(4, 5, 3);
      // Let user know that training has started.
      if (this.output.text !== null) {
        this.output.text.innerText += 'Training has begun.\n';
      }
      // Begin training.
      this.nn.train(sets[0], sets[1], epochs, eta, 'logs/iris/neural-network.log');
      // Let user know that training has ended.
      if (this.output.text !== null) {
        this.output.text.innerText += 'Training has ended after ' + epochs + ' epochs.\n\n';
      }

      // Get accuracies of training, testing, and validation sets.
      this.appendAccuracyAndConfusion(sets);

      // Save weights for future use.
      this.nn.save('logs/iris/weights.dat', false, 8);
      this.output.text.innerText += 'Final weights saved.';

      // Populate scatter plot.
      this.populateChart();
    }),
    new Page('Cancer', function() {
      // Empty output.
      this.output.text.innerText = '';
      // Create DataClass.
      this.data = new DataClass('data/cancer.dat');
      // Normalise.
      this.data.normalise(0, 8);
      // Make exemplars.
      this.data.makeExemplar(9, 2, 1);
      // Split into three even sets: training, testing, validation.
      var sets = this.data.splitEvenly(3);
      // Number of training epochs and training constant.
      var epochs = 14, eta = 0.03;
      this.log(`Epochs: ${epochs}`);
      this.log(`Learning constant: ${eta}\n`);
      // Create neural network.
      this.nn = new NeuralNetwork(9, 3, 2);
      this.log('Training has begun.');
      var stopwatch = new Stopwatch();
      stopwatch.start();
      this.nn.train(sets[0], sets[1], epochs, eta, 'logs/cancer/neural-network.log');
      stopwatch.stop();
      this.log(`Training has ended after ${stopwatch.elapsed()} milliseconds.\n`);

      this.appendAccuracyAndConfusion(sets);

      this.nn.save('logs/cancer/weights.dat', false, 8);
      this.log('Final weights saved.');

      this.populateChart();
    }),
    new Page('Wine', function() {
      // Empty output.
      this.output.text.innerText = '';
      // Create data class.
      this.data = new DataClass('data/wine.dat');
      // Normalise and make exemplar.
      this.data.normalise(0, 12);
      this.data.makeExemplar(13, 3, 1);
      // Split into three even sets: training, testing, and validation.
      var sets = this.data.splitEvenly(3);
      // Epochs and training constant.
      var epochs = 70, eta = 0.05;
      this.log(`Epochs: ${epochs}`);
      this.log(`Learning constant: ${eta}\n`);
      // Create neural network.
      this.nn = new NeuralNetwork(13, 5, 3);
      var stopwatch = new Stopwatch();
      stopwatch.start();
      this.log('Training has begun.');
      this.nn.train(sets[0], sets[1], epochs, eta, 'logs/wine/neural-network.log');
      stopwatch.stop();
      this.log(`Training has ended after ${stopwatch.elapsed()} milliseconds.\n`);

      this.appendAccuracyAndConfusion(sets);

      this.nn.save('logs/wine/weights.dat', false, 8);
      this.log('Final weights saved.');

      this.populateChart();
    })
  ],
  selected: 0,
  parent: '#page',
  // Displays the selected page in the parent node.
  display: function() {
    this.list[this.selected].display(this.parent);
  }
};

window.on('load', () => {
  // Generate buttons for switching between tabs.
  var nav = get('#navigation nav ul');
  for (var i = 0; i < pages.list.length; i++) {
    nav.appendChild(createElement('li', {
      children: [
        createElement('button', {
          text: pages.list[i].name,
          on: {
            click: (function(index) {
              return function() {
                pages.selected = index;
                pages.display();
              };
            })(i)
          }
        })
      ]
    }));
  }
  // Display default page.
  pages.display();
});
