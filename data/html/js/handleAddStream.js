all_audio = [];
all_video = [];

handleCodec =$(document).ready(function () {
    $('#add_stream').on('click', function(){($('#modal_add_stream').modal())
})});


handleAudio =$(document).ready(function () {
    $('#add_stream1').on('click', function(){
        var data = new audio_data();
        if(!jQuery.isEmptyObject(data) && data.hasOwnProperty("acodec")){
            all_audio.push(data);

            $("#audio_stream").find(":selected").remove();

            claeraudio();
            requiredChange();
            showAudio();
            var mainList = $("#audio_map");




            mainList.append("<p class=\"list-group-item-heading\">"+"Codec: "+ data.acodec+ ", Bitrate: "+ data.audio_bitrate+ ", ID: "+ data.audio_stream+"</p>");

        }
    })});



handleVideo =$(document).ready(function () {
    $('#add_stream2').on('click', function(){
        var data = new video_data();
        if (!jQuery.isEmptyObject(data) && data.hasOwnProperty("vcodec")){

            all_video.push(data);
            console.log(data);
            var video_streams = $("#video_stream").find(":selected").val();
            removeValue('video_stream', video_streams);
            $('#pip_contained').find('> div').each(function () {
                $(this).find("select option[value=\""+video_streams+"\"]").remove()
            });
            if($('#pip_check').prop('checked')) {
                $("#pip_contained").find("option:selected").each(function (i, val) {
                    if(val.value.length !==0) {
                        removeValue('video_stream', val.value);
                        $("#pip_contained").find("option").each(function (index, elem) {
                            if(elem.value === val.value) {
                                $(elem).remove();
                            }
                        })
                    }
                })
            }
            clearvideo();
            requiredChange();
            showVideo();
            var mainList = $("#video_map");

            mainList.append("<p class=\"list-group-item-heading\">"+"Codec: "+ data.vcodec+ ", Bitrate: "+ data.video_bitrate+ ", ID: "+ data.video_stream+", fps: "+ data.fps+", height: "+ data.height+", width "+data.width+ "</p>");

        }


    })});
removeValue = function (stream, values) {

    $( "#"+stream+" option[value=\""+values+"\"]" ).each(function () {
        //console.log(this)
        $(this).remove();

    });

};

claeraudio = function () {

        $("#acodec").val('');
        $("#audio_bitrate").val('');
        $("#audio_stream").prop('selectedIndex',0);
};
clearvideo = function () {
    $("#video_bitrate").val('');
    $("#fps").val('');
    $("#vcodec").val('');
    $("#video_stream").prop("selectedIndex",0);
    $("#height").val('');
    $("#width").val('');
    $('#pip_contained').find('>  div').each(function () {
        $(this).find("input[name='pip_height']").val('');
        $(this).find("input[name='pip_width']").val('');
        $(this).find("input[name='pip_x']").val('');
        $(this).find("input[name='pip_y']").val('');
    });
    $('#pip_contained').find('>  div:not(:last)').each(function () {
        $(this).remove()
    });
    $('#pip_check').prop('checked',false);
    showpip()

};


audio_data = function () {
    if ($('#audio_bitrate').val()!==""){this.audio_bitrate=$('#audio_bitrate').val();}
    if ($('#audio_stream').val()!==""){this.audio_stream=$('#audio_stream').val();}
    if ($('#acodec').val()!==null){this.acodec=$('#acodec').val();}
};

video_data = function () {
    if ($('#video_bitrate').val()!==""){this.video_bitrate=$('#video_bitrate').val();}
    if ($('#video_stream').val()!==""){this.video_stream=$('#video_stream').val();}
    if ($('#vcodec').val()!==null){this.vcodec=$('#vcodec').val();}
    if ($('#fps').val()!==""){this.fps=$('#fps').val();}
    if($('#width').val()!==""){ this.width=$('#width').val();}
    if ($('#height').val()!==""){this.height=$('#height').val();}
    if($('#pip_check').prop("checked")){
        this.pip = $('#pip_contained').find('>  div').map(function () {
            return new pip_data(this);
        }).toArray()

    }
};
pip_data = function (data) {
    //console.log($(data).find('option:selected'))
    this.pip_stream=$(data).find("option:selected").val();
    this.pip_height=$(data).find("input[name='pip_height']").val();
    this.pip_width=$(data).find("input[name='pip_width']").val();
    this.x=$(data).find("input[name='pip_x']").val();
    this.y=$(data).find("input[name='pip_y']").val();

};



