var uri_i = 0
var pip_i = 0
handleUri =$(document).ready(function () {

    $('#add_uri').on('click', function(){
        uri_i++;
        $("#uri-container").append("<div><label class=\"col-lg-2\">URI</label><input class=\"col-lg-9\" id=\"uri_"+uri_i+"\" name=\"uri\" type=\"text\" /></div>")
    })});

handleUriRemove =$(document).on('click','#remove_uri', function () {
    if(!$('#uri-container div:last input').prop('required')){
        removeNames($('#uri-container div:last input').attr('id'), "video_stream");
        removeNames($('#uri-container div:last input').attr('id'), "audio_stream");
        removeNames($('#uri-container div:last input').attr('id'), "pip_stream");
        $('#uri-container div:last').remove();
        uri_i--;
    }
})

handlepip = $(document).on('click', "#add_pip", function () {
    pip_i ++;
    $('#pip_contained').append("                               <div id=\"pip"+pip_i+"\">        <div><label class=\"col-lg-2\">MultiVideo Stream ID</label>\n" +
        "                                            <select class=\"col-lg-9\" id=\"pip_stream"+pip_i+"\" name=\"pip_stream\" >\n" +
        "                                                <option value=\"\" disabled selected style=\"display:none;\">Choose pip stream</option>\n" +
        "                                            </select>\n" +
        "                                        </div>\n" +
        "                                        <div><label class=\"col-lg-2\">MultiVideo x</label>\n" +
        "                                            <input class=\"col-lg-4\" id=\"pip_x"+pip_i+"\" name=\"pip_x\" type=\"number\" min=\"1\"/>\n" +
        "                                        </div>\n" +
        "                                        <div><label class=\"col-lg-2\">MultiVideo y</label>\n" +
        "                                            <input class=\"col-lg-4\" id=\"pip_y"+pip_i+"\" name=\"pip_stream\" type=\"number\"min=\"1\" />\n" +
        "                                        </div>\n" +
        "                                        <div><label class=\"col-lg-2\">MultiVideo height</label>\n" +
        "                                            <input class=\"col-lg-4\" id=\"pip_height"+pip_i+"\" name=\"pip_height\" type=\"number\" min=\"1\"/>\n" +
        "                                        </div>\n" +
        "                                        <div><label class=\"col-lg-2\">MultiVideo width</label>\n" +
        "                                            <input class=\"col-lg-4\" id=\"pip_width"+pip_i+"\" name=\"pip_width\" type=\"number\" min=\"1\"/>\n" +
        "                                        </div></div>")
    $("#pip_contained div:first select[name='pip_stream'] option").each(function () {
        $("#pip_contained div select[name='pip_stream']:last").append($(this).clone());
    })
}
)