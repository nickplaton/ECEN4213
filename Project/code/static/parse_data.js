if (!!window.EventSource) {
    var source = new EventSource('/');
    source.onmessage = function(e) {
      var bumper = e.data[1]
      var cliff = e.data[3];
      var drop = e.data[5];
      
      const bumper_values = [
        "OFF", "Right", "Center", "CenterRight",
        "Left", "LeftRight", "LeftCenter", "All"
      ];
      const drop_values = [
        "OFF", "Right", "Left", "Both"
      ];
      const cliff_values = [
        "OFF", "Right", "Center", "CenterRight",
        "Left", "LeftRight", "LeftCenter", "All"
      ];


      // finish the code to handle the bumper status
        /*if (bumper=="0")
          {
            document.getElementById("but1").value = "OFF";
          }
        else if (bumper == "1")
        {
          document.getElementById("but1").value = "Right";
        }
        else if (bumper == "2"){
          document.getElementById("but1").value = "Center";
        }*/
      document.getElementById("but1").value = bumper_values[bumper];
        
         
        
        // finish the code to handle the wheel drop status 
      /*if (drop=="0")
        {
          document.getElementById("the id of button where you need to display the sensor status").value = "OFF";

        }*/
      document.getElementById("but2").value = drop_values[drop];

      // finish the code to handle cliff status 
      /*if (cliff=="0")
        {
          document.getElementById("the id of button where you need to display the sensor status").value = "OFF";
        }*/
      document.getElementById("but3").value = cliff_values[cliff];

    }
  }