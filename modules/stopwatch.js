module.exports = function() {
  this.time_start = 0;
  this.time_stop = 0;

  this.start = function() {
    this.time_start = new Date();
    this.time_stop = 0;
  };

  this.stop = function() {
    this.time_stop = new Date();
  };

  this.elapsed = function() {
    if (this.time_stop === 0) {
      return new Date() - this.time_start;
    } else {
      return this.time_stop - this.time_start;
    }
  };

  return this;
};
