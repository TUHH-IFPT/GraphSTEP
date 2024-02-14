var addon = require("bindings")("graphstepAddon");

function pull () {
    var outputDir = "data/out.stp";

    const databaseInfo = {
        host: 'http://localhost:7474/',
        database: 'pointcloud',
        user: {
            name: 'neo4j',
            password: 'testpassword'
        }
    };

    console.log("estimated duration: " + addon.estimateDurationDownload(JSON.stringify(databaseInfo)) + " seconds");
    addon.pullFile(outputDir, JSON.stringify(databaseInfo));
}

pull();