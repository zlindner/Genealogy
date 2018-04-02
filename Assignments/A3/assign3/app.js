'use strict'

// C library API
const ffi = require('ffi');

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

//******************** Your code goes here ********************

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
        var result = {name: filename};

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

    var result = {result: ''};

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

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
