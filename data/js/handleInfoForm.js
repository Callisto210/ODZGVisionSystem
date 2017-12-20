handleInfoForm =$(document).ready(function () {
    $('#outputform').submit(function () {
        InfoStream(new InfoData());
        console.log('handle')
        return false;

    })


});