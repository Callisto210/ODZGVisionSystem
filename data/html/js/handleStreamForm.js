handleForm =$(document).ready(function () {

            $('#msform').submit(function () {
                console.log('handle')

                SendingStream(new StreamData());
        return false;

    })


});

