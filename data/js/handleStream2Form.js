handleForm =$(document).ready(function () {
    $('#outputform').submit(function () {
        SendingStream2(new Stream2Data());
        console.log('handle')
        return false;

    })


});