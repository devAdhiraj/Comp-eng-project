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
          var data = new google.visualization.DataTable();
          data.addColumn('string', 'Element');
          data.addColumn('number', 'Percentage');
          data.addRows([
            ['Live Count', liveCount],
            ['Space Left', spaceLeft],
          ]);

          // Instantiate and draw the chart
          var chart = new google.visualization.PieChart(document.getElementById('piechart'));
          chart.draw(data, null);
        }
      }
  })
  .catch((err) => {
    document.getElementById("Piechart").innerHTML = "Device Inactive";
    })
