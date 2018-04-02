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
            $('#fileSelect5').append(fileSelect);
            $('#fileSelect6').append(fileSelect);
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
                        $('#fileSelect5').append(fileSelect);
                        $('#fileSelect6').append(fileSelect);
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
        });
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
        });
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
        });
    });

    $('#loginModal').modal({
        backdrop: 'static',
        keyboard: false
    });

    $('#connect').click(function() {
        var username = $('#usernameInput').val();
        var password = $('#passwordInput').val();
        var database = $('#databaseInput').val();

        $('#modalReenter').empty();
        $('#modalReenter').append('<br><br><p>Connecting...</p>');

        if (username.length == 0 || password.length == 0 || database.length == 0) {
            $('#modalReenter').empty();
            $('#modalReenter').append('<br><br><p>Connection failed, please try again.</p>');
        } else {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/connect',
                data: ({
                    username: username,
                    password: password,
                    database: database,
                }),
                success: function(data) {
                    $('#modalReenter').empty();

                    if (data.connection == 'fail') {
                        $('#modalReenter').append('<br><br><p>Connection failed, please try again.</p>');
                        return;
                    }

                    $('#loginModal').modal('hide');
                },
                fail: function(error) {
                    console.log(error);
                }
            });
        }
    });

    $('#displayDBStatus').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/dbStatus',
            success: function(data) {
                $('#statusText').append('Database has ' + data.files + ' files and ' + data.indivs + ' individuals\n');
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    $('#storeAllFiles').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/readUploads',
            success: function(data) {
                $.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/storeAll',
                    data: ({
                        files: data
                    }),
                    success: function(data) {
                        $.ajax({
                            type: 'get',
                            dataType: 'json',
                            url: '/dbStatus',
                            success: function(data) {
                                $('#statusText').append('Database has ' + data.files + ' files and ' + data.indivs + ' individuals\n');
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
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    $('#clearAllData').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/clearAll',
            success: function(data) {
                $.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/dbStatus',
                    success: function(data) {
                        $('#statusText').append('Database has ' + data.files + ' files and ' + data.indivs + ' individuals\n');
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

    $('#submitQuery').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: ({
                query: $('#queryTextArea').val(),
            }),
            url: '/submitQuery',
            success: function(data) {
                $('#queryResults').empty();
                $('#qResults').empty();

                if (data.length == 0) {
                    $('#qResults').append('<p>Error executing user inputted query</p>');
                    return;
                }

                var results = '';

                results += '<tr>';

                for (var i = 0; i < Object.keys(data[0]).length; i++) {
                    results += '<th>' + Object.keys(data[0])[i] + '</th>';
                }

                results += '</tr>';

                for (var i = 0; i < data.length; i++) {
                    results += '<tr>';

                    for (var j = 0; j < Object.keys(data[0]).length; j++) {
                        results += '<td>' + data[i][Object.keys(data[0])[j]] + '</td>';
                    }

                    results += '</tr>';
                }

                $('#queryResults').append(results);
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    $('#queryHelp').click(function() {
        $('#helpModal').modal();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/queryHelp',
            success: function(data) {
                $('#helpBody').empty();
                $('#helpFileTable').empty();
                $('#helpIndivTable').empty();

                $('#fileBody').html('<h4>FILE Table</h4>');

                var fileTable = '';
                fileTable += '<tr>';
                fileTable += '<th>' + 'Field' + '</th>';
                fileTable += '<th>' + 'Type' + '</th>';
                fileTable += '</tr>'

                for (var i = 0; i < data.file_info.length; i++) {
                    fileTable += '<tr>';
                    fileTable += '<td>' + data.file_info[i].field + '</td>';
                    fileTable += '<td>' + data.file_info[i].type + '</td>';
                    fileTable += '</tr>';
                }

                $('#indivBody').html('<br><br><h4>INDIVIDUAL Table</h4>');

                var indivTable = '';
                indivTable += '<tr>';
                indivTable += '<th>' + 'Field' + '</th>';
                indivTable += '<th>' + 'Type' + '</th>';
                indivTable += '</tr>'

                for (var i = 0; i < data.indiv_info.length; i++) {
                    indivTable += '<tr>';
                    indivTable += '<td>' + data.file_info[i].field + '</td>';
                    indivTable += '<td>' + data.file_info[i].type + '</td>';
                    indivTable += '</tr>';
                }

                $('#helpFileTable').append(fileTable);
                $('#helpIndivTable').append(indivTable);
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    $('#query1').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/query1',
            success: function(data) {
                $('#qResults').empty();
                $('#queryResults').empty();

                var results = '';

                if (data.length == 0) {
                    $('#qResults').append('<p>There are no individuals</p>');
                    return;
                }

                results += '<tr>';
                results += '<th>' + 'Given Name' + '</th>';
                results += '<th>' + 'Surname' + '</th>';
                results += '<th>' + 'Sex' + '</th>';
                results += '<th>' + 'Family Size' + '</th>';
                results += '</tr>'

                for (var i = 0; i < data.length; i++) {
                    results += '<tr>';
                    results += '<td>' + data[i].given + '</td>';
                    results += '<td>' + data[i].sur + '</td>';
                    results += '<td>' + data[i].sex + '</td>';
                    //viewTable += '<td>' + data[i]['famSize'] + '</td>';
                    results += '<td></td>';
                    results += '</tr>';
                }

                $('#queryResults').append(results);
            },
            fail: function(error) {
                console.log(error);
            }
        })
    });

    $('#fileSelect5').change(function() {
        $('#query2').prop('disabled', false);
        $('#query2').html('Display Individuals From ' + $('#fileSelect5').val());
    });

    $('#query2').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: ({
                filename: $('#fileSelect5').val(),
            }),
            url: '/query2',
            success: function(data) {
                $('#qResults').empty();
                $('#queryResults').empty();

                var results = '';

                if (data.length == 0) {
                    $('#qResults').append('<p>There are no individuals in the file</p>');
                    return;
                }

                results += '<tr>';
                results += '<th>' + 'Given Name' + '</th>';
                results += '<th>' + 'Surname' + '</th>';
                results += '<th>' + 'Sex' + '</th>';
                results += '<th>' + 'Family Size' + '</th>';
                results += '</tr>'

                for (var i = 0; i < data.length; i++) {
                    results += '<tr>';
                    results += '<td>' + data[i].given + '</td>';
                    results += '<td>' + data[i].sur + '</td>';
                    results += '<td>' + data[i].sex + '</td>';
                    //viewTable += '<td>' + data[i]['famSize'] + '</td>';
                    results += '<td></td>';
                    results += '</tr>';
                }

                $('#queryResults').append(results);
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    var given = false;

    $('#queryGiven').on('input', function(e) {
        given = true;

        if ($('#queryGiven').val().length == 0) {
            given = false;
            $('#query3').prop('disabled', true);
        }

        if (given && sur) {
            $('#query3').prop('disabled', false);
        }
    });

    var sur = false;

    $('#querySur').on('input', function(e) {
        sur = true;

        if ($('#querySur').val().length == 0) {
            sur = false;
            $('#query3').prop('disabled', true);
        }

        if (given && sur) {
            $('#query3').prop('disabled', false);
        }
    });

    $('#query3').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: ({
                given: $('#queryGiven').val(),
                sur: $('#querySur').val()
            }),
            url: '/query3',
            success: function(data) {
                $('#qResults').empty();
                $('#queryResults').empty();

                if (data.length == 0) {
                    $('#qResults').append('<p>The individual doesn\'t belong to any file</p>');
                    return;
                }

                var result = '';

                result += '<tr>';
                result += '<th>' + 'File ID' + '</th>';
                result += '<th>' + 'Filename' + '</th>';
                result += '<th>' + 'Source' + '</th>';
                result += '<th>' + 'Version' + '</th>';
                result += '<th>' + 'Encoding' + '</th>';
                result += '<th>' + 'Submitter Name' + '</th>';
                result += '<th>' + 'Submitter Addr' + '</th>';
                result += '<th>' + 'Number of Individuals' + '</th>';
                result += '<th>' + 'Number of Families' + '</th>';
                result += '</tr>'

                for (var i = 0; i < data.length; i++) {
                    result += '<tr>';
                    result += '<td>' + data[i].file_id + '</td>';
                    result += '<td>' + data[i].file_Name + '</td>';
                    result += '<td>' + data[i].source + '</td>';
                    result += '<td>' + data[i].version + '</td>';
                    result += '<td>' + data[i].encoding + '</td>';
                    result += '<td>' + data[i].sub_name + '</td>';
                    result += '<td>' + data[i].sub_addr + '</td>';
                    result += '<td>' + data[i].num_individuals + '</td>';
                    result += '<td>' + data[i].num_families + '</td>';
                    result += '</tr>';
                }

                $('#queryResults').append(result);
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    $('#selectQuery4').change(function() {
        $('#query4').prop('disabled', false);
        $('#query4').html('Find All Individuals That ' + $('#selectQuery4').val());
    });

    $('#query4').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: ({
                selected: $('#selectQuery4').prop('selectedIndex'),
            }),
            url: '/query4',
            success: function(data) {
                $('#qResults').empty();
                $('#queryResults').empty();

                if (data.length == 0) {
                    $('#qResults').append('<p>There are no individuals that ' + $('#selectQuery4').val() + '</p>');
                    return;
                }

                var results = '';

                results += '<tr>';
                results += '<th>' + 'Given Name' + '</th>';
                results += '<th>' + 'Surname' + '</th>';
                results += '<th>' + 'Sex' + '</th>';
                results += '<th>' + 'Family Size' + '</th>';
                results += '</tr>'

                for (var i = 0; i < data.length; i++) {
                    results += '<tr>';
                    results += '<td>' + data[i].given_name + '</td>';
                    results += '<td>' + data[i].surname + '</td>';
                    results += '<td>' + data[i].sex + '</td>';
                    results += '<td>' + data[i].fam_size + '</td>';
                    results += '</tr>';
                }

                $('#queryResults').append(results);
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });

    var fileSelected = false;

    $('#fileSelect6').change(function() {
        fileSelected = true;

        if (fileSelected && sexSelected) {
            $('#query5').prop('disabled', false);
            $('#query5').html('Display all ' + $('#sexSelect').val() + '\'s From ' + $('#fileSelect6').val());
        }
    });

    var sexSelected = false;

    $('#sexSelect').change(function() {
        sexSelected = true;

        if (fileSelected && sexSelected) {
            $('#query5').prop('disabled', false);
            $('#query5').html('Display all ' + $('#sexSelect').val() + '\'s From ' + $('#fileSelect6').val());
        }
    });

    $('#query5').click(function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            data: ({
                filename: $('#fileSelect6').val(),
                sex: $('#sexSelect').val()[0],
            }),
            url: '/query5',
            success: function(data) {
                $('#qResults').empty();
                $('#queryResults').empty();

                if (data.length == 0) {
                    $('#qResults').append('<p>There are no individuals that fit the criteria</p>');
                    return;
                }

                var results = '';

                results += '<tr>';
                results += '<th>' + 'Given Name' + '</th>';
                results += '<th>' + 'Surname' + '</th>';
                results += '<th>' + 'Sex' + '</th>';
                results += '<th>' + 'Family Size' + '</th>';
                results += '</tr>'

                for (var i = 0; i < data.length; i++) {
                    results += '<tr>';
                    results += '<td>' + data[i].given_name + '</td>';
                    results += '<td>' + data[i].surname + '</td>';
                    results += '<td>' + data[i].sex + '</td>';
                    results += '<td>' + data[i].fam_size + '</td>';
                    results += '</tr>';
                }

                $('#queryResults').append(results);
            },
            fail: function(error) {
                console.log(error);
            }
        });
    });
});