/*
This file contains the API fetch request which allows it to retrieve the count and Capacity
data from the thingspeak website. It then provides those values to the piechart, and then it
renders the piechart and sends it to the index.html file.
It also includes logic, so if their is no data available, then it displays device inactive.
Also, this code resizes the graph when the window resizes, which makes the graph responsive.
This means that you can view this website on any device like phone, tablet, laptop, pc, etc.
*/

window.addEventListener("resize", myFunction);
myFunction();
function myFunction(){
  fetch('https://api.thingspeak.com/channels/1265432/feeds/last/')
  .then((response) => {
    return response.json()
  })
  .then((data) => {
    liveCount = parseInt(data.field1);
    maxCapacity = parseInt(data.field2);
    spaceLeft = maxCapacity - liveCount;

    if(maxCapacity < liveCount){
      spaceLeft = 0;
      document.getElementById('inactive').innerHTML = "At Max Capacity";
    }

    document.getElementById("count").innerHTML = "Live Count: " + String(liveCount);
    document.getElementById("cap").innerHTML = "Max Capacity: " + String(maxCapacity);
    document.getElementById("space").innerHTML = "Space Left: " + String(spaceLeft);

    console.log(typeof(maxCapacity) == "number");
    console.log(maxCapacity);
    console.log(spaceLeft);

    if(typeof(maxCapacity) == "number" && maxCapacity > 0 && spaceLeft > 0){
      console.log("wtf");
    google.charts.load('current', {packages: ['corechart']});
        google.charts.setOnLoadCallback(drawChart);

        function drawChart() {
          // Define the chart to be drawn.
          var options = {
    backgroundColor: 'transparent',
    pieSliceText: 'label',
    pieSliceBorderColor: '#333642',
    is3D: true,
    chartArea: {
      width: '100%',
      height: '94%'
    },
    legend: {
      textStyle: {
        color: '#ffffff'
      },
      position:'none'
    }
  };

          var data = new google.visualization.DataTable();
          data.addColumn('string', 'Element');
          data.addColumn('number', 'Percentage');
          data.addRows([
            ['Live Count', liveCount],
            ['Space Left', spaceLeft],
          ]);

          // Instantiate and draw the chart
          var chart = new google.visualization.PieChart(document.getElementById('piechart'));
          chart.draw(data, options);
        }
      }
    })

  .catch((err) => {
    document.getElementById("inactive").innerHTML = "Device Inactive";
    })
}
