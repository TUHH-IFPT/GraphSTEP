var addon = require("bindings")("graphstepAddon");

function rotate() {
    var inputPath = "data/testfile.stp";
    var quaternion = "(0.0,0.0,0.0,1.0)";

    const databaseInfo = {
        host: 'http://localhost:7474/',
        database: 'pointcloud',
        user: {
            name: 'neo4j',
            password: 'testpassword'
        }
    };

    addon.rotatePart(inputPath, quaternion, JSON.stringify(databaseInfo));
}

rotate();