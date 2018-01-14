var uri_i = 0;
var pip_i = 0;
handleUri =$(document).ready(function () {

    $('#add_uri').on('click', function(){
        uri_i++;
        $("#uri-container").append("<div><label class=\"col-lg-2\">URI</label><input class=\"col-lg-9\" id=\"URI "+uri_i+"\" name=\"uri\" type=\"text\" /></div>")
    })});

handleUriRemove =$(document).on('click','#remove_uri', function () {
    if(!$('#uri-container').find('div:last input').prop('required')){
        var names = $('#uri-container').find('div:last input').attr('id');
        removeAudioList(names);
        removeNames(names, "video_stream");
        removeNames(names, "audio_stream");
        $('#pip_contained').find('> div').each(function () {
            $(this).find("select option[name=\""+names+"\"]").remove()
        });

        console.log(all_audio)
        $('#uri-container').find('div:last').remove();
        uri_i--;
    }
});

handlepip = $(document).on('click', "#add_pip", function () {
    pip_i ++;
    $('#pip_contained').append("                               <div id=\"pip"+pip_i+"\">        <div><label class=\"col-lg-2\">MultiVideo Stream ID</label>\n" +
        "                                            <select class=\"col-lg-9\" id=\"pip_stream"+pip_i+"\" name=\"pip_stream\" required=\"\">\n" +
        "                                                <option value=\"\" disabled selected style=\"display:none;\">Choose pip stream</option>\n" +
        "                                            </select>\n" +
        "                                        </div>\n" +
        "                                        <div><label class=\"col-lg-2\">MultiVideo x</label>\n" +
        "                                            <input class=\"col-lg-4\" id=\"pip_x"+pip_i+"\" name=\"pip_x\" type=\"number\" min=\"1\" required=\"\" />\n" +
        "                                        </div>\n" +
        "                                        <div><label class=\"col-lg-2\">MultiVideo y</label>\n" +
        "                                            <input class=\"col-lg-4\" id=\"pip_y"+pip_i+"\" name=\"pip_y\" type=\"number\"min=\"1\" required=\"\" />\n" +
        "                                        </div>\n" +
        "                                        <div><label class=\"col-lg-2\">MultiVideo height</label>\n" +
        "                                            <input class=\"col-lg-4\" id=\"pip_height"+pip_i+"\" name=\"pip_height\" type=\"number\" min=\"1\" required=\"\" />\n" +
        "                                        </div>\n" +
        "                                        <div><label class=\"col-lg-2\">MultiVideo width</label>\n" +
        "                                            <input class=\"col-lg-4\" id=\"pip_width"+pip_i+"\" name=\"pip_width\" type=\"number\" min=\"1\" required=\"\"/>\n" +
        "                                        </div></div>");
    $("#pip_contained").find("div:first select[name='pip_stream'] option").each(function () {
        $("#pip_contained").find("div select[name='pip_stream']:last").append($(this).clone());
    });
        $("#pip_contained").find("div select[name='pip_stream']:last").prop('selectedIndex',0);
}
);

removeAudioList = function (names) {
    $('#audio_map').find("p[name=\""+names+"\"]").each(function (index, value) {
        console.log(value)
        var audio_in = all_audio.findIndex(function (element) {
            return element.value === value.value
        });
        if (audio_in > -1) {
            all_audio.splice(audio_in, 1);
        }
        $(value).remove()
    })

};

// removeVideoList = function (names) {
//     $('#audio_map').find("p[name=\""+names+"\"]").each(function (index, value) {
//         console.log(value)
//         var audio_in = all_audio.findIndex(function (element) {
//             return element.value === value.value
//         });
//         if (audio_in > -1) {
//             all_audio.splice(audio_in, 1);
//         }
//         $(value).remove()
//     })
//
// }