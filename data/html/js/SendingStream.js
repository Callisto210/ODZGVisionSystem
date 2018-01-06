function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

onSuccess = async function (random) {
    var string='http://' + window.location.hostname;
    var result=string.concat(':8000/',random,'.webm');

    await sleep(1000);

    $('#video').attr("src",result).attr("width", 750).attr("height", 600);
    $('#myModal').modal();
;}

onError = function (random) {
   console.log('error');
}

SendingStream = function (StreamData) {
    console.log('ajax');

    console.log(StreamData);
    $.ajax({
        url: 'http://' + window.location.hostname + ':8090' +  '/input',
        type: 'POST',
        contentType: "application/json",
        data: JSON.stringify(StreamData),
        success: onSuccess(StreamData.random),
        error: onError(StreamData.random)
    })

}


