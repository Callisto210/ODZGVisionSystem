onSuccess = function () {
    console.log('success');
}

onError = function () {
    console.log('error');
}

sendPipe = function (streamData) {
    console.log('ajax');
    console.log("Input stream");
    console.log(streamData);
    $.ajax({
        url: "/api/input",		
        type: 'POST',
        contentType: "application/json",
        data: JSON.stringify(streamData),
        success: onSuccess,
        error: onError
    })
}
