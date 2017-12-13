andleForm =$(document).ready(function () {
    $('#msform').submit(function () {
        SendingStream(new StreamData());
        console.log('handle')
        return false;

    })


});