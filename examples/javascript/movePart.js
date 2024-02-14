var addon = require("bindings")("graphstepAddon");

function move() {
    var inputPath = "data/testfile.stp";
    var position = "(10.0,10.0,10.0)"

    const databaseInfo = {
        host: 'http://localhost:7474/',
        database: 'pointcloud',
        user: {
            name: 'neo4j',
            password: 'testpassword'
        }
    };

    addon.movePart(inputPath, position, JSON.stringify(databaseInfo));
}

move();