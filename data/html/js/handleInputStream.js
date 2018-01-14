InputInfo = function (str, names) {
    console.log('ajax');
    datas = {"uri" : str}
    $.ajax({
        url: 'http://' + window.location.hostname + ':8090' + '/info',
        dataType: 'json',
        type: 'post',
        contentType: "application/json",
        data: JSON.stringify(datas),
        success: function( data, textStatus, jQxhr ){
            console.log(data)
            removeNames(names, "audio_stream");
            // $("#audio_stream").append()
            for (i = 0; i < data["audio"].length; i++) {
                resultHtmls =(("<option value=\""+data["audio"][i]["streamid"]+"\" name=\""+names+"\">" +data["audio"][i]["streamid"]+"</option>"))
                $( "#audio_stream" ).append( resultHtmls );
            }

            //$( "#video_stream" ).html( "" );
            removeNames(names, "video_stream");
            //$("#video_stream").append()
            //$( "#pip_stream" ).html( "" );
            //TODO
            removeNames(names, "pip_stream");
            //$("#pip_stream").append("<option value=\"\" disabled selected style=\"display:none;\">Choose video stream</option>")
            for (i = 0; i < data["video"].length; i++) {
                resultHtmls =(("<option value=\""+data["video"][i]["streamid"]+ "\" name=\""+names+"\">" +data["video"][i]["streamid"]+"</option>"))
                $( "#video_stream" ).append( resultHtmls );
                $("#pip_contained select[name='pip_stream']").each(function () {
                    //console.log($(this))
                    $(this).append(resultHtmls);
                })
            }



        },
        error: function( jqXhr, textStatus, errorThrown ){
            //$( "#audio_stream" ).html( "<option value=\"\" disabled selected style=\"display:none;\">Choose audio stream</option>" );
            //$( "#video_stream" ).html( "<option value=\"\" disabled selected style=\"display:none;\">Choose video stream</option>" );
            //$( "#pip_stream" ).html( "<option value=\"\" disabled selected style=\"display:none;\">Choose video stream</option>" );
            console.log(errorThrown)


        }
    })

}

removeNames = function (names, stream) {
    $( "#"+stream+" option[name=\""+names+"\"]" ).each(function () {
        //console.log(this)
        $(this).remove();

    });

}

$( "#uri-container" ).on('change','*',function() {
        if($(this).val().trim().length !=0){
            InputInfo($(this).val(), $(this).attr('id')   )
        }
        else {
            removeNames($(this).attr('id'), "video_stream");
            removeNames($(this).attr('id'), "audio_stream");

            //TODO
            removeNames($(this).attr('id'), "pip_stream");
        }
});

$( document).on('change','#video_stream',function() {
    console.log("video_change")
    var e = $("#video_stream :selected")[0]
    $("#pip_contained option:not(:selected)").each(function (subindex, subvalue) {
        var flag =true
        $("#pip_contained  option:selected").each(function (index,values){

            if(values.value.length != 0 && subvalue.value.length!=0){
                if (values.value == subvalue.value) {

                    $(subvalue).prop('disabled', true)
                    flag = false
                }
            }
        })
        if(flag){
            if(subvalue.value == e.value) {
                $(subvalue).prop('disabled', true)
            }
            else {
                $(subvalue).prop('disabled', false)
            }
        }

    });

    //var f = document.getElementById("pip_stream");
    //f.options[e.selectedIndex].disabled = true;
});

$(document).on('change','#pip_contained' , function() {
    console.log("pip_change")



    $("#video_stream option").each(function (subindex, subvalue) {
        //console.log(subvalue)
        var flag =true
        $("#pip_contained  option:selected").each(function (index,values) {
        if (values.value == subvalue.value) {
            console.log(subvalue)
            $(subvalue).prop('disabled', true)
            flag = false

        }


        })
        if(flag){
            //console.log(subvalue)
            $(subvalue).prop('disabled', false)
        }
    });
    var e = $("#video_stream :selected")[0]
    $("#pip_contained option:not(:selected)").each(function (subindex, subvalue) {
        var flag =true
        $("#pip_contained  option:selected").each(function (index,values){

        if(values.value.length != 0 && subvalue.value.length!=0){
            if (values.value == subvalue.value) {

                $(subvalue).prop('disabled', true)
                flag = false
            }
        }
        })
        if(flag){
            if(subvalue.value == e.value) {
                $(subvalue).prop('disabled', true)
            }
            else {
                $(subvalue).prop('disabled', false)
            }
        }

    });
});
