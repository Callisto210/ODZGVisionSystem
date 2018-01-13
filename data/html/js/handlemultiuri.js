var uri_i = 0
handleUri =$(document).ready(function () {

    $('#add_uri').on('click', function(){
        uri_i++;
        $("#uri-container").append("<div><label class=\"col-lg-2\">URI</label><input class=\"col-lg-9\" id=\"uri_"+uri_i+"\" name=\"uri\" type=\"text\" required /></div>")
    })});