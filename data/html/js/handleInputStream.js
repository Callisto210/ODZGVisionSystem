InputInfo = function (str, names) {
    console.log('ajax');
    datas = {"uri" : str};
    $.ajax({
        url: 'http://' + window.location.hostname + ':8090' + '/info',
        dataType: 'json',
        type: 'post',
        contentType: "application/json",
        data: JSON.stringify(datas),
        success: function( data, textStatus, jQxhr ){
            console.log(data);
            removeNames(names, "audio_stream");
            for (i = 0; i < data["audio"].length; i++) {
                resultHtmls =(("<option value=\""+data["audio"][i]["streamid"]+"\" name=\""+names+"\">" +data["audio"][i]["streamid"]+"</option>"));
                $( "#audio_stream" ).append( resultHtmls );
            }

            removeNames(names, "video_stream");

            $('#pip_contained').find('> div').each(function () {
                $(this).find("select option[name=\""+names+"\"]").remove()
            });
            for (i = 0; i < data["video"].length; i++) {
                resultHtmls =(("<option value=\""+data["video"][i]["streamid"]+ "\" name=\""+names+"\">" +data["video"][i]["streamid"]+"</option>"));
                $( "#video_stream" ).append( resultHtmls );
                $("#pip_contained").find("select[name='pip_stream']").each(function () {
                    $(this).append(resultHtmls);
                })
            }
        },
        error: function( jqXhr, textStatus, errorThrown ){
            console.log(errorThrown)


        }
    })

};

removeNames = function (names, stream) {
    $( "#"+stream+" option[name=\""+names+"\"]" ).each(function () {
        $(this).remove();

    });

};

$( "#uri-container" ).on('change','*',function() {
        if($(this).val().trim().length !==0){
            InputInfo($(this).val(), $(this).attr('id')   )
        }
        else {
            var names = $(this).attr('id');
            removeNames(names, "video_stream");
            removeNames(names, "audio_stream");

            $('#pip_contained').find('> div').each(function () {
                $(this).find("select option[name=\""+names+"\"]").remove()
            })

        }
});

$( document).on('change','#video_stream',function() {
    console.log("video_change");
    video_change()

});
video_change = function () {
    var e = $("#video_stream").find(":selected")[0];
    $("#pip_contained").find("option:not(:selected)").each(function (subindex, subvalue) {
        var flag =true;
        $("#pip_contained").find("option:selected").each(function (index,values){
            if(values.value.length !== 0 && subvalue.value.length!==0){
                if (values.value === subvalue.value) {
                    $(subvalue).prop('disabled', true);
                    flag = false
                }
            }
        });
        if(flag){
            if(subvalue.value === e.value) {
                $(subvalue).prop('disabled', true)
            }
            else {
                $(subvalue).prop('disabled', false)
            }}
    });
};

$(document).on('change','#pip_contained' , function() {
    console.log("pip_change");

    $("#video_stream").find("option").each(function (subindex, subvalue) {
        var flag =true;
        $("#pip_contained").find("option:selected").each(function (index,values) {
        if (values.value === subvalue.value) {
            //console.log(subvalue)
            $(subvalue).prop('disabled', true);
            flag = false
        }});
        if(flag){
            $(subvalue).prop('disabled', false)
        }
    });
    var e = $("#video_stream").find(":selected")[0];
    $("#pip_contained").find("option:not(:selected)").each(function (subindex, subvalue) {
        var flag =true;
        $("#pip_contained").find("option:selected").each(function (index,values){
        if(values.value.length !== 0 && subvalue.value.length!==0){
            if (values.value === subvalue.value) {
                $(subvalue).prop('disabled', true);
                flag = false
            }}
        });
        if(flag){
            if(subvalue.value === e.value) {
                $(subvalue).prop('disabled', true)
            }
            else {
                $(subvalue).prop('disabled', false)
            }
        }
    });
});
