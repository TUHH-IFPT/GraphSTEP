var addon = require("bindings")("graphstepAddon");

function push() {
    var inputPath = "/home/user/GraphSTEP/data/testfile.stp";

    const databaseInfo = {
        host: 'http://localhost:7474/',
        database: 'pointcloud',
        user: {
            name: 'neo4j',
            password: 'testpassword'
        }
    };

    // write to console
    console.log("estimated duration: " + addon.estimateDurationUpload(inputPath) + " seconds");
    addon.pushFile(inputPath, JSON.stringify(databaseInfo));
}

push();