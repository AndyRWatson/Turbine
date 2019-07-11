var express = require('express');

//const Gpio = require('pigpio').Gpio;

// ***************************************
// Get Weather Report  for a given city name
// ***************************************
function setSpeed(speed, callback) {
   var speed_rc=0;
   console.log("[setSpeed] Set speed to ",speed);
   callback(speed_rc);
}

//**************** 
// Main
//**************** 

module.exports = function(app) {
    var router = express.Router();
  
    router.get('/', function (req, res, next) {
       var speed = req.query.speed;
       console.log("speed=",speed)

       if (speed == null) {
          speed = "90";
       }
      
       if (speed == "On" || speed == "ON" || speed == "on"){
         speed = "10";
       }
       if (speed == "Off" || speed == "OFF" || speed == "off"){
         speed = "0";
      }
      
      if (speed >=0 && speed <=100){

  // ***********************
  // Uncomment following block 
  // ***********************
       setSpeed(speed, function(speed_rc){
           console.log("setSpeed RC ",speed_rc);
       });
       res.json({setTurbineSpeed_RC: speed});
      }
    });
    
    console.log("/setTurbineSpeed ** Running **")
    app.use("/setTurbineSpeed", router);
  }