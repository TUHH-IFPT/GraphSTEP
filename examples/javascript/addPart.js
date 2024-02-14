var addon = require("bindings")("graphstepAddon");

function add() {
    var inputPath = "data/testfile.stp";
    var assembly = "montage-lasche";
    var part = "";

    const databaseInfo = {
        host: 'http://localhost:7474/',
        database: 'pointcloud',
        user: {
            name: 'neo4j',
            password: 'testpassword'
        }
    };

    addon.addPart(inputPath, assembly, part, JSON.stringify(databaseInfo));
}

add();