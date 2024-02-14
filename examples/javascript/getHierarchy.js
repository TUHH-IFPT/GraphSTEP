var addon = require("bindings")("graphstepAddon");

function getHierarchy () {
    var outputDir = "data/out.stp";

    const databaseInfo = {
        host: 'http://localhost:7474/',
        database: 'productgraph',
        user: {
            name: 'neo4j',
            password: 'testpassword'
        }
    };

    const hierarchy = JSON.parse(addon.getProductHierarchy(JSON.stringify(databaseInfo)));
    console.log(JSON.stringify(hierarchy));
}

getHierarchy();