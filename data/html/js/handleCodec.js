handleCodec =$(document).ready(function () {

    $('#vcodec').on('change', function() {
        requiredChange()
    });


    $('#acodec').on('change', function() {
        requiredChange()
    })
})

function requiredChange(){
    if( $('#acodec').val()!==""){
        $("#vcodec").prop('required',false);

    }
    if( $('#vcodec').val()!==""){
        $("#acodec").prop('required',false);

    }
}