onSuccess = function () {
    console.log('success');
}

onError = function () {
    console.log('error');
}

SendingStream1 = function (SendStream1) {
    console.log('ajax');

    console.log(SendStream1);
    $.ajax({
        url: "http://localhost:8080/YoutubeManager/api3",
        type: 'POST',
        contentType: "application/json",
        data: JSON.stringify(SendStream1),
        success: onSuccess,
        error: onError
    })


}