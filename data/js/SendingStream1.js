onSuccess = function (random) {
    var string=window.location.host;
    var result=string.concat('/',random,'.webm');

    $('#video').attr("src",result);
    $('#myModal').modal();
;}

onError = function (random) {
   console.log('error');
}

SendingStream1 = function (Stream1Data) {
    console.log('ajax');

    console.log(Stream1Data);
    $.ajax({
        url: "/data/html/index.html",
        type: 'POST',
        contentType: "application/json",
        data: JSON.stringify(Stream1Data),
        success: onSuccess(Stream1Data.random),
        error: onError(Stream1Data.random)
    })

}


