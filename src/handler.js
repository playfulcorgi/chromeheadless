// ReasonML cannot easily export a function using the CommonJS format, so this file bridges the gap between node-faas and ReasonML by "unpacking" the
// object returned by ReasonML and returning just the function under the "handler" key.
module.exports = require('./ActualHandler.bs').handler
