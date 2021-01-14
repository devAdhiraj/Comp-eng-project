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
    document.getElementById("count").innerHTML = "Live Count: " + String(liveCount);
    document.getElementById("cap").innerHTML = "Max Capacity: " + String(maxCapacity);
    document.getElementById("space").innerHTML = "Space Left: " + String(spaceLeft);
    if(maxCapacity != 0){

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
