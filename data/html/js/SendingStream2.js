onSuccess = function () {
    console.log('success');

}

onError = function () {
    console.log('error');
}

SendingOutStream = function (stream) {
    console.log('ajax');
    console.log("output stream");
    console.log(stream);
    $.ajax({
        url: "/api/output",
        type: 'POST',
        contentType: "application/json",
        data: JSON.stringify(stream),
        success: onSuccess,
        error: onError
    })


}
