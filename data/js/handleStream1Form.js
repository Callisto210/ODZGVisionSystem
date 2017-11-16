handleForm =$(document).ready(function () {
    $('#msform').submit(function () {
        SendingStream1(new Stream1Data());
        console.log('handle')
        return false;

    })


});