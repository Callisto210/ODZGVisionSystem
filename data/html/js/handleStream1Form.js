console.log("Załadował!");

$(document).ready( 
	function () {
    	$('#msform').submit(function () {
        	sendPipe(new Stream1Data());
        	console.log('handle msform')
	        return false;

    	});
	}
);
