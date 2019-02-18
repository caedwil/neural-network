module.exports = function(tag, attr) {
  // Create elemnt using the specified tag.
  var e = document.createElement(tag);

  // Add any specified attributes.
  if (attr !== undefined && attr !== null) {
    // If defined, set classname.
    if (attr.classname !== undefined && attr.classname !== null) {
      e.className = attr.classname;
      // Set classname to undefined.
      attr.classname = undefined;
    }
    // If defined, add classes.
    if (attr.classes !== undefined && Array.isArray(attr.classes)) {
      for (var cls of attr.classes) {
        e.classList.add(cls);
      }
      // Set classes to undefined.
      attr.classes = undefined;
    }
    // If defined, add styles.
    if (attr.css !== undefined && attr.css !== null) {
      for (var prop in attr.css) {
        e.style[prop] = attr.css[prop];
      }
      // Set css to undefined.
      attr.css = undefined;
    }
    // If defined, set inner text.
    if (attr.text !== undefined) {
      e.innerText = attr.text;
      // Set text to undefined.
      attr.text = undefined;
    }
    // If defined, set inner HTML.
    if (attr.html !== undefined) {
      e.innerHTML = attr.html;
      // Set html to undefined.
      attr.html = undefined;
    }
    // If defined and is an array, append children.
    if (attr.children !== undefined && Array.isArray(attr.children)) {
      for (var child of attr.children) {
        e.appendChild(child);
      }
      // Set children to undefined.
      attr.children = undefined;
    }
    // If defined, add event listeners.
    if (attr.on !== undefined && attr.on !== null) {
      for (var prop in attr.on) {
        e.addEventListener(prop, attr.on[prop]);
      }
      // Set on to undefined.
      attr.on = undefined;
    }
    // Add all remaining attributes to the element.
    for (var prop in attr) {
      if (attr[prop] !== undefined && attr[prop] !== null) {
        e[prop] = attr[prop];
      }
    }
  }

  // Return finished element.
  return e;
};
