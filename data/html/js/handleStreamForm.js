handleForm =$(document).ready(function () {

            $('#msform').submit(function () {
                console.log('handle')

                SendingStream(new StreamData());
                all_video = []
                all_audio = []
               $("#video_map").html('');
                $("#audio_map").html('');
                requiredChange()
        return false;

    })


});

