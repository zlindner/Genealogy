$(document).ready(function() {
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/readUploads',
        success: function(data) {
            var logTable = '';
            var fileSelect = '';

            if (data.length == 0) {
                logTable += '<p>No GEDCOM files found</p>';
                $('#logContent').append(logTable);
                return;
            }

            logTable += '<tr>';
            logTable += '<th>' + 'Filename' + '</th>';
            logTable += '<th>' + 'Source' + '</th>';
            logTable += '<th>' + 'GEDCOM Version' + '</th>';
            logTable += '<th>' + 'Encoding' + '</th>';
            logTable += '<th>' + 'Submitter Name' + '</th>';
            logTable += '<th>' + 'Submitter Address' + '</th>';
            logTable += '<th>' + 'Number of Individuals' + '</th>';
            logTable += '<th>' + 'Number of Families' + '</th>';
            logTable += '</tr>';

            for (var i = 0; i < data.length; i++) {
                logTable += '<tr>';
                logTable += '<td>' + '<a href=\"/uploads/' + data[i].name + '\">' + data[i].name + '</td>';
                logTable += '<td>' + data[i].source + '</td>';
                logTable += '<td>' + data[i].version + '</td>';
                logTable += '<td>' + data[i].encoding + '</td>';
                logTable += '<td>' + data[i].subName + '</td>';
                logTable += '<td>' + data[i].subAddr + '</td>';
                logTable += '<td>' + data[i].numIndivs + '</td>';
                logTable += '<td>' + data[i].numFams + '</td>';
                logTable += '</tr>';

                fileSelect += '<option value=\"' + data[i].name + '\">' + data[i].name + '</option>'
            }

            $('#logTable').append(logTable);
            $('#fileSelect').append(fileSelect);
            $('#fileSelect2').append(fileSelect);
            $('#fileSelect3').append(fileSelect);
            $('#fileSelect4').append(fileSelect);
        },
        fail: function(error) {
            console.log(error);
        }
    });

    $('#clear').click(function() {
        $('#statusText').html('');
    });

    $('#fileSelect').change(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/viewFile',
            data: ({
                file: $('#fileSelect').val()
            }),
            success: function(data) {
                $('#viewTable').empty();
                $('#viewContent').empty();

                var viewTable = '';

                if (data.length == 0) {
                    viewTable += '<p>No individuals</p>';
                    $('#viewContent').append(viewTable);
                    return;
                }

                viewTable += '<tr>';
                viewTable += '<th>' + 'Given Name' + '</th>';
                viewTable += '<th>' + 'Surname' + '</th>';
                viewTable += '<th>' + 'Sex' + '</th>';
                viewTable += '<th>' + 'Family Size' + '</th>';
                viewTable += '</tr>'

                for (var i = 0; i < data.length; i++) {
                    viewTable += '<tr>';
                    viewTable += '<td>' + data[i]['givenName'] + '</td>';
                    viewTable += '<td>' + data[i]['surname'] + '</td>';
                    viewTable += '<td>' + data[i]['sex'] + '</td>';
                    //viewTable += '<td>' + data[i]['famSize'] + '</td>';
                    viewTable += '<td></td>';
                    viewTable += '</tr>';
                }

                $('#viewTable').append(viewTable);
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    $('#createForm').submit(function(e) {
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/createGEDCOM',
            data: {
                filename: $('#filenameInput').val(),
                source: $('#sourceInput').val(),
                encoding: $('#encodingSelect').prop('selectedIndex'),
                version: $('#versionInput').val(),
                subName: $('#subNameInput').val(),
                subAddr: $('#subAddrInput').val(),
            },
            success: function(data) {
                if (data.length != 0) {
                    $('#statusText').append('Successfully created ' + data.name + '\n');
                } else {
                    $('#statusText').append('Error creating GEDCOM file\n');
                    return;
                }

                $.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/reload',
                    data: {
                        file: data.name,
                    },
                    success: function(data) {
                        var logTable = '';
                        var fileSelect = '';

                        logTable += '<tr>';
                        logTable += '<td>' + '<a href=\"/uploads/' + data.name + '\">' + data.name + '</td>';
                        logTable += '<td>' + data.source + '</td>';
                        logTable += '<td>' + data.version + '</td>';
                        logTable += '<td>' + data.encoding + '</td>';
                        logTable += '<td>' + data.subName + '</td>';
                        logTable += '<td>' + data.subAddr + '</td>';
                        logTable += '<td>' + data.numIndivs + '</td>';
                        logTable += '<td>' + data.numFams + '</td>';
                        logTable += '</tr>';

                        fileSelect += '<option value=\"' + data.name + '\">' + data.name + '</option>'

                        $('#logTable').append(logTable);
                        $('#fileSelect').append(fileSelect);
                        $('#fileSelect2').append(fileSelect);
                        $('#fileSelect3').append(fileSelect);
                        $('#fileSelect4').append(fileSelect);
                    },
                    fail: function(error) {
                        console.log(error);
                    }
                });
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    $('#addIndivForm').submit(function(e) {
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/addIndividual',
            data: {
                givenName: $('#givenInput').val(),
                surname: $('#surnameInput').val(),
                filename: $('#fileSelect2').val(),
            },
            success: function(data) {
                if (data.result == 'success') {
                    $('#statusText').append('Successfully added ' + $('#givenInput').val() + ' ' + $('#surnameInput').val() + ' to ' + $('#fileSelect2').val() + '\n');
                } else {
                    $('#statusText').append('Error adding an individual\n');
                    return;
                }

                $.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/readUploads',
                    success: function(data) {
                        $('#logTable').empty();

                        var logTable = '';

                        if (data.length == 0) {
                            logTable += '<p>No GEDCOM files found</p>';
                            $('#logContent').append(logTable);
                            return;
                        }

                        logTable += '<tr>';
                        logTable += '<th>' + 'Filename' + '</th>';
                        logTable += '<th>' + 'Source' + '</th>';
                        logTable += '<th>' + 'GEDCOM Version' + '</th>';
                        logTable += '<th>' + 'Encoding' + '</th>';
                        logTable += '<th>' + 'Submitter Name' + '</th>';
                        logTable += '<th>' + 'Submitter Address' + '</th>';
                        logTable += '<th>' + 'Number of Individuals' + '</th>';
                        logTable += '<th>' + 'Number of Families' + '</th>';
                        logTable += '</tr>';

                        for (var i = 0; i < data.length; i++) {
                            logTable += '<tr>';
                            logTable += '<td>' + '<a href=\"/uploads/' + data[i].name + '\">' + data[i].name + '</td>';
                            logTable += '<td>' + data[i].source + '</td>';
                            logTable += '<td>' + data[i].version + '</td>';
                            logTable += '<td>' + data[i].encoding + '</td>';
                            logTable += '<td>' + data[i].subName + '</td>';
                            logTable += '<td>' + data[i].subAddr + '</td>';
                            logTable += '<td>' + data[i].numIndivs + '</td>';
                            logTable += '<td>' + data[i].numFams + '</td>';
                            logTable += '</tr>';
                        }

                        $('#logTable').append(logTable);
                    },
                    fail: function(error) {
                        console.log(error);
                    }
                });
            },
            fail: function(error) {
                console.log(error);
            }
        })
    });

    $('#getDescForm').submit(function(e) {
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getDescendants',
            data: {
                givenName: $('#givenInput2').val(),
                surname: $('#surnameInput2').val(),
                filename: $('#fileSelect3').val(),
                maxGen: $('#genInput').val(),
            },
            success: function(data) {
                $('#descTable').empty();
                $('#descContent').empty();

                var descTable = '';

                if (data.length == 0) {
                    descTable += '<p>No descendants found</p>';
                    $('#descContent').append(descTable);
                    return;
                }

                for (var i = 0; i < data.length; i++) {
                    descTable += '<tr>';

                    for (var j = 0; j < data[i].length; j++) {
                        descTable += '<td>' + data[i][j].givenName + ' ' + data[i][j].surname + '</td>';
                    }

                    descTable += '</tr>';
                }

                $('#descTable').append(descTable);
            },
            fail: function(error) {
                console.log(error);
            }
        })
    });

    $('#getAncForm').submit(function(e) {
        e.preventDefault();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getAncestors',
            data: {
                givenName: $('#givenInput3').val(),
                surname: $('#surnameInput3').val(),
                filename: $('#fileSelect4').val(),
                maxGen: $('#genInput2').val(),
            },
            success: function(data) {
                $('#ancTable').empty();
                $('#ancContent').empty();

                var ancTable = '';

                if (data.length == 0) {
                    ancTable += '<p>No ancestors found</p>';
                    $('#ancContent').append(ancTable);
                    return;
                }

                for (var i = 0; i < data.length; i++) {
                    ancTable += '<tr>';

                    for (var j = 0; j < data[i].length; j++) {
                        ancTable += '<td>' + data[i][j].givenName + ' ' + data[i][j].surname + '</td>';
                    }

                    ancTable += '</tr>';
                }

                $('#ancTable').append(ancTable);
            },
            fail: function(error) {
                console.log(error);
            }
        })
    });
});