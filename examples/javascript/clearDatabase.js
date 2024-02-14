var addon = require("bindings")("graphstepAddon");

function clear() {

    const databaseInfo = {
        host: 'http://localhost:7474/',
        database: 'pointcloud',
        user: {
            name: 'neo4j',
            password: 'testpassword'
        }
    };

    addon.clearDatabase(JSON.stringify(databaseInfo));
}

clear();