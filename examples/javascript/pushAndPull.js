var addon = require("bindings")("graphstepAddon");

const databaseInfo = {
    host: 'http://localhost:7474/',
    database: 'pointcloud',
    user: {
        name: 'neo4j',
        password: 'testpassword'
    }
};

function push() {
    var inputPath = "/home/user/GraphSTEP/data/testfile.stp";
    addon.pushFile(inputPath, JSON.stringify(databaseInfo));
}

function pull () {
    var outputDir = "data/out.stp";
    addon.pullFile(outputDir, JSON.stringify(databaseInfo));
}

push();
pull();