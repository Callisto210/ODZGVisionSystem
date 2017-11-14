onSuccess = function () {
    console.log('success');

}

onError = function () {
    console.log('error');
}

SendingStream2 = function (SendStream2) {
    console.log('ajax');

    console.log(SendStream2);
    $.ajax({
        url: "http://localhost:8080/YoutubeManager/api3",
        type: 'POST',
        contentType: "application/json",
        data: JSON.stringify(SendStream2),
        success: onSuccess,
        error: onError
    })


}