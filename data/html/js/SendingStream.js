function sleep(ms) {
  return new Promise(function (resolve) { setTimeout(resolve, ms);});
}
var global_width = 750;
var global_height = 600;
var useIcecast = false;
var onSuccess = function (random) {
    var string = 'http://' + window.location.hostname;
    var result = string.concat(':8000/', random, '.webm');
    //
    // await
    sleep(100);
    console.log("Result: " + result);
    var div_result = '<div class="row">' +
        '<div class="col">' +
        '<p>Result stream will be available on <a href="' + result + '">link</a></p>' +
        '</div>' +
        '</div>';
    if (useIcecast) {
        $('#targetAddress').html(div_result);
        $.ajax({
            url: result,
            type: 'GET',
            tryCount: 0,
            countLimit: 50,
            success: function (data, textStatus, xhr) {
            },
            complete: function (xhr) {
                console.log("Complete");
            },
            timeout: 2000,
            error: function (xhr, textStatus, errorThrown) {
                if(errorThrown == 'timeout') {
                    console.log("Showing video: " + result);
                    $('#video').attr('src', result).attr('width', global_width).attr('height', global_height);
                    $('#myModal').modal();
                } else if (this.tryCount <= this.countLimit) {
                    this.tryCount++;
                    sleep(1000);
                    $.ajax(this);
                }
            }
        }).then(function(response) {
            console.log(response.status);}
        );
    }

};

onError = function (random) {
   console.log('error');
};

SendingStream = function (StreamData) {
    console.log('ajax');
    if (StreamData.height) {
        global_height = StreamData.height;
    }
    if (StreamData.width)
        global_width = StreamData.width;
    useIcecast = (StreamData.sink == 'icecast');
    console.log(StreamData);
    $.ajax({
        url: 'http://' + window.location.hostname + ':8090' +  '/input',
        type: 'POST',
        contentType: "application/json",
        data: JSON.stringify(StreamData),
        success: onSuccess(StreamData.random),
        error: onError(StreamData.random)
    })

};


