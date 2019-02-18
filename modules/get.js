module.exports = function(s, parent) {
  // If parent is undefined, default to document.
  parent = parent || document;
  // Get node list from selector.
  var nodes = parent.querySelectorAll(s);
  // Add event listener wrapper ('on') to each node in the list.
  wrap_all(nodes);
  // If there is only one node, return only that node.
  return (nodes.length === 1) ? nodes[0] : nodes;
};

// Adds all relevant wrappers to the node.
function wrap(node) {
  node.on = function(name, callback) {
    node.addEventListener(name, callback);
  };
}

// Adds all relevant wrappers to every node passed to this function.
function wrap_all() {
  // Iterate over list of arguments.
  for (var arg of arguments) {
    // If arg is an array or a node list wrap each node within it,
    // otherwise treat arg as a single node.
    if (Array.isArray(arg) || arg instanceof NodeList) {
      for (var node of arg) {
        wrap(node);
      }
    } else {
      wrap(arg);
    }
  }
}

// Do some default wrapping.
wrap_all(window, document);
