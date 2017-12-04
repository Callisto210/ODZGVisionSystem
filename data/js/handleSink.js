handleSink =$(document).ready(function () {
    $('#sink').on('change', function() {
        if( $('#sink').val()=="file"){
            clean();
            $('#file_div').show();
            $("#file").prop('required',true);

        }
        if( $('#sink').val()=="udp"){
            clean();
            $('#udp_div').show();
            $("#port").prop('required',true);
            $("#host").prop('required',true);

        }
        if( $('#sink').val()=="icecast"){
            clean();
            $('#icecast_udp').show();
            $("#ip").prop('required',true);

        }
  });
});

function clean() {
    $("#file").val('');
    $("#port").val('');
    $("#host").val('');
    $("#ip").val('');
    $('#icecast_udp').hide();
    $('#udp_div').hide();
    $('#file_div').hide();
    $("#port").prop('required',false);
    $("#host").prop('required',false);
    $("#ip").prop('required',false);
    $("#file").prop('required',false);

}