    var socket;
    var firstconnect = true,
        i2cNum  = "0x70",
	disp = [];

// Create a matrix of LEDs inside the <table> tags.
var matrixData;
for(var j=7; j>=0; j--) {
	matrixData += '<tr>';
	for(var i=0; i<8; i++) {
	    matrixData += '<td><div class="LED" id="id'+i+'_'+j+
		'" onclick="LEDclick('+i+','+j+')">'+
		i+','+j+'</div></td>';
	    }
	matrixData += '</tr>';
}
$('#matrixLED').append(matrixData);

// Send one column when LED is clicked.
function LEDclick(i, j) {
	// Finite state machine for the color changing methods
    	if ($('#id'+i+'_'+j).hasClass('ongreen')) {
		$('#id'+i+'_'+j).removeClass('ongreen').addClass('onorange')
		disp[2*i+1] ^= 0x1<<j;
		socket.emit('i2cset', {i2cNum: i2cNum, i: 2*i+1, disp: '0x'+disp[2*i+1].toString(16)});
	} else if ($('#id'+i+'_'+j).hasClass('onorange')) {
		$('#id'+i+'_'+j).removeClass('onorange').addClass('onred')
		disp[2*i] ^= 0x1<<j;
		socket.emit('i2cset', {i2cNum: i2cNum, i: 2*i, disp: '0x'+disp[2*i].toString(16)});
	} else if ($('#id'+i+'_'+j).hasClass('onred')) {
		$('#id'+i+'_'+j).removeClass('onred')
		disp[2*i+1] ^= 0x1<<j;
		socket.emit('i2cset', {i2cNum: i2cNum, i: 2*i+1, disp: '0x'+disp[2*i+1].toString(16)});
	} else {
		$('#id'+i+'_'+j).addClass('ongreen')
		disp[2*i] ^= 0x1<<j;
		socket.emit('i2cset', {i2cNum: i2cNum, i: 2*i, disp: '0x'+disp[2*i].toString(16)});
	}
}

    function connect() {
      if(firstconnect) {
        socket = io.connect(null);

        // See https://github.com/LearnBoost/socket.io/wiki/Exposed-events
        // for Exposed events
        socket.on('message', function(data)
            { status_update("Received: message " + data);});
        socket.on('connect', function()
            { status_update("Connected to Server"); });
        socket.on('disconnect', function()
            { status_update("Disconnected from Server"); });
        socket.on('reconnect', function()
            { status_update("Reconnected to Server"); });
        socket.on('reconnecting', function( nextRetry )
            { status_update("Reconnecting in " + nextRetry/1000 + " s"); });
        socket.on('reconnect_failed', function()
            { message("Reconnect Failed"); });

        socket.on('matrix',  matrix);
        // Read display for initial image.  Store in disp[]
        socket.emit("matrix", i2cNum);

        firstconnect = false;
      }
      else {
        socket.socket.reconnect();
      }
    }

    function disconnect() {
      socket.disconnect();
    }

    // When new data arrives, convert it and display it.
    // data is a string of 16 values, each a pair of hex digits.
    function matrix(data) {
        var i, j;
        // Make data an array, each entry is a pair of digits
        data = data.split(" ");

	// Parse the display information
	for (i = 0; i < data.length; i++){
		disp[i] = parseInt(data[i],16);
	}
	
	// i cycles through each column
        for (i = 0; i < 8; i++) {
            // j cycles through each bit
            for (j = 0; j < 8; j++) {
		if ((((disp[2*i] >> j) & 0x1) === 1) && (((disp[2*i+1] >> j) & 0x1) === 1)) {
			$('#id' + i + '_' + j).removeClass('ongreen onred').addClass('onorange');
		} else if (((disp[2*i] >> j) & 0x1) === 1) {
			$('#id' + i + '_' + j).removeClass('onred onorange').addClass('ongreen');
                } else if ((disp[2*i+1] >> j & 0x1) === 1) {
			$('#id' + i + '_' + j).removeClass('ongreen onorange').addClass('onred');
                } else {	
		        $('#id' + i + '_' + j).removeClass('ongreen onred onorange');
		}
            }
        }
    }

    function status_update(txt){
	$('#status').html(txt);
    }

    function updateFromLED(){
      socket.emit("matrix", i2cNum);    
    }

connect();

$(function () {
    // setup control widget
    $("#i2cNum").val(i2cNum).change(function () {
        i2cNum = $(this).val();
    });
});
