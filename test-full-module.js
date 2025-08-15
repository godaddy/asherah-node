const asherah = require('./build/Release/asherah.node');
console.log('Module:', asherah);
console.log('Type:', typeof asherah);
console.log('Constructor:', asherah.constructor.name);

// Try to access Asherah class
if (asherah.Asherah) {
  console.log('Found Asherah class');
  const instance = new asherah.Asherah();
  console.log('Instance methods:', Object.getOwnPropertyNames(instance.constructor.prototype));
}