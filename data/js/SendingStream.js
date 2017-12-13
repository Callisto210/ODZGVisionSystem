onSuccess = function (random) {
    var string=window.location.host;
    var result=string.concat('/',random,'.webm');

    $('#video').attr("src",result);
    $('#myModal').modal();
;}

onError = function (random) {
   console.log('error');
}

SendingStream = function (StreamData) {
    console.log('ajax');

    console.log(StreamData);
    $.ajax({
        url: "/data/html/index.html",
        type: 'POST',
        contentType: "application/json",
        data: JSON.stringify(StreamData),
        success: onSuccess(StreamData.random),
        error: onError(StreamData.random)
    })

}


