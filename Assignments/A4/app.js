'use strict'

// C library API
const ffi = require('ffi');

// MySQL
const mysql = require('mysql');

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/', function(req, res) {
    res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style, do not change
app.get('/style.css', function(req, res) {
    //Feel free to change the contents of style.css to prettify your Web app
    res.sendFile(path.join(__dirname + '/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js', function(req, res) {
    fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function(err, contents) {
        const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {
            compact: true,
            controlFlowFlattening: true
        });
        res.contentType('application/javascript');
        res.send(minimizedContents._obfuscatedCode);
    });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
    if (!req.files) {
        return res.status(400).send('No files were uploaded.');
    }

    let uploadFile = req.files.uploadFile;

    // Use the mv() method to place the file somewhere on your server
    uploadFile.mv('uploads/' + uploadFile.name, function(err) {
        if (err) {
            return res.status(500).send(err);
        }

        res.redirect('/');
    });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req, res) {
    fs.stat('uploads/' + req.params.name, function(err, stat) {
        console.log(err);
        if (err == null) {
            res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
        } else {
            res.send('');
        }
    });
});

//A3

let gedcom = ffi.Library('gedcom', {
    'getFileInfo': ['string', ['string']],
    'getIndividuals': ['string', ['string']],
    'createSimpleGEDCOM': ['int', ['string', 'string', 'int', 'string', 'string', 'string']],
    'createIndividual': ['int', ['string', 'string', 'string']],
    'getDesc': ['string', ['string', 'string', 'string', 'int']],
    'getAnc': ['string', ['string', 'string', 'string', 'int']],
});

app.get('/readUploads', function(req, res) {
    var file_array = [];

    fs.readdir('uploads/', (err, files) => {
        files.forEach(file => {
            let info = JSON.parse(gedcom.getFileInfo('uploads/' + file));

            if (info != null) {
                info.name = file;

                file_array.push(info);
            }
        });

        res.send(file_array);
    })
});

app.get('/viewFile', function(req, res) {
    let file = req.query.file;
    var indivs = gedcom.getIndividuals('uploads/' + file);

    if (indivs != null) {
        res.send(indivs);
    }
});

app.get('/createGEDCOM', function(req, res) {
    let filename = req.query.filename;
    let source = req.query.source;
    let encoding = req.query.encoding;
    let version = req.query.version;
    let subName = req.query.subName;
    let subAddr = req.query.subAddr;

    let json = [];

    let val = gedcom.createSimpleGEDCOM(filename, source, encoding, version, subName, subAddr);

    if (val == '1') {
        var result = {
            name: filename
        };

        res.send(result);
    } else {
        res.send(json);
    }
});

app.get('/reload', function(req, res) {
    let info = JSON.parse(gedcom.getFileInfo('uploads/' + req.query.file));

    if (info != null) {
        info.name = req.query.file;

        res.send(info);
    }
});

app.get('/addIndividual', function(req, res) {
    let given = req.query.givenName;
    let sur = req.query.surname;
    let filename = req.query.filename;

    var result = {
        result: ''
    };

    if ((!given && !sur) || !filename.includes('.ged')) {
        result.result = 'fail';
    } else {
        let val = gedcom.createIndividual(filename, given, sur);

        if (val == '1') {
            result.result = 'success';
        } else {
            result.result = 'fail';
        }
    }

    res.send(result);
});

app.get('/getDescendants', function(req, res) {
    let given = req.query.givenName;
    let sur = req.query.surname;
    let filename = req.query.filename;
    let maxGen = parseInt(req.query.maxGen);

    var json = [];

    if (!req.query.maxGen || maxGen < 0 || !filename.includes('.ged') || (!given && !sur)) {
        res.send(json);
    } else {
        let gList = gedcom.getDesc(filename, given, sur, maxGen);

        if (gList == null) {
            res.send(json);
        } else {
            res.send(gList);
        }
    }
});

app.get('/getAncestors', function(req, res) {
    let given = req.query.givenName;
    let sur = req.query.surname;
    let filename = req.query.filename;
    let maxGen = parseInt(req.query.maxGen);

    var json = [];

    if (!req.query.maxGen || maxGen < 0 || !filename.includes('.ged') || (!given && !sur)) {
        res.send(json);
    } else {
        let gList = gedcom.getAnc(filename, given, sur, maxGen);

        if (gList == null) {
            res.send(json);
        } else {
            res.send(gList);
        }
    }
});

//A4

let connection;

app.get('/connect', function(req, res) {
    let username = req.query.username;
    let password = req.query.password;
    let database = req.query.database;

    var json = {
        connection: ''
    };

    connection = mysql.createConnection({
        host: 'dursley.socs.uoguelph.ca',
        user: username,
        password: password,
        database: database
    });

    connection.connect(function(err) {
        if (err) {
            json.connection = 'fail';
        } else {
            json.connection = 'success';
        }
    });

    if (json.connection == 'fail') {
        res.send(json);
        return;
    }

    var query = 'CREATE TABLE FILE (';
    query += 'file_id INT AUTO_INCREMENT PRIMARY KEY,';
    query += 'file_Name VARCHAR(60) NOT NULL,';
    query += 'source VARCHAR(10) NOT NULL,';
    query += 'version VARCHAR(10) NOT NULL,';
    query += 'encoding VARCHAR(10) NOT NULL,';
    query += 'sub_name VARCHAR(62) NOT NULL,';
    query += 'sub_addr VARCHAR(256),';
    query += 'num_individuals INT,';
    query += 'num_families INT)';

    connection.query(query, function(error, results, field) {
        if (error) {
            if (error.code != 'ER_TABLE_EXISTS_ERROR') {
                console.log(error);
            }
        }
    });

    query = 'ALTER TABLE FILE DROP INDEX IF EXISTS file_Name';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        }
    });

    query = 'ALTER TABLE FILE ADD UNIQUE (file_Name)';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        }
    });

    query = 'CREATE TABLE INDIVIDUAL (';
    query += 'ind_id INT AUTO_INCREMENT PRIMARY KEY,';
    query += 'surname VARCHAR(256) NOT NULL,';
    query += 'given_name VARCHAR(256) NOT NULL,';
    query += 'sex VARCHAR(1),';
    query += 'fam_size INT,';
    query += 'source_file INT,';
    query += 'FOREIGN KEY (source_file) REFERENCES FILE (file_id) ON DELETE CASCADE)';

    connection.query(query, function(error, results, field) {
        if (error) {
            if (error.code != 'ER_TABLE_EXISTS_ERROR') {
                console.log(error);
            }
        }
    });

    query = 'ALTER TABLE INDIVIDUAL DROP INDEX IF EXISTS unique_indivs';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        }
    });

    query = 'ALTER TABLE INDIVIDUAL ADD UNIQUE unique_indivs (surname, given_name)';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        }
    });

    res.send(json);
});

app.get('/dbStatus', function(req, res) {
    var status = {
        files: 0,
        indivs: 0
    };

    var query = 'SELECT COUNT(*) AS files FROM FILE';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        } else {
            status.files = results[0].files;

            query = 'SELECT COUNT(*) AS indivs FROM INDIVIDUAL';

            connection.query(query, function(error, results, field) {
                if (error) {
                    console.log(error);
                } else {
                    status.indivs = results[0].indivs;

                    res.send(status);
                }
            });
        }
    });
});

app.get('/storeAll', function(req, res) {
    for (var i = 0; i < req.query.files.length; i++) {
        var name = req.query.files[i].name;
        var source = req.query.files[i].source;
        var version = req.query.files[i].version;
        var encoding = req.query.files[i].encoding;
        var subName = req.query.files[i].subName;
        var subAddr = req.query.files[i].subAddr;
        var numIndivs = req.query.files[i].numIndivs;
        var numFams = req.query.files[i].numFams;

        var query = 'INSERT IGNORE INTO FILE (file_Name, source, version, encoding, sub_name, sub_addr, num_individuals, num_families) ';
        query += 'VALUES ("' + name + '", "' + source + '", "' + version + '", "' + encoding + '", "' + subName + '", "' + subAddr + '", "' + numIndivs + '", "' + numFams + '");';

        connection.query(query, function(error, results, field) {
            if (error) {
                console.log(error);
            }
        });
    }

    for (var i = 0; i < req.query.files.length; i++) {
        var name = req.query.files[i].name;

        var query = 'SELECT file_id FROM FILE WHERE file_Name = "' + name + '"';

        connection.query(query, function(error, results, field) {
            if (error) {
                console.log(error);
                return;
            }

            var id = results[0].file_id;
            var name = req.query.files[id - 1].name;
            var indivs = JSON.parse(gedcom.getIndividuals('uploads/' + name));

            for (var j = 0; j < indivs.length; j++) {
                var given = indivs[j].givenName;
                var sur = indivs[j].surname;
                var sex = indivs[j].sex;
                var fam = indivs[j].famSize;

                query = 'INSERT IGNORE INTO INDIVIDUAL (surname, given_name, sex, fam_size, source_file) ';
                query += 'VALUES ("' + sur + '", "' + given + '", "' + sex + '", "' + fam + '", "' + id + '");';

                connection.query(query, function(error, results, field) {
                    if (error) {
                        console.log(error);
                        return;
                    }
                });
            }
        });
    }

    res.json([]);
});

app.get('/clearAll', function(req, res) {
    var query = 'TRUNCATE TABLE INDIVIDUAL';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        }
    });

    query = 'SET FOREIGN_KEY_CHECKS = 0;';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        }
    });

    query = 'TRUNCATE TABLE FILE';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        }
    });

    query = 'SET FOREIGN_KEY_CHECKS = 1;';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        }
    });

    res.json([]);
});

app.get('/submitQuery', function(req, res) {
    var query = req.query.query;

    connection.query(query, function(error, results, field) {
        if (error) {
            res.json([]);
        } else {
            res.send(results);
        }
    });
});

app.get('/queryHelp', function(req, res) {
    var query = 'DESCRIBE FILE';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        } else {
            var file = [];

            for (var i = 0; i < results.length; i++) {
                var col = {
                    field: results[i].Field,
                    type: results[i].Type
                };

                file.push(col);
            }

            query = 'DESCRIBE INDIVIDUAL';

            connection.query(query, function(error, results, field) {
                if (error) {
                    console.log(error);
                } else {
                    var individual = [];

                    for (var i = 0; i < results.length; i++) {
                        var col = {
                            field: results[i].Field,
                            type: results[i].Type
                        };

                        individual.push(col);
                    }

                    res.json({
                        file_info: file,
                        indiv_info: individual
                    });
                }
            });
        }
    });
});

app.get('/query1', function(req, res) {
    var indivs = [];
    var query = 'SELECT * FROM INDIVIDUAL';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        } else {
            for (var i = 0; i < results.length; i++) {
                var indiv = {
                    given: results[i].given_name,
                    sur: results[i].surname,
                    sex: results[i].sex,
                    fam: results[i].fam_size
                };

                indivs.push(indiv);
            }

            indivs.sort(function(a, b) {
                var x = a.sur.toLowerCase();
                var y = b.sur.toLowerCase();
                return ((x < y) ? -1: ((x > y) ? 1: 0));
            });

            res.send(indivs);
        }
    });
});

app.get('/query2', function(req, res) {
    var query = 'SELECT file_id FROM FILE WHERE file_Name = "' + req.query.filename + '"';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        } else {
            if (results[0] == null) {
                res.json([]);
                return;
            }

            var id = results[0].file_id;

            query = 'SELECT * FROM INDIVIDUAL WHERE source_file = "' + id + '"';

            connection.query(query, function(error, results, field) {
                if (error) {
                    console.log(error);
                } else {
                    var indivs = [];

                    for (var i = 0; i < results.length; i++) {
                        var indiv = {
                            given: results[i].given_name,
                            sur: results[i].surname,
                            sex: results[i].sex,
                            fam: results[i].fam_size
                        };

                        indivs.push(indiv);
                    }

                    res.send(indivs);
                }
            });
        }
    });
});

app.get('/query3', function(req, res) {
    var given = req.query.given;
    var sur = req.query.sur;

    var query = 'SELECT * FROM INDIVIDUAL WHERE given_name = "' + given + '" AND surname = "' + sur + '"';

    connection.query(query, function(error, results, field) {
        if (error) {
            console.log(error);
        } else {
            if (results.length == 0) {
                res.json([]);
                return;
            }

            var id = results[0].source_file;

            query = 'SELECT * FROM FILE WHERE file_id = "' + id + '"';

            connection.query(query, function(error, results, field) {
                if (error) {
                    console.log(error);
                } else {
                    res.send(results);
                }
            });
        }
    });
});

app.get('/query4', function(req, res) {
    var selected  = req.query.selected;

    var query = 'SELECT * FROM INDIVIDUAL WHERE fam_size = "' + selected + '"';

    connection.query(query, function(error, results, field) {
        if (error) {
            res.json([]);
            return;
        } else {
            res.send(results);
        }
    });
});

app.get('/query5', function(req, res) {
    var file = req.query.filename;
    var sex = req.query.sex;

    var query = 'SELECT * FROM FILE WHERE file_Name = "' + file + '"';

    connection.query(query, function(error, results, field) {
        if (error) {
            res.json([]);
            return;
        } else {
            if (results.length == 0) {
                res.json([]);
                return;
            }

            var id = results[0].file_id;

            query = 'SELECT * FROM INDIVIDUAL WHERE source_file = "' + id + '" AND sex = "' + sex + '"';

            connection.query(query, function(error, results, field) {
                if (error) {
                    res.json([]);
                    return;
                } else {
                    if (results.length == 0) {
                        res.json([]);
                        return;
                    }

                    res.send(results);
                }
            });
        }
    });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);